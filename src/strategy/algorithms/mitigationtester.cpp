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
#include "mitigationtester.h"

#include <iostream>
#include <sstream>

#include <math.h>

#include "../../msio/image2d.h"
#include "../../msio/pngfile.h"

#include "../../util/aologger.h"
#include "../../util/ffttools.h"
#include "../../util/stopwatch.h"

#include "../../imaging/model.h"
#include "../../imaging/observatorium.h"

#include "../../types.h"

#include "localfitmethod.h"
#include "thresholdmitigater.h"

MitigationTester::MitigationTester() : _real(), _imaginary()
{
}


MitigationTester::~MitigationTester()
{
	Clear();
}

void MitigationTester::Clear()
{
	_real.reset();
	_imaginary.reset();
}

void MitigationTester::GenerateNoise(size_t scanCount, size_t frequencyCount, bool independentComplex, double sigma, enum NoiseType noiseType)
{
	Clear();

	_real = Image2D::CreateUnsetImagePtr(scanCount, frequencyCount);
	_imaginary = Image2D::CreateUnsetImagePtr(scanCount, frequencyCount);

	if(independentComplex) {
		for(size_t f=0; f<frequencyCount;++f) {
			for(size_t t=0; t<scanCount;++t) {
				_real->SetValue(t, f, Rand(noiseType)*sigma);
				_imaginary->SetValue(t, f, Rand(noiseType)*sigma);
			}
		}
	} else {
		for(size_t f=0; f<frequencyCount;++f) {
			for(size_t t=0; t<scanCount;++t) {
				double r = Rand(noiseType)*sigma;
				_real->SetValue(t, f, r);
				_imaginary->SetValue(t, f, r);
			}
		}
	}
}

void MitigationTester::AddBroadbandLine(Image2DPtr data, Mask2DPtr rfi, double lineStrength, size_t startTime, size_t duration, double frequencyRatio, double frequencyOffsetRatio)
{
	size_t frequencyCount = data->Height();
	unsigned fStart = (size_t) (frequencyOffsetRatio * frequencyCount);
	unsigned fEnd = (size_t) ((frequencyOffsetRatio + frequencyRatio) * frequencyCount);
	AddBroadbandLinePos(data, rfi, lineStrength, startTime, duration, fStart, fEnd, UniformShape);
}

void MitigationTester::AddBroadbandLinePos(Image2DPtr data, Mask2DPtr rfi, double lineStrength, size_t startTime, size_t duration, unsigned frequencyStart, double frequencyEnd, enum BroadbandShape shape)
{
	const double s = (frequencyEnd-frequencyStart);
	for(size_t f=frequencyStart;f<frequencyEnd;++f) {	
		// x will run from -1 to 1
		const double x = (double) ((f-frequencyStart)*2)/s-1.0;
		double factor = shapeLevel(shape, x);
		for(size_t t=startTime;t<startTime+duration;++t) {
			data->AddValue(t, f, lineStrength * factor);
			if(lineStrength > 0.0)
				rfi->SetValue(t, f, true);
		}
	}
}

void MitigationTester::AddSlewedBroadbandLinePos(Image2DPtr data, Mask2DPtr rfi, double lineStrength, double slewrate, size_t startTime, size_t duration, unsigned frequencyStart, double frequencyEnd, enum BroadbandShape shape)
{
	const double s = (frequencyEnd-frequencyStart);
	for(size_t f=frequencyStart;f<frequencyEnd;++f) {	
			// x will run from -1 to 1
		const double x = (double) ((f-frequencyStart)*2)/s-1.0;
		double factor = shapeLevel(shape, x);
		double slew = slewrate * (double) f;
		size_t slewInt = (size_t) slew;
		double slewRest = slew - slewInt;
		
		data->AddValue(startTime+slewInt, f, lineStrength * factor * (1.0 - slewRest));
		if(lineStrength > 0.0)
			rfi->SetValue(startTime+slewInt, f, true);
		for(size_t t=startTime+1;t<startTime+duration;++t) {
			data->AddValue(t+slewInt, f, lineStrength * factor);
			if(lineStrength > 0.0)
				rfi->SetValue(t+slewInt, f, true);
		}
		data->AddValue(startTime+duration+slewInt, f, lineStrength * factor * slewRest);
		if(lineStrength > 0.0)
			rfi->SetValue(startTime+duration+slewInt, f, true);
	}
}

