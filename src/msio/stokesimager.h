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
		static Image2DPtr CreateSum(Image2DCPtr left, Image2DCPtr right);
		static Image2DPtr CreateDifference(Image2DCPtr left, Image2DCPtr right);
		static Image2DPtr CreateNegatedSum(Image2DCPtr left, Image2DCPtr right);

		static Image2DPtr CreateStokesIAmplitude(Image2DCPtr realXX, Image2DCPtr imaginaryXX, Image2DCPtr realYY, Image2DCPtr imaginaryYY);
		static Image2DPtr CreateStokesI(Image2DCPtr xx, Image2DCPtr yy) { return CreateSum(xx, yy); }
		static Image2DPtr CreateStokesQ(Image2DCPtr xx, Image2DCPtr yy) { return CreateDifference(xx, yy); }

		static Image2DPtr CreateAvgPhase(Image2DCPtr xx, Image2DCPtr yy);
	private:
		StokesImager() { }
		~StokesImager() { }
};

#endif
