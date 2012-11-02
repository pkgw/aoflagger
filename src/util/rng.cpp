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
#include "rng.h"

#include <stdlib.h>
#include <cmath>

#ifndef M_PIl
#define M_PIl M_PI
#endif

RNG::RNG()
{
}

double RNG::Uniform()
{
	return (long double) rand() / (long double) RAND_MAX;
}

double RNG::Gaussian()
{
	long double a, b;
	DoubleGaussian(a, b);
	return a;
}

double RNG::GaussianProduct()
{
	long double a, b;
	DoubleGaussian(a, b);
	return a * b;
}

double RNG::GaussianPartialProduct()
{
	long double a, b;
	DoubleGaussian(a, b);
	if(a >= 0.0)
		a = pow(a, M_SQRT2/2.0);
	else
		a = -pow(-a, M_SQRT2/2.0);
	if(b >= 0.0)
		b = pow(b, M_SQRT2/2.0);
	else
		b = -pow(-b, M_SQRT2/2.0);
	return a*b;
}

void RNG::DoubleGaussian(long double &a, long double &b)
{
	long double x1, x2, w;
	
	do {
		long double r1 = (long double) rand() / (long double) RAND_MAX; 
		long double r2 = (long double) rand() / (long double) RAND_MAX; 
		x1 = 2.0 * r1 - 1.0;
		x2 = 2.0 * r2 - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = std::sqrt( (-2.0 * std::log( w ) ) / w );
	a = x1 * w;
	b = x2 * w;
}

double RNG::Rayleigh()
{
	double x = Gaussian(), y = Gaussian();
	return sqrt(x*x + y*y);
}

double RNG::EvaluateRayleigh(double x, double sigma)
{
	return x * exp(-x*x/(2.0*sigma*sigma)) / (sigma * sigma);
}

double RNG::IntegrateGaussian(long double upperLimit)
{
	long double integral = 0.0L, term = 0.0L, stepSize = 1e-4L;
	upperLimit -= stepSize/2.0L;
	do {
		term = std::exp(-upperLimit * upperLimit / 2.0L);
		upperLimit -= stepSize;
		integral += term * stepSize;
	} while(term >= 1e-6 || upperLimit >= 0);
	return integral / std::sqrt(2.0L * M_PIl);
}

double RNG::EvaluateGaussian(double x, double sigmaSquared)
{
	return 1.0 / (sigmaSquared * std::sqrt(2.0L*M_PI)) * std::exp(-0.5*x*x/sigmaSquared);
}

long double RNG::EvaluateGaussian(long double x, long double sigmaSquared)
{
	return 1.0L / (sigmaSquared * std::sqrt(2.0L*M_PI)) * std::exp(-0.5L*x*x/sigmaSquared);
}

double RNG::EvaluateUnnormalizedGaussian(double x, double sigmaSquared)
{
	return std::exp(-0.5*x*x/sigmaSquared);
}

double RNG::EvaluateGaussian2D(long double x1, long double x2, long double sigmaX1, long double sigmaX2)
{
	return 1.0L / (2.0L*M_PI*sigmaX1*sigmaX2) * std::exp(-0.5L*(x1*(1.0L/sigmaX1)*x1 + x2*(1.0L/sigmaX2)*x2));
}

void RNG::ComplexGaussianAmplitude(num_t &r, num_t &i)
{
	num_t amplitude = Gaussian();
	num_t phase = Uniform() * 2.0 * M_PIn;
	r = amplitude * std::cos(phase);
	i = amplitude * std::sin(phase);
}
