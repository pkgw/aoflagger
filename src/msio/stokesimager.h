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
#ifndef STOKESIMAGER_H
#define STOKESIMAGER_H

#include "timefrequencyimager.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class StokesImager{
	public:
		StokesImager();
		~StokesImager();

		const class Image2D &StokesI() const throw() { return *_stokesI; }
		const class Image2D &StokesQ() const throw() { return *_stokesQ; }
		const class Image2D &StokesU() const throw() { return *_stokesU; }
		const class Image2D &StokesV() const throw() { return *_stokesV; }

		void Image(const TimeFrequencyImager &timeFreq) {
			Image(*timeFreq.RealXX(), *timeFreq.ImaginaryXX(), *timeFreq.RealXY(), *timeFreq.ImaginaryXY(), *timeFreq.RealYY(), *timeFreq.ImaginaryYY());
		}

		void Image(const Image2D &realXX, const Image2D &imaginaryXX, const Image2D &realXY, const Image2D &imaginaryXY, const Image2D &realYY, const Image2D &imaginaryYY); 
		
		static Image2DPtr CreateSum(Image2DCPtr left, Image2DCPtr right);
		static Image2DPtr CreateDifference(Image2DCPtr left, Image2DCPtr right);
		static Image2DPtr CreateNegatedSum(Image2DCPtr left, Image2DCPtr right);

		static Image2DPtr CreateStokesIAmplitude(Image2DCPtr realXX, Image2DCPtr imaginaryXX, Image2DCPtr realYY, Image2DCPtr imaginaryYY);
		static Image2DPtr CreateStokesI(Image2DCPtr xx, Image2DCPtr yy) { return CreateSum(xx, yy); }
		static Image2DPtr CreateStokesQ(Image2DCPtr xx, Image2DCPtr yy) { return CreateDifference(xx, yy); }

		static Image2DPtr CreateAvgPhase(Image2DCPtr xx, Image2DCPtr yy);
	private:

		Image2D *_stokesI, *_stokesQ, *_stokesU, *_stokesV;
};

#endif
