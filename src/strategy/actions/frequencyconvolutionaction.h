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
#ifndef RFI_FREQUENCY_CONVOLUTION_ACTION
#define RFI_FREQUENCY_CONVOLUTION_ACTION

#include <stdexcept>

#include "../../msio/samplerow.h"

#include "../../imaging/uvimager.h"

#include "../../util/aologger.h"
#include "../../util/progresslistener.h"

#include "action.h"

#include "../control/actionblock.h"
#include "../control/artifactset.h"

#include "../algorithms/thresholdtools.h"

namespace rfiStrategy {

	class FrequencyConvolutionAction : public Action
	{
		public:
			enum KernelKind { RectangleKernel, SincKernel, TotalKernel };
			
			FrequencyConvolutionAction() : Action(), _kernelKind(SincKernel), _convolutionSize(4.0), _inSamples(false)
			{
			}
			virtual std::string Description()
			{
				return "Frequency convolution";
			}
			virtual ActionType Type() const { return FrequencyConvolutionActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				TimeFrequencyData &data = artifacts.ContaminatedData();
				if(_kernelKind == TotalKernel)
				{
					Image2DPtr
						rImage = Image2D::CreateCopy(data.GetImage(0)),
						iImage = Image2D::CreateCopy(data.GetImage(1));
					Convolve(rImage, iImage, artifacts.MetaData(), listener);
					data.SetImage(0, rImage);
					data.SetImage(1, iImage);
					artifacts.SetRevisedData(data);
				}
				else
				{
					if(data.ImageCount() != 1)
						throw std::runtime_error("A frequency convolution can only be applied on single component data");

					Image2DPtr newImage;
					switch(_kernelKind)
					{
						default:
						case RectangleKernel:
						newImage = ThresholdTools::FrequencyRectangularConvolution(data.GetImage(0), (unsigned) roundn(_convolutionSize));
						break;
						case SincKernel:
						newImage = sincConvolution(artifacts.MetaData(), data.GetImage(0));
						break;
					}
					
					data.SetImage(0, newImage);
				}
			}
			
			numl_t ConvolutionSize() const { return _convolutionSize; }
			void SetConvolutionSize(numl_t size) { _convolutionSize = size; }
			
			enum KernelKind KernelKind() const { return _kernelKind; }
			void SetKernelKind(enum KernelKind kind) { _kernelKind = kind; }
			
			bool InSamples() const { return _inSamples; }
			void SetInSamples(bool inSamples) { _inSamples = inSamples; }
		private:
			Image2DPtr sincConvolution(TimeFrequencyMetaDataCPtr metaData, Image2DCPtr source)
			{
				numl_t uvDist = averageUVDist(metaData);
				AOLogger::Debug << "Avg uv dist: " << uvDist << '\n';
				numl_t convolutionSize = convolutionSizeInSamples(uvDist, source->Height());
				AOLogger::Debug << "Convolution size: " << convolutionSize << '\n';
				Image2DPtr destination = Image2D::CreateUnsetImagePtr(source->Width(), source->Height());
				for(unsigned x=0;x<source->Width();++x)
				{
					SampleRowPtr row = SampleRow::CreateFromColumn(source, x);
					row->ConvolveWithSinc(1.0 / convolutionSize);
					row->SetVerticalImageValues(destination, x);
				}
				return destination;
			}
			
			numl_t averageUVDist(TimeFrequencyMetaDataCPtr metaData)
			{
				numl_t sum = 0.0;
				const numl_t
					lowFreq = metaData->Band().channels.begin()->frequencyHz,
					highFreq = metaData->Band().channels.rbegin()->frequencyHz;
				const std::vector<UVW> &uvw = metaData->UVW();
				for(std::vector<UVW>::const_iterator i=uvw.begin();i!=uvw.end();++i)
				{
					const numl_t
						lowU = i->u * lowFreq,
						lowV = i->v * lowFreq,
						highU = i->u * highFreq,
						highV = i->v * highFreq,
						ud = lowU - highU,
						vd = lowV - highV;
					sum += sqrtnl(ud * ud + vd * vd);
				}
				return sum / ((numl_t) uvw.size() * UVImager::SpeedOfLight());
			}
			
			numl_t convolutionSizeInSamples(numl_t uvDist, unsigned height)
			{
				if(_inSamples)
					return _convolutionSize;
				else
					return _convolutionSize * height / uvDist;
			}
			
			void Convolve(Image2DPtr rImage, Image2DPtr iImage, TimeFrequencyMetaDataCPtr metaData, ProgressListener &listener)
			{
				Image2DPtr copyReal = Image2D::CreateCopy(rImage);
				Image2DPtr copyImag = Image2D::CreateCopy(iImage);
				const size_t
					width = rImage->Width(),
					height = rImage->Height();
				const std::vector<UVW> &uvws = metaData->UVW();
				
				const num_t factor = M_PIn / _convolutionSize;
				
				for(size_t y=0;y<height;++y)
				{
					for(size_t x=0;x<width;++x)
					{
						const UVW uvw = uvws[x];
						const num_t sol = UVImager::SpeedOfLight();
						const double freq1 = metaData->Band().channels[y].frequencyHz / sol;
						const num_t
							u1 = uvw.u * freq1,
							v1 = uvw.v * freq1;
						//	w1 = uvw.w * freq1;
						num_t real = 0.0f, imaginary = 0.0f, weight = 1.0f;
						
						for(size_t yi=0;yi<height;++yi)
						{
							const double freq2 = metaData->Band().channels[yi].frequencyHz / sol;
							for(size_t xi=0;xi<width;++xi)
							{
								if(xi == x && yi == y)
								{
									real += copyReal->Value(xi, yi);
									imaginary += copyImag->Value(xi, yi);
								} else {
									const UVW uvwi = uvws[xi];
									const num_t
										du = uvwi.u * freq2 - u1,
										dv = uvwi.v * freq2 - v1;
										//dw = uvwi.w * freq2 - w1;
									const num_t dist = sqrtn(du*du + dv*dv) * factor; // + dw*dw
									const num_t sincVal = sinn(dist) / dist;
									/*if(xi == 0)
									{
										AOLogger::Debug << dist << '*' << factor << " -> " << sincVal << '\n';
									}*/
									
									real += sincVal * copyReal->Value(xi, yi);
									imaginary += sincVal * copyImag->Value(xi, yi);
									weight += sincVal;
								}
							}
						}
						rImage->SetValue(x, y, real / weight);
						iImage->SetValue(x, y, imaginary / weight);
					}
					listener.OnProgress(*this, y+1, height);
				}
			}
			
			enum KernelKind _kernelKind;
			numl_t _convolutionSize;
			bool _inSamples;
	};

} // namespace

#endif // RFI_FREQUENCY_CONVOLUTION_ACTION