void MitigationTester::AddRfiPos(Image2DPtr data, Mask2DPtr rfi, double lineStrength, size_t startTime, size_t duration, unsigned frequencyPos)
{
	for(size_t t=startTime;t<startTime+duration;++t) {
		data->AddValue(t, frequencyPos, lineStrength);
		if(lineStrength > 0)
			rfi->SetValue(t, frequencyPos, true);
	}
}

void MitigationTester::AddRFI(size_t &rfiCount)
{
	size_t scanCount = _real->Width();
	size_t frequencyCount = _real->Height();
	// Continuous wideband lines
	size_t t1 = scanCount / 4;
	size_t t2 = scanCount / 2;
	double line1Strength = 1.5;
	double line2Strength = 2.0;
	for(size_t f=0;f<frequencyCount/2;++f) {
		size_t f1 = f+frequencyCount/4;
		size_t f2 = f+frequencyCount/3;
		// Fat line
		_real->AddValue(t1-1, f1, line1Strength);
		_imaginary->AddValue(t1-1, f1, line1Strength);
		_real->AddValue(t1, f1, line1Strength);
		_imaginary->AddValue(t1, f1, line1Strength);
		_real->AddValue(t1+1, f1, line1Strength);
		_imaginary->AddValue(t1+1, f1, line1Strength);
		// Thin line
		_real->AddValue(t2, f2, line2Strength);
		_imaginary->AddValue(t2, f2, line2Strength);
	}

	// Make 10 exceptional high rfi-dots of spectral width "3" x time "2"
	for(size_t i=0;i<10;++i) {
		double pointStrength = 3.0 + 12.0 * RNG::Uniform();
		size_t
			x = (size_t) (RNG::Uniform()*scanCount),
			y = (size_t) (RNG::Uniform()*(frequencyCount-2));
		_real->AddValue(x, y, pointStrength);
		_imaginary->AddValue(x, y, pointStrength);
		_real->AddValue(x, y+1, pointStrength);
		_imaginary->AddValue(x, y+1, pointStrength);
		_real->AddValue(x, y+2, pointStrength);
		_imaginary->AddValue(x, y+2, pointStrength);
		_real->AddValue(x+1, y, pointStrength);
		_imaginary->AddValue(x+1, y, pointStrength);
		_real->AddValue(x+1, y+1, pointStrength);
		_imaginary->AddValue(x+1, y+1, pointStrength);
		_real->AddValue(x+1, y+2, pointStrength);
		_imaginary->AddValue(x+1, y+2, pointStrength);
	}

	rfiCount = (frequencyCount/2)*4 + 6*10;
}

void MitigationTester::SetZero()
{
	for(size_t y=0;y<_real->Height();++y) {
		for(size_t x=0;x<_real->Width();++x) {
			_real->SetValue(x, y, 0.0);
			_imaginary->SetValue(x, y, 0.0);
		}
	}
}

void MitigationTester::CountResults(Mask2DCPtr thresholdedMask, Mask2DCPtr originalRFI, size_t &correct, size_t &notfound, size_t &error)
{
	correct = 0;
	notfound = 0;
	error = 0;
	for(size_t y=0;y<thresholdedMask->Height();++y) {
		for(size_t x=0;x<thresholdedMask->Width();++x) {
			bool rfiTresholded = thresholdedMask->Value(x, y);
			bool rfiOriginal = originalRFI->Value(x, y);
			if(rfiTresholded && rfiOriginal) {
				correct++;
			} else if(!rfiTresholded && rfiOriginal) {
				notfound++;
			} else if(rfiTresholded && !rfiOriginal) {
				error++;
			}
		}
	}
}

void MitigationTester::CountCorrectRFI(Image2DCPtr tresholdedReal, Image2DCPtr tresholdedImaginary, size_t &correct, size_t &notfound, size_t &error)
{
	correct = 0;
	notfound = 0;
	error = 0;
	for(size_t y=0;y<tresholdedReal->Height();++y) {
		for(size_t x=0;x<tresholdedReal->Width();++x) {
			bool rfiTresholded = tresholdedReal->Value(x, y) != 0.0;
			bool rfiOriginal = _real->Value(x, y) != 0.0;
			if(rfiTresholded && rfiOriginal) {
				correct++;
			} else if(!rfiTresholded && rfiOriginal) {
				notfound++;
			} else if(rfiTresholded && !rfiOriginal) {
				error++;
			}
			rfiTresholded = tresholdedImaginary->Value(x, y) != 0.0;
			rfiOriginal = _imaginary->Value(x, y) != 0.0;
			if(rfiTresholded && rfiOriginal) {
				correct++;
			} else if(!rfiTresholded && rfiOriginal) {
				notfound++;
			} else if(rfiTresholded && !rfiOriginal) {
				error++;
			}
		}
	}
}

