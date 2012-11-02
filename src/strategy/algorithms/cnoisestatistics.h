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
#ifndef CNOISESTATISTICS_H
#define CNOISESTATISTICS_H

#include "noisestatistics.h"

#include <complex>

class CNoiseStatistics
{
	public:
		CNoiseStatistics() : real(), imaginary()
		{
		}
		
		CNoiseStatistics(const NoiseStatistics::Array &realValues, const NoiseStatistics::Array &imaginaryValues)
		: real(realValues), imaginary(imaginaryValues)
		{
		}
		
		CNoiseStatistics(const CNoiseStatistics &source) : real(source.real), imaginary(source.imaginary)
		{
		}
		
		void operator=(const CNoiseStatistics &source)
		{
			real = source.real;
			imaginary = source.imaginary;
		}
		
		void operator+=(const CNoiseStatistics &rhs)
		{
			real.Add(rhs.real);
			imaginary.Add(rhs.imaginary);
		}
		
		long unsigned int Count() const
		{
			return real.Count();
		}
		
		void SetCount(long unsigned int count)
		{
			real.SetCount(count);
			imaginary.SetCount(count);
		}
		
		void AddCount(long unsigned int count)
		{
			real.SetCount(real.Count() + count);
			imaginary.SetCount(imaginary.Count() + count);
		}
		
		std::complex<float> Sum() const
		{
			return std::complex<float>(real.Sum(), imaginary.Sum());
		}
		
		void SetSum(std::complex<float> sum)
		{
			real.SetSum(sum.real());
			imaginary.SetSum(sum.imag());
		}
		
		void AddSum(std::complex<float> sum)
		{
			real.SetSum(real.Sum() + sum.real());
			imaginary.SetSum(imaginary.Sum() + sum.imag());
		}
		
		std::complex<float> Sum2() const
		{
			return std::complex<float>(real.Sum2(), imaginary.Sum2());
		}
		
		void SetSum2(std::complex<float> sum2)
		{
			real.SetSum2(sum2.real());
			imaginary.SetSum2(sum2.imag());
		}
		
		void AddSum2(std::complex<float> sum2)
		{
			real.SetSum2(real.Sum2() + sum2.real());
			imaginary.SetSum2(imaginary.Sum2() + sum2.imag());
		}
		
		std::complex<float> Sum3() const
		{
			return std::complex<float>(real.Sum3(), imaginary.Sum3());
		}
		
		std::complex<float> Sum4() const
		{
			return std::complex<float>(real.Sum4(), imaginary.Sum4());
		}
		
		std::complex<float> Mean() const
		{
			return std::complex<float>(real.Mean(), imaginary.Mean());
		}
		
		std::complex<float> StdDevEstimator() const
		{
			return std::complex<float>(real.StdDevEstimator(), imaginary.StdDevEstimator());
		}
		
		std::complex<float> VarianceEstimator() const
		{
			return std::complex<float>(real.VarianceEstimator(), imaginary.VarianceEstimator());
		}
		
		std::complex<float> SecondMoment() const
		{
			return std::complex<float>(real.SecondMoment(), imaginary.SecondMoment());
		}
		
		std::complex<float> FourthMoment() const
		{
			return std::complex<float>(real.FourthMoment(), imaginary.FourthMoment());
		}
		
		std::complex<float> VarianceOfVarianceEstimator() const
		{
			return std::complex<float>(real.VarianceOfVarianceEstimator(), imaginary.VarianceOfVarianceEstimator());
		}
		
		NoiseStatistics real;
		NoiseStatistics imaginary;
};

#endif
