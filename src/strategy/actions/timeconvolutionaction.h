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
#ifndef RFI_TIME_CONVOLUTION_ACTION
#define RFI_TIME_CONVOLUTION_ACTION

#include "../algorithms/sinusfitter.h"
#include "../algorithms/convolutions.h"
#include "../algorithms/uvprojection.h"

#include "action.h"

#include "../control/actionblock.h"
#include "../control/artifactset.h"

#include "../../imaging/uvimager.h"

#include "../../util/aologger.h"
#include "../../util/ffttools.h"
#include "../../util/progresslistener.h"

#include <boost/concept_check.hpp>

namespace rfiStrategy {

	class TimeConvolutionAction : public Action
	{
		public:
			enum Operation { SingleSincOperation, SincOperation, ProjectedSincOperation, ProjectedFTOperation, ExtrapolatedSincOperation, IterativeExtrapolatedSincOperation, FFTSincOperation };
			
			TimeConvolutionAction() : Action(), _operation(IterativeExtrapolatedSincOperation), _sincSize(32.0), _directionRad(M_PI*(-86.7/180.0)), _etaParameter(0.2), _autoAngle(true), _isSincScaleInSamples(false), _alwaysRemove(false), _useHammingWindow(false), _iterations(1), _channelAveragingSize(4)
			{
			}
			virtual std::string Description()
			{
				switch(_operation)
				{
					case SingleSincOperation:
						return "Time sinc convolution (once)";
						break;
					case SincOperation:
						return "Time sinc convolution (round)";
						break;
					case ProjectedSincOperation:
						return "Projected sinc convolution";
						break;
					case ProjectedFTOperation:
						return "Projected Fourier transform";
						break;
					case ExtrapolatedSincOperation:
						return "Projected extrapolated sinc";
						break;
					case IterativeExtrapolatedSincOperation:
						return "Iterative projected extrapolated sinc";
						break;
					case FFTSincOperation:
						return "FFT sinc";
						break;
					default:
						return "?";
						break;
				}
			}
			virtual ActionType Type() const { return TimeConvolutionActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				Image2DCPtr newImage;
				TimeFrequencyData newRevisedData;
				switch(_operation)
				{
					case SingleSincOperation:
						newImage = PerformSingleSincOperation(artifacts);
						break;
					case SincOperation:
						newImage = PerformSincOperation(artifacts);
						break;
					case ProjectedSincOperation:
						newImage = PerformProjectedSincOperation(artifacts, listener);
						break;
					case ProjectedFTOperation:
					case ExtrapolatedSincOperation:
					case IterativeExtrapolatedSincOperation:
					{
						if(_autoAngle)
							_directionRad = FindStrongestSourceAngle(artifacts, artifacts.ContaminatedData());
						TimeFrequencyData data = artifacts.ContaminatedData();
						TimeFrequencyData *realData = data.CreateTFData(TimeFrequencyData::RealPart);
						TimeFrequencyData *imagData = data.CreateTFData(TimeFrequencyData::ImaginaryPart);
						Image2DPtr real = Image2D::CreateCopy(realData->GetSingleImage());
						Image2DPtr imaginary = Image2D::CreateCopy(imagData->GetSingleImage());
						delete realData;
						delete imagData;
						PerformExtrapolatedSincOperation(artifacts, real, imaginary, listener);
						newRevisedData = TimeFrequencyData(data.Polarisation(), real, imaginary);
					}
					break;
					case FFTSincOperation:
						TimeFrequencyData data = artifacts.ContaminatedData();
						TimeFrequencyData *realData = data.CreateTFData(TimeFrequencyData::RealPart);
						TimeFrequencyData *imagData = data.CreateTFData(TimeFrequencyData::ImaginaryPart);
						Image2DPtr real = Image2D::CreateCopy(realData->GetSingleImage());
						Image2DPtr imaginary = Image2D::CreateCopy(imagData->GetSingleImage());
						delete realData;
						delete imagData;
						PerformFFTSincOperation(artifacts, real, imaginary);
						newRevisedData = TimeFrequencyData(data.Polarisation(), real, imaginary);
						break;
				}
				
				if(_operation == SingleSincOperation || _operation == SincOperation || _operation == ProjectedSincOperation)
				{
					newRevisedData = TimeFrequencyData(artifacts.ContaminatedData().PhaseRepresentation(), artifacts.ContaminatedData().Polarisation(), newImage);
				}

				newRevisedData.SetMask(artifacts.RevisedData());

				TimeFrequencyData *contaminatedData =
					TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), newRevisedData);
				contaminatedData->SetMask(artifacts.ContaminatedData());
				artifacts.SetRevisedData(newRevisedData);
				artifacts.SetContaminatedData(*contaminatedData);
				delete contaminatedData;
			}
			
			enum Operation Operation() const { return _operation; }
			void SetOperation(enum Operation operation) { _operation = operation; }

			num_t DirectionRad() const { return _directionRad; }
			void SetDirectionRad(num_t directionRad) { _directionRad = directionRad; }
			
			num_t SincScale() const { return _sincSize; }
			void SetSincScale(num_t sincSize) { _sincSize = sincSize; }

			num_t EtaParameter() const { return _etaParameter; }
			void SetEtaParameter(num_t etaParameter) { _etaParameter = etaParameter; }

			unsigned Iterations() const { return _iterations; }
			void SetIterations(unsigned iterations) { _iterations = iterations; }

			bool AutoAngle() const { return _autoAngle; }
			void SetAutoAngle(bool autoAngle) { _autoAngle = autoAngle; }

			bool IsSincScaleInSamples() const { return _isSincScaleInSamples; }
			void SetIsSincScaleInSamples(bool inSamples) { _isSincScaleInSamples = inSamples; }

			unsigned ChannelAveragingSize() const { return _channelAveragingSize; }
			void SetChannelAveragingSize(unsigned size) { _channelAveragingSize = size; }
			
			bool AlwaysRemove() const { return _alwaysRemove; }
			void SetAlwaysRemove(bool alwaysRemove) { _alwaysRemove = alwaysRemove; }
			
			bool UseHammingWindow() const { return _useHammingWindow; }
			void SetUseHammingWindow(bool useHammingWindow) { _useHammingWindow = useHammingWindow; }
