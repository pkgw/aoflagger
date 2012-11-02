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
#include "sinusfitter.h"

#include <cmath>

SinusFitter::SinusFitter()
{
}

SinusFitter::~SinusFitter()
{
}

void SinusFitter::FindPhaseAndAmplitude(num_t &phase, num_t &amplitude, const num_t *dataX, const num_t *dataT, const size_t dataSize, const num_t frequency) const throw()
{
	// calculate 1/N * \sum_t x(t) e^{-i * frequency * t}
	num_t sumR = 0.0L, sumI = 0.0L;
	for(unsigned i=0;i<dataSize;++i)
	{
		const num_t t = dataT[i];
		const num_t x = dataX[i];
		
		sumR += x * cosn(-t * frequency);
		sumI += x * sinn(-t * frequency);
	}

	sumR /= (num_t) dataSize;
	sumI /= (num_t) dataSize;

	phase = Phase(sumR, sumI);
	amplitude = 2.0L * sqrtn(sumR*sumR + sumI*sumI);
}

void SinusFitter::FindPhaseAndAmplitudeComplex(num_t &phase, num_t &amplitude, const num_t *dataR, const num_t *dataI, const num_t *dataT, const size_t dataSize, const num_t frequency) const throw()
{
	// calculate 1/N * \sum_t x(t) e^{-i * frequency * t}
	num_t sumR = 0.0L, sumI = 0.0L;
	for(unsigned i=0;i<dataSize;++i)
	{
		const num_t t = dataT[i];
		const num_t xR = dataR[i];
		const num_t xI = dataI[i];
		
		sumR += xR * cosn(-t * frequency);
		sumR += xI * sinn(-t * frequency);

		sumI += xR * sinn(-t * frequency);
		sumI -= xI * cosn(-t * frequency);
	}

	sumR /= (num_t) dataSize;
	sumI /= (num_t) dataSize;

	phase = Phase(sumR, sumI);
	amplitude = sqrtn(sumR*sumR + sumI*sumI);
}

num_t SinusFitter::FindMean(const num_t phase, const num_t amplitude, const num_t *dataX, const num_t *dataT, const size_t dataSize, const num_t frequency)
{
	num_t sum = 0.0L;
	for(unsigned i=0;i<dataSize;++i)
	{
		sum += dataX[i] - Value(phase, amplitude, dataT[i], frequency, 0.0L);
	}
	return sum / dataSize;
}