Image2D *MitigationTester::CreateRayleighData(unsigned width, unsigned height)
{
	Image2D *image = Image2D::CreateUnsetImage(width, height);
	for(unsigned y=0;y<height;++y) {
		for(unsigned x=0;x<width;++x) {
			image->SetValue(x, y, RNG::Rayleigh());
		}
	}
	return image; 
}

Image2D *MitigationTester::CreateGaussianData(unsigned width, unsigned height)
{
	Image2D *image = Image2D::CreateUnsetImage(width, height);
	for(unsigned y=0;y<height;++y) {
		for(unsigned x=0;x<width;++x) {
			image->SetValue(x, y, RNG::Gaussian());
		}
	}
	return image; 
}

std::string MitigationTester::GetTestSetDescription(int number)
{
	switch(number)
	{
		case 0: return "Image of all zero's";
		case 1: return "Image of all ones";
		case 2: return "Noise";
		case 3: return "Several broadband RFI contaminating all channels";
		case 4: return "Several broadband RFI contaminating a part of channels";
		case 5: return "Several broadband RFI contaminating a random part of channels";
		case 6: return "Several broadband RFI on a sine wave background";
		case 7: return "Several broadband lines on a background of rotating sine waves";
		case 8: return "Testset 7 with a background fit on the background";
		case 9: return "Testset 7 in the time-lag domain";
		case 10: return "Identity matrix";
		case 11: return "FFT of Identity matrix";
		case 12: return "Broadband RFI contaminating all channels";
		case 13: return "Model of three point sources with broadband RFI";
		case 14: return "Model of five point sources with broadband RFI";
		case 15: return "Model of five point sources with partial broadband RFI";
		case 16: return "Model of five point sources with random broadband RFI";
		case 17: return "Background-fitted model of five point sources with random broadband RFI";
		case 18: return "Model of three point sources with random RFI"; 
		case 19: return "Model of three point sources with noise";
		case 20: return "Model of five point sources with noise";
		case 21: return "Model of three point sources";
		case 22: return "Model of five point sources";
		case 26: return "Gaussian lines";
		default: return "?";
	}
}

