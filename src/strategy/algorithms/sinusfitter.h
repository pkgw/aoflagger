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
#ifndef SINUSFITTER_H
#define SINUSFITTER_H

#include <cstring>
#include <cmath>

#include "../../msio/types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class SinusFitter {
	public:
		SinusFitter();
		~SinusFitter();
		void FindPhaseAndAmplitude(num_t &phase, num_t &amplitude, const num_t *dataX, const num_t *dataT, const size_t dataSize, const num_t frequency) const throw();
		void FindPhaseAndAmplitudeComplex(num_t &phase, num_t &amplitude, const num_t *dataR, const num_t *dataI, const num_t *dataT, const size_t dataSize, const num_t frequency) const throw();


		num_t FindMean(const num_t phase, const num_t amplitude, const num_t *dataX, const num_t *dataT, const size_t dataSize, const num_t frequency);

		static num_t Value(const num_t phase, const num_t amplitude, const num_t t, const num_t frequency, num_t mean)
		{
			return cosn(phase + t * frequency) * amplitude + mean;
		}

		/*template<typename T> static T Phase(T real, T imaginary)
		{
			if(real==0.0L)
			{
				if(imaginary==0.0L)
					return 0.0L;
				else if(imaginary > 0.0L)
					return M_PIn*0.5;
				else
					return -M_PIn*0.5;
			}
			else if(real>0.0L)
			{
				if(imaginary>=0.0L) // first 
					return atannl(imaginary/real);
				else // fourth
					return atannl(imaginary/real)+2.0*M_PIn;
			}
			else
			{
				if(imaginary>=0.0L) // second
					return atannl(imaginary/real) + 1.0*M_PIn;
				else // third
					return atannl(imaginary/real) + 1.0*M_PIn;
			}
		}*/
		static num_t Phase(num_t real, num_t imaginary)
		{
			return atan2n(imaginary, real);
		}
		static numl_t Phase(numl_t real, numl_t imaginary)
		{
			return atan2nl(imaginary, real);
		}
	private:
		
};

#endif
