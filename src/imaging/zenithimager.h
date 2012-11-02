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

#ifndef ZENITH_IMAGER_H
#define ZENITH_IMAGER_H

#include <complex>

#include "../msio/image2d.h"

class ZenithImager
{
	public:
		ZenithImager();
		
		void Initialize(size_t resolution);
		
		void Clear();
		
		void Add(const class BandInfo &band, const std::complex<float> *samples, const bool *isRFI, double u, double v, double w, double phaseRotation);
		
		void FourierTransform(Image2DPtr &real, Image2DPtr &imaginary);
		
		Image2DCPtr UVReal() const { return _real; }
		Image2DCPtr UVImaginary() const { return _imaginary; }
	private:
		void add(const class BandInfo &band, double r, double i, double u, double v, double w, double phaseRotation, double wavelength);
		
		static long double Wavelength(long double frequency)
		{
			return 299792458.0L / frequency;
		}
		
		size_t _resolution, _totalCount, _outsideCount;
		
		Image2DPtr _real, _imaginary, _weights;
};

#endif