Image2DPtr MitigationTester::CreateTestSet(int number, Mask2DPtr rfi, unsigned width, unsigned height, int gaussianNoise)
{
	Image2DPtr image;
	switch(number)
	{
		case 0: // Image of all zero's
		return Image2D::CreateZeroImagePtr(width, height);
		case 1: // Image of all ones
		image = Image2D::CreateUnsetImagePtr(width, height);
		image->SetAll(1.0);
		break;
		case 2: // Noise
		return Image2DPtr(CreateNoise(width, height, gaussianNoise));
		case 3: { // Several broadband lines
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddBroadbandToTestSet(image, rfi, 1.0);
		} break;
		case 4: { // Several broadband lines
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddBroadbandToTestSet(image, rfi, 0.5);
		} break;
		case 5: { // Several broadband lines of random length
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddVarBroadbandToTestSet(image, rfi);
		} break;
		case 6: { // Different broadband lines + low freq background
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddVarBroadbandToTestSet(image, rfi);
		for(unsigned y=0;y<image->Height();++y) {
			for(unsigned x=0;x<image->Width();++x) {
				image->AddValue(x, y, sinn((long double) x*M_PIn*5.0 / image->Width()) + 0.1);
			}
		}
		} break;
		case 7: { // Different broadband lines + high freq background 
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		for(unsigned y=0;y<image->Height();++y) {
			for(unsigned x=0;x<image->Width();++x) {
				image->AddValue(x, y, sinn((long double) (x+y*0.1)*M_PIn*5.0L / image->Width() + 0.1));
				image->AddValue(x, y, sinn((long double) (x+pown(y, 1.1))*M_PIn*50.0L / image->Width() + 0.1));
			}
		}
		AddVarBroadbandToTestSet(image, rfi);
		for(unsigned y=0;y<image->Height();++y) {
			for(unsigned x=0;x<image->Width();++x) {
				image->AddValue(x, y, 1.0); 
			}
		}
		} break;
		case 8: {  // Different droadband lines + smoothed&subtracted high freq background
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		for(unsigned y=0;y<image->Height();++y) {
			for(unsigned x=0;x<image->Width();++x) {
				image->AddValue(x, y, sinn((num_t) (x+y*0.1)*M_PIn*5.0 / image->Width() + 0.1));
				image->AddValue(x, y, sinn((num_t) (x+pown(y, 1.1))*M_PIn*50.0 / image->Width() + 0.1));
			}
		}
		SubtractBackground(image);
		AddVarBroadbandToTestSet(image, rfi);
		} break;
		case 9: { //FFT of 7
		image=CreateTestSet(7, rfi, width, height);
		//FFTTools::Sqrt(*image);
		Image2D *copy = Image2D::CreateCopy(*image);
		FFTTools::CreateHorizontalFFTImage(*image, *copy, false);
		delete copy;
		for(unsigned y=0;y<rfi->Height();++y) {
			for(unsigned x=0;x<rfi->Width();++x) {
				image->SetValue(x, y, image->Value(x, y) / sqrtn(image->Width()));
			}
		}
		} break;
		case 10: { // Identity matrix
		image=Image2D::CreateZeroImagePtr(width, height);
		unsigned min = width < height ? width : height;
		for(unsigned i=0;i<min;++i) {
			image->SetValue(i, i, 1.0);
			rfi->SetValue(i, i, true);
		}
		} break;
		case 11: { // FFT of identity matrix
		image=CreateTestSet(10, rfi, width, height);
		Image2D *copy = Image2D::CreateCopy(*image);
		FFTTools::CreateHorizontalFFTImage(*image, *copy, false);
		delete copy;
		for(unsigned y=0;y<rfi->Height();++y) {
			for(unsigned x=0;x<rfi->Width();++x) {
				image->SetValue(x, y, image->Value(x, y) / sqrtn(width)); 
			}
		}
		} break;
		case 12: { // Broadband contaminating all channels
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		for(unsigned y=0;y<image->Height();++y) {
			for(unsigned x=0;x<image->Width();++x) {
				image->AddValue(x, y, sinn((num_t) (x+y*0.1)*M_PIn*5.0 / image->Width() + 0.1));
				image->AddValue(x, y, sinn((num_t) (x+powl(y, 1.1))*M_PIn*50.0 / image->Width() + 0.1));
			}
		}
		AddBroadbandToTestSet(image, rfi, 1.0);
		} break;
		case 13: { // Model of three point sources with broadband RFI
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 3);
		AddBroadbandToTestSet(image, rfi, 1.0L);
		} break;
		case 14: { // Model of five point sources with broadband RFI
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 5);
		AddBroadbandToTestSet(image, rfi, 1.0L);
		} break;
		case 15: { // Model of five point sources with partial broadband RFI
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 5);
		AddBroadbandToTestSet(image, rfi, 0.5L);
		} break;
		case 16: { // Model of five point sources with random broadband RFI
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 5);
		AddVarBroadbandToTestSet(image, rfi);
		} break;
		case 17: { // Background-fitted model of five point sources with random broadband RFI
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 5);
		SubtractBackground(image);
		AddVarBroadbandToTestSet(image, rfi);
		} break;
		case 18: { // Model of three point sources with random RFI
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 3);
		AddVarBroadbandToTestSet(image, rfi);
		} break;
		case 19: { // Model of three point sources with noise
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 3);
		} break;
		case 20: { // Model of five point sources with noise
		image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
		AddModelData(image, 5);
		} break;
		case 21: { // Model of three point sources
		image = Image2D::CreateZeroImagePtr(width, height);
		AddModelData(image, 3);
		} break;
		case 22: { // Model of five point sources
		image = Image2D::CreateZeroImagePtr(width, height);
		AddModelData(image, 5);
		} break;
		case 23:
			image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
			AddBroadbandToTestSet(image, rfi, 0.5, 0.1, true);
		break;
		case 24:
			image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
			AddBroadbandToTestSet(image, rfi, 0.5, 10.0, true);
		break;
		case 25:
			image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
			AddBroadbandToTestSet(image, rfi, 0.5, 1.0, true);
		break;
		case 26: { // Several Gaussian broadband lines
			image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
			AddBroadbandToTestSet(image, rfi, 1.0, 1.0, false, GaussianShape);
		} break;
		case 27: { // Several Sinusoidal broadband lines
			image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
			AddBroadbandToTestSet(image, rfi, 1.0, 1.0, false, SinusoidalShape);
		} break;
		case 28: { // Several slewed Gaussian broadband lines
			image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
			AddSlewedBroadbandToTestSet(image, rfi, 1.0);
		} break;
		case 29: { // Several bursty broadband lines
			image = Image2DPtr(CreateNoise(width, height, gaussianNoise));
			AddBurstBroadbandToTestSet(image, rfi);
		} break;
		case 30: { // noise + RFI ^-2 distribution
			image = sampleRFIDistribution(width, height, 1.0);
			rfi->SetAll<true>();
		} break;
		case 31: { // noise + RFI ^-2 distribution
			image = sampleRFIDistribution(width, height, 0.1);
			rfi->SetAll<true>();
		} break;
		case 32: { // noise + RFI ^-2 distribution
			image = sampleRFIDistribution(width, height, 0.01);
			rfi->SetAll<true>();
		} break;
	}
	return image;
}

