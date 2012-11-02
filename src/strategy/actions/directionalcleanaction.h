/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef RFI_DIRECTIONALCLEAN_ACTION_H
#define RFI_DIRECTIONALCLEAN_ACTION_H

#include <iostream>

#include "../../util/ffttools.h"
#include "../../util/plot.h"

#include "../../msio/samplerow.h"
#include "../../msio/timefrequencydata.h"

#include "action.h"

#include "../algorithms/thresholdtools.h"
#include "../algorithms/uvprojection.h"

#include "../control/artifactset.h"
#include "../control/actionblock.h"

namespace rfiStrategy {

	class DirectionalCleanAction : public Action
	{
		public:
			DirectionalCleanAction() : Action(), _limitingDistance(1.0), _removeRatio(0.25), _attenuationOfCenter(0.01), _channelConvolutionSize(1), _makePlot(true), _values(0)
			{
			}
			virtual ~DirectionalCleanAction()
			{
				Finish();
			}
			virtual void Finish()
			{
				if(_values != 0)
				{
					if(_makePlot)
					{
						Plot plot("clean.pdf");
						unsigned left = 0, right = 0;
						for(unsigned i=0;i!=_valueWidth/2;++i)
						{
							if(_values[i] != 0) left = i;
							if(_values[_valueWidth-i-1] != 0) right = i;
						}
						plot.StartScatter("Positive");
						for(unsigned i=0;i<=left;++i)
							plot.PushDataPoint(i, _values[i]);
						plot.StartScatter("Negative");
						for(unsigned i=0;i<=right;++i)
							plot.PushDataPoint(i, _values[_valueWidth-i-1]);
						plot.Close();
						plot.Show();
					}
					delete[] _values;
					_values = 0;
				}
			}
			virtual std::string Description()
			{
				return "Directional cleaning";
			}
			virtual ActionType Type() const { return DirectionalCleanActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData &contaminated = artifacts.ContaminatedData();
				if(contaminated.ImageCount() != 2 || contaminated.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation)
					throw std::runtime_error("Directional clean action requires single complex image in contaminated data");

				TimeFrequencyData &revised = artifacts.RevisedData();
				if(revised.ImageCount() != 2 || revised.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation)
					throw std::runtime_error("Directional clean action requires single complex image in revised data");

				TimeFrequencyData &original = artifacts.OriginalData();
				if(original.ImageCount() != 2 || original.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation)
					throw std::runtime_error("Directional clean action requires single complex image in original data");

				Image2DPtr
					realDest = Image2D::CreateCopy(revised.GetRealPart()),
					imagDest = Image2D::CreateCopy(revised.GetImaginaryPart()),
					realOriginal = Image2D::CreateCopy(original.GetRealPart()),
					imagOriginal = Image2D::CreateCopy(original.GetImaginaryPart());

				Image2DPtr amplitudes = FFTTools::CreateAbsoluteImage(contaminated.GetImage(0), contaminated.GetImage(1));

				if(_channelConvolutionSize != 1)
					amplitudes = ThresholdTools::FrequencyRectangularConvolution(amplitudes, _channelConvolutionSize);

				if(_values == 0)
				{
					_valueWidth = amplitudes->Width();
					_values = new num_t[amplitudes->Width()];
					for(unsigned i=0;i<amplitudes->Width();++i) _values[i] = 0.0;
				}

				for(unsigned y=0;y<contaminated.ImageHeight();++y)
				{
					performFrequency(artifacts, amplitudes, realDest, imagDest, realOriginal, imagOriginal, y, y == contaminated.ImageHeight()/2);
				}
				revised.SetImage(0, realDest);
				revised.SetImage(1, imagDest);
				original.SetImage(0, realOriginal);
				original.SetImage(1, imagOriginal);
				AOLogger::Debug << "Done: direction clean iteration\n";
			}
			numl_t LimitingDistance() const { return _limitingDistance; }
			void SetLimitingDistance(double limitingDistance) { _limitingDistance = limitingDistance; }

			numl_t RemoveRatio() const { return _removeRatio; }
			void SetRemoveRatio(numl_t ratio) { _removeRatio = ratio; }

			unsigned ChannelConvolutionSize() const { return _channelConvolutionSize; }
			void SetChannelConvolutionSize(unsigned channelConvolutionSize) { _channelConvolutionSize = channelConvolutionSize; }

			numl_t AttenuationOfCenter() const { return _attenuationOfCenter; }
			void SetAttenuationOfCenter(numl_t attenuationOfCenter) { _attenuationOfCenter = attenuationOfCenter; }

			bool MakePlot() const { return _makePlot; }
			void SetMakePlot(bool makePlot) { _makePlot = makePlot; }
		private:
			double _limitingDistance;
			numl_t _removeRatio, _attenuationOfCenter;
			unsigned _channelConvolutionSize;
			unsigned _valueWidth;
			bool _makePlot;
			num_t *_values;