private:
			class IterationData
			{
			public:
				ArtifactSet
					*artifacts;
				size_t
					width,
					fourierWidth,
					rangeStart,
					rangeEnd,
					vZeroPos,
					startXf,
					endXf;
				numl_t
					maxDist;
				numl_t
					*rowRValues,
					*rowIValues,
					*rowUPositions,
					*rowVPositions,
					*fourierValuesReal,
					*fourierValuesImag,
					*channelMaxDist;
				numl_t
					**fSinTable,
					**fCosTable;
					
				IterationData() :
					artifacts(0), width(0), fourierWidth(0), rangeStart(0), rangeEnd(0),
					vZeroPos(0), startXf(0), endXf(0),
					maxDist(0.0),
					rowRValues(0), rowIValues(0), rowUPositions(0), rowVPositions(0),
					fourierValuesReal(0), fourierValuesImag(0), channelMaxDist(0),
					fSinTable(0), fCosTable(0)
				{
				}
			};

			Image2DPtr PerformSingleSincOperation(ArtifactSet &artifacts) const
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				num_t *row = new num_t[image->Width()];
				Image2DPtr newImage = Image2D::CreateUnsetImagePtr(image->Width(), image->Height());
				unsigned width = image->Width();

				const BandInfo band = artifacts.MetaData()->Band();
				for(unsigned y=0;y<image->Height();++y)
				{
					const num_t sincScale = ActualSincScaleInSamples(artifacts, band.channels[y].frequencyHz);
					if(y == image->Height()/2)
					{
						AOLogger::Debug << "Horizontal sinc scale: " << sincScale << " (filter scale: " << Angle::ToString(ActualSincScaleAsRaDecDist(artifacts, band.channels[y].frequencyHz)) << ")\n";
					}
					if(sincScale > 1.0)
					{
						for(unsigned x=0;x<width;++x)
							row[x] = image->Value(x, y);
						if(_useHammingWindow)
							Convolutions::OneDimensionalSincConvolutionHammingWindow(row, width, 1.0 / sincScale);
						else
							Convolutions::OneDimensionalSincConvolution(row, width, 1.0 / sincScale);
						for(unsigned x=0;x<width;++x)
							newImage->SetValue(x, y, row[x]);
					} else {
						for(unsigned x=0;x<width;++x)
							newImage->SetValue(x, y, image->Value(x, y));
					}
				}
				delete[] row;
				
				return newImage;
			}
			
			void PerformFFTSincOperation(ArtifactSet &artifacts, Image2DPtr real, Image2DPtr imag) const;
			
			Image2DPtr PerformSincOperation(ArtifactSet &artifacts) const
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				num_t *row = new num_t[image->Width()*3];
				Image2DPtr newImage = Image2D::CreateUnsetImagePtr(image->Width(), image->Height());
				unsigned width = image->Width();
				num_t sign;
				if(data.IsImaginary())
					sign = -1.0;
				else
					sign = 1.0;
				const BandInfo band = artifacts.MetaData()->Band();
				for(unsigned y=0;y<image->Height();++y)
				{
					const num_t sincScale = ActualSincScaleInSamples(artifacts, band.channels[y].frequencyHz);
					if(y == image->Height()/2)
					{
						AOLogger::Debug << "Horizontal sinc scale: " << sincScale << " (filter scale: " << Angle::ToString(ActualSincScaleAsRaDecDist(artifacts, band.channels[y].frequencyHz)) << ")\n";
					}
					if(sincScale > 1.0)
					{
						for(unsigned x=0;x<width;++x) {
							row[x] = sign * image->Value(x, y);
							row[x+width] = image->Value(x, y);
							row[x+2*width] = sign * image->Value(x, y);
						}
						if(_useHammingWindow)
							Convolutions::OneDimensionalSincConvolutionHammingWindow(row, width*3, 1.0 / sincScale);
						else
							Convolutions::OneDimensionalSincConvolution(row, width*3, 1.0 / sincScale);
						for(unsigned x=0;x<width;++x)
							newImage->SetValue(x, y, row[x+width]);
					} else {
						for(unsigned x=0;x<width;++x)
							newImage->SetValue(x, y, image->Value(x, y));
					}
				}
				delete[] row;
				
				return newImage;
			}
			
			Image2DPtr PerformProjectedSincOperation(ArtifactSet &artifacts, class ProgressListener &listener) const
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				Image2DPtr newImage = Image2D::CreateUnsetImagePtr(image->Width(), image->Height());
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				bool isImaginary = data.IsImaginary();
				const size_t width = image->Width();
				const BandInfo band = artifacts.MetaData()->Band();

				for(size_t y=0;y<image->Height();++y)
				{
					const num_t sincScale = ActualSincScaleInLambda(artifacts, band.channels[y].frequencyHz);
					listener.OnProgress(*this, y, image->Height());
					
					numl_t
						*rowValues = new numl_t[width],
						*rowUPositions = new numl_t[width],
						*rowVPositions = new numl_t[width];

					UVProjection::ProjectPositions(metaData, width, y, rowUPositions, rowVPositions, _directionRad);
					
					UVProjection::Project(image, y, rowValues, isImaginary);

					// Perform the convolution
					for(size_t t=0;t<width;++t)
					{
						const numl_t pos = rowUPositions[t];

						numl_t valueSum = 0.0;
						numl_t weightSum = 0.0;
						
						for(size_t x=0;x<width;++x)
						{
							// if this is exactly a point on the u axis, this point is ignored
							// (it would have infinite weight)
							const UVW &uvw = metaData->UVW()[x];
							if(uvw.v != 0.0) 
							{
								//const numl_t weight = fabsnl(uvw.u / uvw.v);
								const numl_t dist = (rowUPositions[x] - pos) / sincScale;
								if(dist!=0.0)
								{
									const numl_t sincValue = sinnl(dist) / dist;
									valueSum += sincValue * rowValues[x];// * weight;
									weightSum += sincValue;
								}
								else
								{
									valueSum += rowValues[x];// * weight;
									weightSum += 1.0;
								}
							}
						}
						newImage->SetValue(t, y, (num_t) (valueSum / weightSum));
					}
					
					delete[] rowValues;
					delete[] rowUPositions;
					delete[] rowVPositions;
				}
				listener.OnProgress(*this, image->Height(), image->Height());
				
				return newImage;
			}

			void Project(class IterationData &iterData, Image2DCPtr real, Image2DCPtr imaginary, size_t yStart, size_t yEnd) const
			{
				const size_t width = iterData.width;
				iterData.rowRValues = new numl_t[width];
				iterData.rowIValues = new numl_t[width];
				iterData.rowUPositions = new numl_t[width];
				iterData.rowVPositions = new numl_t[width];

				for(size_t x=0;x<width;++x)
				{
					iterData.rowRValues[x] = 0.0;
					iterData.rowIValues[x] = 0.0;
					iterData.rowUPositions[x] = 0.0;
					iterData.rowVPositions[x] = 0.0;
				}

				iterData.maxDist = 0.0;

				std::vector<numl_t>
					rowRValues(width), rowIValues(width),
					rowUPositions(width), rowVPositions(width);
				numl_t
					yL = (yEnd - yStart);

				// We average all values returned by Project() over yStart to yEnd
				for(size_t y=yStart;y<yEnd;++y)
				{
					UVProjection::ProjectPositions(iterData.artifacts->MetaData(), width, y, &rowUPositions[0], &rowVPositions[0], _directionRad);
					
					UVProjection::Project(real, y, &rowRValues[0], false);
					UVProjection::Project(imaginary, y, &rowIValues[0], true);

					for(size_t x=0;x<width;++x)
					{
						iterData.rowRValues[x] += rowRValues[x] / yL;
						iterData.rowIValues[x] += rowIValues[x] / yL;
						iterData.rowUPositions[x] += rowUPositions[x] / yL;
						iterData.rowVPositions[x] += rowVPositions[x] / yL;
					}

					// Find the point closest to v=0
					numl_t vDist = fabsnl(rowVPositions[0]);
					size_t vZeroPos = 0;
					for(unsigned i=1;i<width;++i)
					{
						if(fabsnl(rowVPositions[i]) < vDist)
						{
							vDist = fabsnl(rowVPositions[i]);
							vZeroPos = i;
						}
					}
					iterData.vZeroPos = vZeroPos;
					iterData.channelMaxDist[y] = fabsnl(rowUPositions[vZeroPos]);
					iterData.maxDist += iterData.channelMaxDist[y] / yL;

					//AOLogger::Debug << "v is min at t=" << vZeroPos << " (v=+-" << vDist << ", maxDist=" << iterData.channelMaxDist[y] << ")\n";
				}
			}

			void FreeProjectedValues(class IterationData &iterData) const
			{
				delete[] iterData.rowRValues;
				delete[] iterData.rowIValues;
				delete[] iterData.rowUPositions;
				delete[] iterData.rowVPositions;
			}

			void PrecalculateFTFactors(class IterationData &iterData) const
			{
				const size_t
					width = iterData.width,
					fourierWidth = iterData.fourierWidth,
					rangeStart = iterData.rangeStart,
					rangeEnd = iterData.rangeEnd;

				iterData.fSinTable = new numl_t*[fourierWidth],
				iterData.fCosTable = new numl_t*[fourierWidth];
				for(size_t xF=0;xF<fourierWidth;++xF)
				{
					iterData.fSinTable[xF] = new numl_t[rangeEnd - rangeStart];
					iterData.fCosTable[xF] = new numl_t[rangeEnd - rangeStart];
					// F(xF) = \\int f(u) * e^(-i 2 \\pi * u_n * xF_n)
					// xF \\in [0 : fourierWidth] -> xF_n = 2 xF / fourierWidth - 1 \\in [-1 : 1];
					// u \\in [-maxDist : maxDist] -> u_n = u * width / maxDist \\in [ -width : width ]
					// final frequenty domain covers [-maxDist : maxDist]
					const numl_t
						fourierPos = (numl_t) xF / fourierWidth - 0.5,
						fourierFactor = -fourierPos * 2.0 * M_PInl * width * 0.5 / iterData.maxDist;
					for(size_t tIndex=rangeStart;tIndex<rangeEnd;++tIndex)
					{
						size_t t = (tIndex + width - iterData.vZeroPos) % width;
						const numl_t posU = iterData.rowUPositions[t];
						iterData.fCosTable[xF][tIndex-rangeStart] = cosnl(fourierFactor * posU);
						iterData.fSinTable[xF][tIndex-rangeStart] = sinnl(fourierFactor * posU);
					}
				}
			}

			void FreeFTFactors(class IterationData &iterData) const
			{
				for(size_t xF=0;xF<iterData.fourierWidth;++xF)
				{
					delete[] iterData.fSinTable[xF];
					delete[] iterData.fCosTable[xF];
				}
				delete[] iterData.fSinTable;
				delete[] iterData.fCosTable;
			}

			void PerformFourierTransform(class IterationData &iterData, Image2DPtr real, Image2DPtr imaginary, size_t yStart, size_t yEnd) const
			{
				const size_t
					rangeStart = iterData.rangeStart,
					rangeEnd = iterData.rangeEnd,
					width = iterData.width,
					vZeroPos = iterData.vZeroPos;

				// Perform a 1d Fourier transform, ignoring eta part of the data
				for(size_t xF=0;xF<iterData.fourierWidth;++xF)
				{
					numl_t
						realVal = 0.0,
						imagVal = 0.0,
						weightSum = 0.0;
					const numl_t
						*fCosTable = iterData.fCosTable[xF],
						*fSinTable = iterData.fSinTable[xF];
					// compute F(xF) = \\int f(x) * exp( -2 * \\pi * i * x * xF )
					for(size_t tIndex=rangeStart;tIndex<rangeEnd;++tIndex)
					{
						size_t t = (tIndex + width - vZeroPos) % width;
						if(iterData.rowUPositions[t] != 0.0)
						{
							const numl_t weight = 1.0;//fabsnl(rowVPositions[t]/posU);
							const numl_t weightSqrt = 1.0;//sqrtnl(weight);
							realVal += (iterData.rowRValues[t] * fCosTable[tIndex-rangeStart] -
													iterData.rowIValues[t] * fSinTable[tIndex-rangeStart]) * weightSqrt;
							imagVal += (iterData.rowIValues[t] * fCosTable[tIndex-rangeStart] +
													iterData.rowRValues[t] * fSinTable[tIndex-rangeStart]) * weightSqrt;
							weightSum += weight;
						}
					}
					iterData.fourierValuesReal[xF] = realVal / weightSum;
					iterData.fourierValuesImag[xF] = imagVal / weightSum;
					if(_operation == ProjectedFTOperation)
					{
						for(size_t y=yStart;y!=yEnd;++y)
						{
							real->SetValue(xF/2, y, (num_t) realVal / weightSum);
							imaginary->SetValue(xF/2, y, (num_t) imagVal / weightSum);
						}
					}
				}
			}

			void InvFourierTransform(class IterationData &iterData, Image2DPtr real, Image2DPtr imaginary, size_t y) const
			{
				const size_t
					startXf = iterData.startXf,
					endXf = iterData.endXf,
					width = iterData.width,
					fourierWidth = iterData.fourierWidth;

				AOLogger::Debug << "Inv FT, using 0-" << startXf << " and " << endXf << "-" << fourierWidth << '\n';
				
				for(size_t t=0;t<width;++t)
				{
					const numl_t
						posU = iterData.rowUPositions[t],
						posV = iterData.rowVPositions[t],
						posFactor = posU * 2.0 * M_PInl;
					numl_t
						realVal = 0.0,
						imagVal = 0.0;
					bool residual = false;

					if(posV != 0.0)
					{
						const numl_t weightSum = 1.0; //(endXf - startXf); // * fabsnl(posV / posU);
						// compute f(x) = \\int F(xF) * exp( 2 * \\pi * i * x * xF )
						size_t xF, loopEnd;
						if(residual)
						{
							loopEnd = startXf;
							xF = 0;
						}
						else
						{
							loopEnd = endXf;
							xF = startXf;
						}
						while(xF < loopEnd)
						{
							const numl_t
								fourierPosL = (numl_t) xF / fourierWidth - 0.5,
								fourierRealL = iterData.fourierValuesReal[xF],
								fourierImagL = iterData.fourierValuesImag[xF];

							const numl_t
								cosValL = cosnl(fourierPosL * posFactor),
								sinValL = sinnl(fourierPosL * posFactor);

							realVal += fourierRealL * cosValL - fourierImagL * sinValL;
							imagVal += fourierImagL * cosValL + fourierRealL * sinValL;

							if(residual)
							{
								const numl_t
									fourierPosR = (numl_t) (endXf + xF) / fourierWidth - 0.5,
									fourierRealR = iterData.fourierValuesReal[endXf + xF],
									fourierImagR = iterData.fourierValuesImag[endXf + xF];

								const numl_t
									cosValR = cosnl(fourierPosR * posFactor),
									sinValR = sinnl(fourierPosR * posFactor);

								realVal += fourierRealR * cosValR - fourierImagR * sinValR;
								imagVal += fourierImagR * cosValR + fourierRealR * sinValR;
							}
							++xF;
						}
						real->SetValue(t, y, -realVal/weightSum);
						imaginary->SetValue(t, y, -imagVal/weightSum);
					}
				}
			}

			size_t FindStrongestComponent(const class IterationData &iterData, bool withinBounds) const
			{
				const size_t
					startXf = iterData.startXf,
					endXf = iterData.endXf;
				size_t xFRemoval = 0;
				numl_t xFValue =
					iterData.fourierValuesReal[0]*iterData.fourierValuesReal[0] + iterData.fourierValuesImag[0]*iterData.fourierValuesImag[0];
				if(withinBounds)
				{
					AOLogger::Debug << "Limiting search to xF<" << startXf << " and xF>" << endXf << '\n'; 
					for(size_t xF=0;xF<startXf;++xF)
					{
						numl_t
							fReal = iterData.fourierValuesReal[xF],
							fImag = iterData.fourierValuesImag[xF],
							val = fReal*fReal + fImag*fImag;
						if(val > xFValue)
						{
							xFRemoval = xF;
							xFValue = val;
						}
						fReal = iterData.fourierValuesReal[xF+endXf];
						fImag = iterData.fourierValuesImag[xF+endXf];
						val = fReal*fReal + fImag*fImag;
						if(val > xFValue)
						{
							xFRemoval = xF+endXf;
							xFValue = val;
						}
					}
				} else {
					const size_t fourierWidth = iterData.fourierWidth;
					const numl_t p = 0.1;

					for(size_t xF=0;xF<fourierWidth;++xF)
					{
						const numl_t
							fReal = iterData.fourierValuesReal[xF],
							fImag = iterData.fourierValuesImag[xF];
						numl_t
							val = fReal*fReal + fImag*fImag;
						if(xF >= startXf && xF <= endXf)
							val *= p + (1.0 - p) * fabsnl(2.0*xF - (numl_t) fourierWidth)/((numl_t) (endXf-startXf));
						if(val > xFValue)
						{
							xFRemoval = xF;
							xFValue = val;
						}
					}
				}

				return xFRemoval;
			}

			void RemoveFourierComponent(class IterationData &iterData, Image2DPtr real, Image2DPtr imaginary, size_t y, numl_t fourierFactor, numl_t fReal, numl_t fImag, bool applyOnImages) const
			{
				const size_t
					width = iterData.width;

				for(size_t t=0;t<width;++t)
				{
					const numl_t
						posU = iterData.rowUPositions[t],
						weightSum = 1.0;
						
					const numl_t
						cosValL = cosnl(-fourierFactor * posU),
						sinValL = sinnl(-fourierFactor * posU);

					numl_t realVal = (fReal * cosValL + fImag * sinValL) * 0.75 / weightSum;
					numl_t imagVal = (fReal * sinValL + fImag * cosValL) * 0.75 / weightSum;
					
					if(applyOnImages)
					{
						real->SetValue(t, y, real->Value(t, y) - realVal);
						imaginary->SetValue(t, y, imaginary->Value(t, y) - imagVal);
					} else {
						iterData.rowRValues[t] -= realVal;
						iterData.rowIValues[t] -= imagVal;
					}
				}
			}
			
			void PerformExtrapolatedSincOperation(ArtifactSet &artifacts, Image2DPtr real, Image2DPtr imaginary, class ProgressListener &listener) const
			{
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				const size_t width = real->Width();
				const BandInfo band = artifacts.MetaData()->Band();
					
				class IterationData iterData;

				iterData.artifacts = &artifacts;

				iterData.width = width;
				iterData.fourierWidth = width * 2;
				iterData.fourierValuesReal = new numl_t[iterData.fourierWidth];
				iterData.fourierValuesImag = new numl_t[iterData.fourierWidth];
				iterData.channelMaxDist = new numl_t[real->Height()];

				iterData.rangeStart = (size_t) roundn(_etaParameter * (num_t) width / 2.0),
				iterData.rangeEnd = width - iterData.rangeStart;

				for(size_t y=0;y<real->Height();y+=_channelAveragingSize)
				{
					listener.OnProgress(*this, y, real->Height());

					Project(iterData, real, imaginary, y, y+_channelAveragingSize);
					
					PrecalculateFTFactors(iterData);

					for(unsigned iteration=0;iteration<_iterations;++iteration)
					{
						PerformFourierTransform(iterData, real, imaginary, y, y+_channelAveragingSize);
					
						numl_t sincScale = ActualSincScaleInLambda(artifacts, band.channels[y].frequencyHz);
						numl_t clippingFrequency = 1.0/(sincScale * width / iterData.maxDist);
						long fourierClippingIndex =
							(long) ceilnl((numl_t) iterData.fourierWidth * 0.5 * clippingFrequency);
						if(fourierClippingIndex*2 > (long) iterData.fourierWidth)
							fourierClippingIndex = iterData.fourierWidth/2;
						if(fourierClippingIndex < 0)
							fourierClippingIndex = 0;
						iterData.startXf = iterData.fourierWidth/2 - fourierClippingIndex,
						iterData.endXf = iterData.fourierWidth/2 + fourierClippingIndex;

						if(_operation == ExtrapolatedSincOperation)
						{
							InvFourierTransform(iterData, real, imaginary, y);
						}
						else if(_operation == IterativeExtrapolatedSincOperation)
						{
							const size_t xFRemoval = FindStrongestComponent(iterData, false);
							const numl_t
									fReal = iterData.fourierValuesReal[xFRemoval],
									fImag = iterData.fourierValuesImag[xFRemoval],
									xFValue = sqrtnl(fReal*fReal + fImag*fImag);
							AOLogger::Debug << "Removing frequency at xF=" << xFRemoval << ", amp=" << xFValue << '\n';
							AOLogger::Debug << "Amplitude = sigma x " << (xFValue / GetAverageAmplitude(iterData)) << '\n';

							if(xFRemoval < iterData.startXf || xFRemoval > iterData.endXf || _alwaysRemove)
							{
								if(!_alwaysRemove)
									AOLogger::Debug << "Within bounds 0-" << iterData.startXf << '/' << iterData.endXf << "-.. removing from image.\n";
								// Now, remove the fringe from each channel 
								for(size_t yI = y; yI != y+_channelAveragingSize; ++yI)
								{
									const numl_t
										channelFactor = iterData.channelMaxDist[yI] / iterData.maxDist,
										fourierPos = (numl_t) xFRemoval / iterData.fourierWidth - 0.5,
										fourierFactor = fourierPos * 2.0 * M_PInl * width * 0.5 / iterData.maxDist * channelFactor;
		
									RemoveFourierComponent(iterData, real, imaginary, yI, fourierFactor, fReal, fImag, true);
								}
							}

							// Subtract fringe from average value
							const numl_t
								fourierPos = (numl_t) xFRemoval / iterData.fourierWidth - 0.5,
								fourierFactor = fourierPos * 2.0 * M_PInl * width * 0.5 / iterData.maxDist;
							RemoveFourierComponent(iterData, real, imaginary, y, fourierFactor, fReal, fImag, false);
						}
					}
					FreeFTFactors(iterData);
					FreeProjectedValues(iterData);
				}
				listener.OnProgress(*this, real->Height(), real->Height());

				delete[] iterData.fourierValuesReal;
				delete[] iterData.fourierValuesImag;
				delete[] iterData.channelMaxDist;
			}

			numl_t avgUVDistance(ArtifactSet &artifacts, const double frequencyHz) const
			{
				return UVImager::UVTrackLength(artifacts.MetaData(), frequencyHz);
			}

			numl_t ActualSincScaleInSamples(ArtifactSet &artifacts, const double frequencyHz) const
			{
				if(_isSincScaleInSamples)
					return _sincSize;
				else
					return _sincSize / avgUVDistance(artifacts, frequencyHz) * (numl_t) artifacts.ContaminatedData().ImageWidth();
			}

			numl_t ActualSincScaleInLambda(ArtifactSet &artifacts, const double frequencyHz) const
			{
				if(_isSincScaleInSamples)
					return _sincSize / ((numl_t) artifacts.ContaminatedData().ImageWidth()) * avgUVDistance(artifacts, frequencyHz);
				else
					return _sincSize;
			}
			
			numl_t ActualSincScaleAsRaDecDist(ArtifactSet &artifacts, const double frequencyHz) const
			{
				return 1.0/ActualSincScaleInLambda(artifacts, frequencyHz);
			}

			numl_t GetAverageAmplitude(class IterationData &iterData) const
			{
				const size_t
					startXf = iterData.startXf,
					endXf = iterData.endXf;

				numl_t sum = 0.0;
				for(size_t xF=0;xF<startXf;++xF)
				{
					numl_t
						fReal = iterData.fourierValuesReal[xF],
						fImag = iterData.fourierValuesImag[xF];
					sum += sqrtnl(fReal*fReal + fImag*fImag);

					fReal = iterData.fourierValuesReal[xF+endXf];
					fImag = iterData.fourierValuesImag[xF+endXf];
					sum += sqrtnl(fReal*fReal + fImag*fImag);
				}

				return sum / (numl_t) (startXf * 2);
			}

			numl_t FindStrongestSourceAngle(ArtifactSet &artifacts, const TimeFrequencyData &data)
			{
				UVImager imager(1024*3, 1024*3);
				imager.Image(data, artifacts.MetaData());
				imager.PerformFFT();
				Image2DPtr image(FFTTools::CreateAbsoluteImage(imager.FTReal(), imager.FTImaginary()));
				const numl_t centralFreq = artifacts.MetaData()->Band().channels[data.ImageHeight()/2].frequencyHz;
				AOLogger::Debug << "Central frequency: " << centralFreq << "\n";
				AOLogger::Debug << "Baseline length: " << artifacts.MetaData()->Baseline().Distance() << '\n';
				AOLogger::Debug << "Sinc scale in lambda: " << ActualSincScaleInLambda(artifacts, centralFreq) << '\n';
				AOLogger::Debug << "Average distance: " << avgUVDistance(artifacts, centralFreq) << '\n';
				const numl_t sincDist = ActualSincScaleAsRaDecDist(artifacts, centralFreq);
				numl_t ignoreRadius = sincDist / imager.UVScaling();
				AOLogger::Debug << "Ignoring radius=" << ignoreRadius << "\n";

				long maxX = 0, maxY = 0;
				num_t maxValue = image->Value(maxX, maxY);
				for(unsigned y=0;y<image->Height();++y)
				{
					for(unsigned x=0;x<image->Width();++x)
					{
						if(image->Value(x, y) > maxValue)
						{
							int x_r = (x*2 - image->Width())/2;
							int y_r = (image->Height() - y*2)/2;
							numl_t distSqr = x_r*x_r + y_r*y_r;
							if(distSqr > ignoreRadius * ignoreRadius)
							{
								maxValue = image->Value(x, y);
								maxX = x;
								maxY = y;
							}
						}
					}
				}
				maxX = maxX*2-image->Width();
				maxY = image->Height() - maxY*2;
				numl_t angle = SinusFitter::Phase((numl_t) maxX, (numl_t) maxY);
 				AOLogger::Debug << "Angle: " << angle/M_PInl*180.0 << ",maxX=" << maxX << ",maxY=" << maxY << '\n';
				return angle;
			}

			enum Operation _operation;
			num_t _sincSize, _directionRad, _etaParameter;
			bool _autoAngle, _isSincScaleInSamples, _alwaysRemove, _useHammingWindow;
			unsigned _iterations, _channelAveragingSize;
	};

} // namespace

#endif // RFI_TIME_CONVOLUTION_ACTION