void MitigationTester::AddBroadbandToTestSet(Image2DPtr image, Mask2DPtr rfi, long double length, double strength, bool align, enum BroadbandShape shape)
{
	size_t frequencyCount = image->Height();
	unsigned step = image->Width()/11;
	if(align)
	{
		// see vertevd.h why this is:
		unsigned n = (unsigned) floor(0.5 + sqrt(0.25 + 2.0 * frequencyCount));
		unsigned affectedAntennas = (unsigned) n*(double)length;
		unsigned index = 0;
		AOLogger::Debug << affectedAntennas << " of " << n << " antennas effected." << '\n';
		AOLogger::Debug << "Affected  baselines: ";
		for(unsigned y=0;y<n;++y)
		{
			for(unsigned x=y+1;x<n;++x)
			{
				double a1, a2;
				if(x<affectedAntennas) a1=1.0; else a1=0.0;
				if(y<affectedAntennas) a2=1.0; else a2=0.0;
				
				if(y<affectedAntennas || x<affectedAntennas)
				{
					AOLogger::Debug << x << " x " << y << ", ";
					AddRfiPos(image, rfi, 3.0*strength*a1*a2, step*1, 3, index);
					AddRfiPos(image, rfi, 2.5*strength*a1*a2, step*2, 3, index);
					AddRfiPos(image, rfi, 2.0*strength*a1*a2, step*3, 3, index);
					AddRfiPos(image, rfi, 1.8*strength*a1*a2, step*4, 3, index);
					AddRfiPos(image, rfi, 1.6*strength*a1*a2, step*5, 3, index);

					AddRfiPos(image, rfi, 3.0*strength*a1*a2, step*6, 1, index);
					AddRfiPos(image, rfi, 2.5*strength*a1*a2, step*7, 1, index);
					AddRfiPos(image, rfi, 2.0*strength*a1*a2, step*8, 1, index);
					AddRfiPos(image, rfi, 1.8*strength*a1*a2, step*9, 1, index);
					AddRfiPos(image, rfi, 1.6*strength*a1*a2, step*10, 1, index);
				}
				++index;
			}
		}
		AOLogger::Debug << ".\n";
	} else {
		unsigned fStart = (unsigned) ((0.5 - length/2.0) * frequencyCount);
		unsigned fEnd = (unsigned) ((0.5 + length/2.0) * frequencyCount);
		AddBroadbandLinePos(image, rfi, 3.0*strength, step*1, 3, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 2.5*strength, step*2, 3, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 2.0*strength, step*3, 3, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 1.8*strength, step*4, 3, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 1.6*strength, step*5, 3, fStart, fEnd, shape);

		AddBroadbandLinePos(image, rfi, 3.0*strength, step*6, 1, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 2.5*strength, step*7, 1, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 2.0*strength, step*8, 1, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 1.8*strength, step*9, 1, fStart, fEnd, shape);
		AddBroadbandLinePos(image, rfi, 1.6*strength, step*10, 1, fStart, fEnd, shape);
	}
}