			void performFrequency(ArtifactSet &artifacts, Image2DCPtr amplitudeValues, Image2DPtr realDest, Image2DPtr imagDest, Image2DPtr realOriginal, Image2DPtr imagOriginal, unsigned y, bool verbose=false)
			{
				Image2DCPtr
					realInput = artifacts.ContaminatedData().GetRealPart(),
					imagInput = artifacts.ContaminatedData().GetImaginaryPart();
				
				const size_t
					inputWidth = realInput->Width(),
					destWidth = realDest->Width();

				numl_t
					*uPositions = new numl_t[inputWidth],
					*vPositions = new numl_t[inputWidth];
					
				SampleRowPtr row = SampleRow::CreateFromRow(amplitudeValues, y);
				
				UVProjection::ProjectPositions(artifacts.MetaData(), inputWidth, y, uPositions, vPositions, artifacts.ProjectedDirectionRad());
				
				numl_t minU, maxU;
				UVProjection::MaximalUPositions(inputWidth, uPositions, minU, maxU);
				
				unsigned lowestIndex, highestIndex;
				UVProjection::GetIndicesInProjectedImage(_limitingDistance, minU, maxU, inputWidth, destWidth, lowestIndex, highestIndex);
				for(unsigned i=0;i!=lowestIndex;++i)
				{
					const numl_t weight = (1.0-_attenuationOfCenter)*((numl_t) i / lowestIndex) + _attenuationOfCenter;
					row->SetValue(i, weight * row->Value(i));
				}
				for(unsigned i=highestIndex;i!=destWidth;++i)
				{
					const numl_t weight = _attenuationOfCenter;
					row->SetValue(i, weight * row->Value(i));
				}
				
				unsigned fIndex = row->IndexOfMax();
				
				if(verbose)
					AOLogger::Debug << "Removing component index " << fIndex << '\n';
				
				const numl_t
					mean = row->Mean(),
					sigma = row->StdDev(mean),
					diffR = realInput->Value(fIndex, y),
					diffI = imagInput->Value(fIndex, y),
					amplitude = sqrtnl(diffR*diffR + diffI*diffI),
					phase = atan2nl(diffI, diffR);
				
				if(verbose)
					AOLogger::Debug << "Mean=" << mean << ", sigma=" << sigma << ", component = " << ((amplitude-mean)/sigma) << " x sigma\n";
				
				numl_t amplitudeRemoved = amplitude * _removeRatio;

				numl_t limit = mean;

				if(amplitude < limit || amplitude < 0.0)
				{
					if(verbose)
						AOLogger::Debug << "Strongest component is < limit not continuing with clean\n";
				} else
				{
					subtractComponent(realDest, imagDest, inputWidth, uPositions, fIndex, amplitudeRemoved, phase, y);
					
					//unsigned upperLimit = ((lowestIndex*2) > (destWidth/2)) ? (destWidth/2) : (lowestIndex*2);
					unsigned upperLimit = destWidth/2;
					if(fIndex >= lowestIndex && fIndex < upperLimit)
					{
						if(verbose)
							AOLogger::Debug << "Within limits " << lowestIndex << "-" << upperLimit << '\n';
						_values[fIndex] += amplitudeRemoved;
						subtractComponent(realOriginal, imagOriginal, inputWidth, uPositions, fIndex, amplitudeRemoved, phase, y);
					} else {
						if(verbose)
							AOLogger::Debug << "Outside limits " << lowestIndex << "-" << upperLimit << '\n';
					}
				}
				
				delete[] uPositions;
				delete[] vPositions;
			}

			void subtractComponent(Image2DPtr real, Image2DPtr imaginary, const size_t inputWidth, const numl_t *uPositions, unsigned fIndex, numl_t amplitude, numl_t phase, unsigned y)
			{
				numl_t minU, maxU;
				UVProjection::MaximalUPositions(inputWidth, uPositions, minU, maxU);

				numl_t w = (numl_t) fIndex;
				if(w >= real->Width()/2) {
					w -= real->Width();
				}

				// The following component will be subtracted:
				// amplitude e ^ ( -i (2 pi w (u - minU) / (maxU - minU) + phase) )
				
				// prefactor w
				w = w / (maxU - minU);
				
				// Since fftw performs unnormalized fft, we have to divide by N
				amplitude = amplitude / real->Width();
				
				for(unsigned t=0;t<real->Width();++t)
				{
					numl_t u = uPositions[t];
					numl_t exponent = -2.0 * M_PInl * w * (u - minU) - phase;
					numl_t realValue = amplitude * cosnl(exponent);
					numl_t imagValue = amplitude * sinnl(exponent);
					real->SetValue(t, y, real->Value(t, y) - realValue);
					imaginary->SetValue(t, y, imaginary->Value(t, y) - imagValue);
				}
			}
	};

} // namespace

#endif // RFI_DIRECTIONALCLEAN_ACTION_H
