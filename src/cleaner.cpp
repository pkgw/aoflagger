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
#include "cleaner.h"

#include "msio/image2d.h"

#include "util/ffttools.h"

#include <iostream>

Cleaner::Cleaner() : _residualReal(0), _residualImaginary(0)
{
}

Cleaner::~Cleaner()
{
	if(_residualReal != 0) {
		delete _residualReal;
		delete _residualImaginary;
		delete _weightsImaginary;
		delete _psfReal;
		delete _psfImaginary;
		delete _deconvolvedReal;
		delete _deconvolvedImaginary;
		delete _convolvedReal;
		delete _convolvedImaginary;
	}
}

void Cleaner::Init(const Image2D &real, const Image2D &imaginary, const Image2D &weights, long double gain)
{
	_real = &real;
	_imaginary = &imaginary;
	_weightsReal = &weights;
	_gain = gain;

	// Imaginary values are 0, as we only use real factors for the weights
	_weightsImaginary = Image2D::CreateZeroImage(weights.Width(), weights.Height());

	_psfReal = Image2D::CreateEmptyImage(weights.Width(), weights.Height());
	_psfImaginary = Image2D::CreateZeroImage(weights.Width(), weights.Height());
	FFTTools::CreateFFTImage(*_weightsReal, *_weightsImaginary, *_psfReal, *_psfImaginary);

	_residualReal = Image2D::CreateCopy(*_real);
	_residualImaginary = Image2D::CreateCopy(*_imaginary);
	_deconvolvedReal = Image2D::CreateZeroImage(real.Width(), real.Height());
	_deconvolvedImaginary = Image2D::CreateZeroImage(imaginary.Width(), imaginary.Height());
	unsigned x, y;
	FindPeak(*_psfReal, *_psfImaginary, x, y);
	_psfPeak = _psfReal->Value(x, y);
	_convolvedReal = Image2D::CreateEmptyImage(_residualReal->Width(), _residualReal->Height()),
	_convolvedImaginary = Image2D::CreateEmptyImage(_residualImaginary->Width(), _residualImaginary->Height());
	_psfSum = 0.0;
	for(unsigned j=0;j<_psfReal->Height();++j) {
		for(unsigned i=0;i<_psfReal->Width();++i) {
			_psfSum += fabsf(_psfReal->Value(i, j));
		}
	}
	std::cout << "psf sum: " << _psfSum << std::endl;
}

void Cleaner::Iteration()
{
	unsigned x, y;
	
	std::cout << "Finding peak in convolved: ";
	FFTTools::FFTConvolve(*_residualReal, *_residualImaginary, *_psfReal, *_psfImaginary, *_convolvedReal, *_convolvedImaginary);

	FindPeak(*_convolvedReal, *_convolvedImaginary, x, y);

	{ //testing
		std::cout << "Finding peak in original: ";
		unsigned x2, y2;
		FindPeak(*_residualReal, *_residualImaginary, x2, y2);
	}

	long double currentGain = _gain * MaxAllowedGain(*_residualReal, *_psfReal, x, y);
	unsigned area = _residualReal->Width()*_residualReal->Height();
	std::cout << "Gain: " << currentGain << " conv gain: " << (_gain*_convolvedReal->Value(x,y)/(_psfSum*area*area)) << std::endl;
	_deconvolvedReal->AddValue(x, y, currentGain);
	Subtract(*_residualReal, *_psfReal, x, y, currentGain);
	Subtract(*_residualImaginary, *_psfImaginary, x, y, currentGain);
}

void Cleaner::FindPeak(const Image2D &real, const Image2D &imaginary, unsigned &x, unsigned &y)
{
	long double highest = real.Value(0, 0); //*real.Value(0, 0) + imaginary.Value(0, 0)*imaginary.Value(0, 0);
	x = 0;
	y = 0;
	for(unsigned j=0;j<real.Height();++j) {
		for(unsigned i=0;i<real.Width();++i) {
			long double vr = real.Value(i, j);
			//long double vi = imaginary.Value(i, j);
			//long double a = vr*vr+vi*vi;
			if(vr > highest) {
				x = i;
				y = j;
				highest = vr;
			}
		}
	}
	std::cout << "Peak: " << x << "," << y << " (" << real.Value(x,y) << ")" << std::endl;
}

long double Cleaner::MaxAllowedGain(const Image2D &image, const Image2D &subtract, unsigned x, unsigned y)
{
	/*unsigned halfwidth = image.Width()/2, halfheight = image.Height()/2, count = 0;
	long double sum = 0.0, norm = 1.0L / image.Value(x,y);
	for(unsigned j=0;j<image.Height();++j) {
		for(unsigned i=0;i<image.Width();++i) {
			long double s = subtract.Value((i - x + halfwidth)%image.Width(), (j - y + halfheight)%image.Height());
			if( ( s > 0) == (image.Value(i, j)>0) ) {
				sum += fabsf(image.Value(i, j) - s);
				++count;
			}
		}
	}
	return sum * norm * 100.0 / (image.Width() * image.Height()); */
	
	unsigned halfwidth = image.Width()/2, halfheight = image.Height()/2;
	long double ratioA = image.Value(x, y) / subtract.Value(halfwidth, halfheight);
	long double ratioB = image.Value((x+1)%image.Width(), y) / subtract.Value(halfwidth+1, halfheight);
	long double ratioC = image.Value((x+image.Width()-1)%image.Width(), y) / subtract.Value(halfwidth-1, halfheight);
	long double ratioD = image.Value(x, (y+1)%image.Height()) / subtract.Value(halfwidth, halfheight+1);
	long double ratioE = image.Value(x, (y+image.Height()-1)%image.Height()) / subtract.Value(halfwidth, halfheight-1);
	std::cout << "A=" << ratioA << ",B=" << ratioB << ",C=" << ratioC << ",D=" << ratioD << ",E=" << ratioE << std::endl;
	return (ratioA*2.0 + ratioB + ratioC + ratioD + ratioE) / 6.0;
}

void Cleaner::Subtract(Image2D &original, const Image2D &rightHand, unsigned x, unsigned y, long double gain)
{
	unsigned cx = rightHand.Width()/2, cy = rightHand.Height()/2;
	for(unsigned j=0;j<rightHand.Height();++j) {
		for(unsigned i=0;i<rightHand.Width();++i) {
			original.AddValue(i, j, -rightHand.Value((i+cx-x)%rightHand.Width(), (j+cy-y)%rightHand.Height()) * gain);
		}
	}
}
