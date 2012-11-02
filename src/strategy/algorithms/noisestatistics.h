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
#ifndef NOISESTATISTICS_H
#define NOISESTATISTICS_H

#include <string>
#include <iostream>
#include <vector>
#include <cmath>

#include "../../msio/types.h"

class NoiseStatistics {
	public:

		typedef numl_t stat_t;
		typedef std::vector<stat_t> Array;
	
		NoiseStatistics()
		: _sum(0.0), _sum2(0.0), _sum3(0.0), _sum4(0.0), _count(0)
		{
		}
		
		NoiseStatistics(const Array &samples)
		: _sum(0.0), _sum2(0.0), _sum3(0.0), _sum4(0.0), _count(0)
		{
			Add(samples);
		}
		
		void Set(const Array &samples)
		{
			_sum = 0.0;
			_sum2 = 0.0;
			_sum3 = 0.0;
			_sum4 = 0.0;
			_count = 0;
			Add(samples);
		}
		
		void Add(const Array &samples)
		{
			// Calculate sum & mean
			for(Array::const_iterator i = samples.begin(); i != samples.end(); ++i)
			{
				const stat_t v = *i;
				_sum += v;
				_sum2 += (v * v);
				_sum3 += (v * v * v);
				_sum4 += (v * v * v * v);
			}
			_count += samples.size();
		}
		
		void Add(const NoiseStatistics &statistics)
		{
			_count += statistics._count;
			_sum += statistics._sum;
			_sum2 += statistics._sum2;
			_sum3 += statistics._sum3;
			_sum4 += statistics._sum4;
		}
		
		unsigned long Count() const
		{
			return _count;
		}
		
		void SetCount(unsigned long count)
		{
			_count = count;
		}
		
		stat_t Sum() const
		{
			return _sum;
		}
		
		void SetSum(stat_t sum)
		{
			_sum = sum;
		}
		
		stat_t Sum2() const
		{
			return _sum2;
		}
		
		void SetSum2(stat_t sum2)
		{
			_sum2 = sum2;
		}
		
		stat_t Sum3() const
		{
			return _sum3;
		}
		
		stat_t Sum4() const
		{
			return _sum4;
		}
		
		stat_t Mean() const
		{
			if(_count == 0)
				return 0.0;
			else
				return _sum / (numl_t) _count;
		}
		
		stat_t StdDevEstimator() const
		{
			return std::sqrt(VarianceEstimator());
		}
		
		stat_t VarianceEstimator() const
		{
			if(_count <= 1)
				return 0.0;
			else
			{
				const stat_t n = _count;
				const stat_t sumMeanSquared = (_sum * _sum) / n;
				return (_sum2 + sumMeanSquared - (_sum * 2.0 * _sum / n)) / (n-1.0);
			}
		}
		
		stat_t SecondMoment() const
		{
			if(_count == 0)
				return 0.0;
			else
			{
				const stat_t n = _count;
				const stat_t sumMeanSquared = (_sum * _sum) / n;
				return (_sum2 + sumMeanSquared - (_sum * 2.0 * _sum / n)) / n;
			}
		}
		
		stat_t FourthMoment() const
		{
			if(_count == 0)
				return 0.0;
			else
			{
				const stat_t
					n = _count,
					mean = _sum / n,
					mean2 = mean * mean;
				return (_sum4
					- 4.0 * (_sum3 * mean + _sum * mean2 * mean)
					+ 6.0 * _sum2 * mean2) / n
					+ mean2 * mean2;
			}
		}
		
		stat_t VarianceOfVarianceEstimator() const
		{
			const long double n = _count;
			if(n <= 1)
				return 0.0;
			else
			{
				const long double moment2 = SecondMoment();
				return ( FourthMoment() - moment2 * moment2 * (n-3.0)/(n-1.0) ) / n;
			}
		}
		
		static unsigned WriteColumnCount() { return 8; }
		static unsigned VarianceColumn() { return 6; }
		
		static void WriteHeaders(const std::string &headerPrefix, std::ostream &stream)
		{
			stream <<
				headerPrefix << "Count\t" << headerPrefix << "Sum\t" << headerPrefix << "Sum2\t" <<
				headerPrefix << "Sum3\t" << headerPrefix << "Sum4\t" <<
				headerPrefix << "Mean\t" << headerPrefix << "Variance\t" <<
				headerPrefix << "VarianceOfVariance";
		}
		
		void WriteValues(std::ostream &stream) const
		{
			stream
				<< _count << '\t'
				<< _sum << '\t'
				<< _sum2 << '\t'
				<< _sum3 << '\t'
				<< _sum4 << '\t'
				<< Mean() << '\t'
				<< VarianceEstimator() << '\t'
				<< VarianceOfVarianceEstimator();
		}
		
		void ReadValues(std::istream &stream)
		{
			stat_t tmp;
			stream >> _count >> _sum >> _sum2 >> _sum3 >> _sum4 >> tmp >> tmp >> tmp;
		}
	private:
		stat_t _sum;
		stat_t _sum2;
		stat_t _sum3;
		stat_t _sum4;
		unsigned long _count;
};

#endif
