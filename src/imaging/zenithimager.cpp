/***************************************************************************
 *   Copyright (C) 2012 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include "zenithimager.h"

#include "../msio/antennainfo.h"

#include "../util/ffttools.h"

ZenithImager::ZenithImager() : _resolution(0), _totalCount(0), _outsideCount(0)
{
}

void ZenithImager::Initialize(size_t resolution)
{
	_resolution = resolution;
	_real = Image2D::CreateZeroImagePtr(resolution, resolution);
	_imaginary = Image2D::CreateZeroImagePtr(resolution, resolution);
	_weights = Image2D::CreateZeroImagePtr(resolution, resolution);
}

void ZenithImager::Clear()
{
	_real->SetAll(0.0);
	_imaginary->SetAll(0.0);
	_weights->SetAll(0.0);
	_totalCount = 0;
	_outsideCount = 0;
}

void ZenithImager::Add(const BandInfo &band, const std::complex<float> *samples, const bool *isRFI, double u, double v, double w, double phaseRotation)
{
	size_t n = band.channels.size();
	for(size_t f=0;f<n;++f)
	{
		if(!isRFI[f])
		{
			const double wavelength = Wavelength(band.channels[f].frequencyHz);
			const double r = samples[f].real(), i = samples[f].imag();
			add(band, r, i, u, v, w, phaseRotation, wavelength);
		}
	}
}

void ZenithImager::add(const BandInfo &band, double r, double i, double u, double v, double w, double phaseRotation, double wavelength)
{
	const double norm = 1.1; //2.0*M_PI; //sqrt(u*u + v*v + w*w) / sqrt(u*u + v*v);
	const double factor = 1.0 / wavelength;
	u *= factor * norm;
	v *= factor * norm;
	//w *= factor;
	
	const double dcpixel = (double) _resolution * 0.5;
	
	// Calculate the pixel indices. Because we want the image domain to
	// have a range of -1 to 1 (which is the horizon), the uv-domain should
	// go from 1/-1 to 1/1. Hence, the u&v need to be multiplied by two.
	int uPos = (int) round(u*2.0 + dcpixel);
	int vPos = (int) round(v*2.0 + dcpixel);
	
	if(uPos >= 0 && vPos >= 0 && uPos < (int) _resolution && vPos < (int) _resolution)
	{
		phaseRotation *= factor * 2.0 * M_PI;
		const double sinR = sin(phaseRotation), cosR = cos(phaseRotation);
		const double rotatedR = r * cosR - i * sinR;
		const double rotatedI = r * sinR + i * cosR;
		
		_real->AddValue(uPos, vPos, rotatedR);
		_imaginary->AddValue(uPos, vPos, rotatedI);
		_weights->AddValue(uPos, vPos, 1.0);
		
		int uPos2 = (int) round(dcpixel - u*2.0);
		int vPos2 = (int) round(dcpixel - v*2.0);
		if(uPos2 >= 0 && vPos2 >= 0 && uPos2 < (int) _resolution && vPos2 < (int) _resolution)
		{
			_real->AddValue(uPos2, vPos2, rotatedR);
			_imaginary->AddValue(uPos2, vPos2, -rotatedI);
			_weights->AddValue(uPos2, vPos2, 1.0);
		}
	} else {
		_outsideCount++;
	}
	
	++_totalCount;
}

void ZenithImager::FourierTransform(Image2DPtr &real, Image2DPtr &imaginary)
{
	std::cout << "Performing FT of " << _totalCount << " samples, " << _outsideCount << " (" << round(100.0*(double) _outsideCount/(double) _totalCount) << "%) fell outside uv area... " << std::flush;
	
	double normFactor = _weights->Sum() / ((num_t) _real->Height() * _real->Width());
	for(size_t y=0;y<_real->Height();++y) {
		for(size_t x=0;x<_real->Width();++x) {
			num_t weight = _weights->Value(x, y);
			if(weight != 0.0)
			{
				_real->SetValue(x, y, _real->Value(x, y) * normFactor / weight);
				_imaginary->SetValue(x, y, _imaginary->Value(x, y) * normFactor / weight);
				_weights->SetValue(x, y, 1.0);
			} 
		}
	}
	
	real = Image2D::CreateUnsetImagePtr(_resolution, _resolution);
	imaginary = Image2D::CreateUnsetImagePtr(_resolution, _resolution);
	FFTTools::CreateFFTImage(*_real, *_imaginary, *real, *imaginary);
}