void MitigationTester::AddSlewedBroadbandToTestSet(Image2DPtr image, Mask2DPtr rfi, long double length, double strength, double slewrate, enum BroadbandShape shape)
{
	size_t frequencyCount = image->Height();
	unsigned step = image->Width()/11;
	unsigned fStart = (unsigned) ((0.5 - length/2.0) * frequencyCount);
	unsigned fEnd = (unsigned) ((0.5 + length/2.0) * frequencyCount);
	AddSlewedBroadbandLinePos(image, rfi, 3.0*strength, slewrate, step*1, 3, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 2.5*strength, slewrate, step*2, 3, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 2.0*strength, slewrate, step*3, 3, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 1.8*strength, slewrate, step*4, 3, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 1.6*strength, slewrate, step*5, 3, fStart, fEnd, shape);

	AddSlewedBroadbandLinePos(image, rfi, 3.0*strength, slewrate, step*6, 1, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 2.5*strength, slewrate, step*7, 1, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 2.0*strength, slewrate, step*8, 1, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 1.8*strength, slewrate, step*9, 1, fStart, fEnd, shape);
	AddSlewedBroadbandLinePos(image, rfi, 1.6*strength, slewrate, step*10, 1, fStart, fEnd, shape);
}

void MitigationTester::AddVarBroadbandToTestSet(Image2DPtr image, Mask2DPtr rfi)
{
	// The "randomness" should be reproducable randomness, so calling
	// the random number generator to generate the numbers is not a good
	// idea.
	unsigned step = image->Width()/11;
	AddBroadbandLine(image, rfi, 3.0, step*1, 3, 0.937071,0.0185952);
	AddBroadbandLine(image, rfi, 2.5, step*2, 3, 0.638442,0.327689);
	AddBroadbandLine(image, rfi, 2.0, step*3, 3, 0.859308,0.0211675);
	AddBroadbandLine(image, rfi, 1.8, step*4, 3, 0.418327,0.324842);
	AddBroadbandLine(image, rfi, 1.6, step*5, 3, 0.842374,0.105613);

	AddBroadbandLine(image, rfi, 3.0, step*6, 1, 0.704607,0.163653);
	AddBroadbandLine(image, rfi, 2.5, step*7, 1, 0.777955,0.0925143);
	AddBroadbandLine(image, rfi, 2.0, step*8, 1, 0.288418,0.222322);
	AddBroadbandLine(image, rfi, 1.8, step*9, 1, 0.892462,0.0381083);
	AddBroadbandLine(image, rfi, 1.6, step*10, 1, 0.444377,0.240526);
}

void MitigationTester::AddModelData(Image2DPtr , unsigned /*sources*/)
{
	//TODO
	/*
	class Model model;
	if(sources>=5) {
		model.AddSource(0.1,0.1,0.5);
		model.AddSource(0.1,0.0,0.35);
		model.AddSource(.101,0.001,0.45);
		model.AddSource(1.0,0.0,1.0);
		model.AddSource(4.0,3.0,0.9);
	} else {
		if(sources>=1)
			model.AddSource(0.1,0.1,0.7);
		if(sources>=2)
			model.AddSource(0.1,0.0,0.5);
		if(sources>=3)
			model.AddSource(1.0,0.0,1.0);
	}
	//WSRTObservatorium wsrt(0,1);
	//std::pair<TimeFrequencyData,TimeFrequencyMetaDataCPtr> data =
	//	model.SimulateObservation(wsrt, 0.05, 0.05, 130.0e+6, 0, 1);
	//image->SetValues(data.first.GetRealPart());
	*/
}

void MitigationTester::SubtractBackground(Image2DPtr image)
{
	Mask2DPtr zero = Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
	LocalFitMethod fittedImage;
	fittedImage.SetToWeightedAverage(20, 40, 7.5, 15.0);
	TimeFrequencyData data(TimeFrequencyData::AmplitudePart, SinglePolarisation, image);
	data.SetGlobalMask(zero);
	fittedImage.Initialize(data);
	for(unsigned i=0;i<fittedImage.TaskCount();++i)
		fittedImage.PerformFit(i);
	Image2D *diff = Image2D::CreateFromDiff(*image, *fittedImage.Background().GetSingleImage());
	image->SetValues(*diff);
	delete diff;
	for(unsigned y=0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width();++x) {
			image->AddValue(x, y, 1.0); 
		}
	}
}

Image2DPtr MitigationTester::sampleRFIDistribution(unsigned width, unsigned height, double ig_over_rsq)
{
	Image2DPtr image = Image2D::CreateUnsetImagePtr(width, height);
	const double sigma = 1.0;

	for(size_t f=0; f<height;++f) {
		for(size_t t=0; t<width;++t) {
			image->SetValue(t, f, Rand(Gaussian)*sigma + ig_over_rsq / RNG::Uniform());
		}
	}
	return image;
}
