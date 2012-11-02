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
#ifndef QUALITY__DEFAULT_STATISTICS_H
#define QUALITY__DEFAULT_STATISTICS_H

#include <complex>
#include <stdint.h>

#include "../util/serializable.h"

class DefaultStatistics : public Serializable
{
	public:
		DefaultStatistics(unsigned polarizationCount) :
			_polarizationCount(polarizationCount)
		{
			initialize();
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] = 0;
				count[p] = 0;
				sum[p] = 0.0;
				sumP2[p] = 0.0;
				dCount[p] = 0;
				dSum[p] = 0.0;
				dSumP2[p] = 0.0;
			}
		}
		
		~DefaultStatistics()
		{
			destruct();
		}
		
		DefaultStatistics(const DefaultStatistics &other)
		: _polarizationCount(other._polarizationCount)
		{
			initialize();
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] = other.rfiCount[p];
				count[p] = other.count[p];
				sum[p] = other.sum[p];
				sumP2[p] = other.sumP2[p];
				dCount[p] = other.dCount[p];
				dSum[p] = other.dSum[p];
				dSumP2[p] = other.dSumP2[p];
			}
		}
		
		DefaultStatistics &operator=(const DefaultStatistics &other)
		{
			if(other._polarizationCount != _polarizationCount)
			{
				destruct();
				_polarizationCount = other._polarizationCount;
				initialize();
			}
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] = other.rfiCount[p];
				count[p] = other.count[p];
				sum[p] = other.sum[p];
				sumP2[p] = other.sumP2[p];
				dCount[p] = other.dCount[p];
				dSum[p] = other.dSum[p];
				dSumP2[p] = other.dSumP2[p];
			}
			return *this;
		}
		
		DefaultStatistics &operator+=(const DefaultStatistics &other)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] += other.rfiCount[p];
				count[p] += other.count[p];
				sum[p] += other.sum[p];
				sumP2[p] += other.sumP2[p];
				dCount[p] += other.dCount[p];
				dSum[p] += other.dSum[p];
				dSumP2[p] += other.dSumP2[p];
			}
			return *this;
		}
		
		DefaultStatistics ToSinglePolarization() const
		{
			if(_polarizationCount == 1)
				return *this;
			
			DefaultStatistics singlePol(1);
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				singlePol.rfiCount[0] += rfiCount[p];
				singlePol.count[0] += count[p];
				singlePol.sum[0] += sum[p];
				singlePol.sumP2[0] += sumP2[p];
				singlePol.dCount[0] += dCount[p];
				singlePol.dSum[0] += dSum[p];
				singlePol.dSumP2[0] += dSumP2[p];
			}
			return singlePol;
		}
		
		virtual void Serialize(std::ostream &stream) const
		{
			SerializeToUInt32(stream, _polarizationCount);
			
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				SerializeToUInt64(stream, rfiCount[p]);
				SerializeToUInt64(stream, count[p]);
				SerializeToLDoubleC(stream, sum[p]);
				SerializeToLDoubleC(stream, sumP2[p]);
				SerializeToUInt64(stream, dCount[p]);
				SerializeToLDoubleC(stream, dSum[p]);
				SerializeToLDoubleC(stream, dSumP2[p]);
			}
		}
		
		virtual void Unserialize(std::istream &stream)
		{
			uint32_t pCount = UnserializeUInt32(stream);
			if(pCount != _polarizationCount)
			{
				destruct();
				_polarizationCount = pCount;
				initialize();
			}
			
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] = UnserializeUInt64(stream);
				count[p] = UnserializeUInt64(stream);
				sum[p] = UnserializeLDoubleC(stream);
				sumP2[p] = UnserializeLDoubleC(stream);
				dCount[p] = UnserializeUInt64(stream);
				dSum[p] = UnserializeLDoubleC(stream);
				dSumP2[p] = UnserializeLDoubleC(stream);
			}
		}
		
		unsigned PolarizationCount() const
		{
			return _polarizationCount;
		}
		
		template<typename T>
		std::complex<T> Mean(unsigned polarization) const
		{
			return std::complex<T>(sum[polarization].real() / count[polarization], sum[polarization].imag() / count[polarization]);
		}
		
		template<typename T>
		std::complex<T> Sum(unsigned polarization) const
		{
			return std::complex<T>(sum[polarization].real(), sum[polarization].imag());
		}
		
		template<typename T>
		std::complex<T> SumP2(unsigned polarization) const
		{
			return std::complex<T>(sumP2[polarization].real(), sumP2[polarization].imag());
		}
		
		template<typename T>
		std::complex<T> DMean(unsigned polarization) const
		{
			return std::complex<T>(dSum[polarization].real() / dCount[polarization], dSum[polarization].imag() / dCount[polarization]);
		}
		
		template<typename T>
		std::complex<T> DSum(unsigned polarization) const
		{
			return std::complex<T>(dSum[polarization].real(), dSum[polarization].imag());
		}

		template<typename T>
		std::complex<T> DSumP2(unsigned polarization) const
		{
			return std::complex<T>(dSumP2[polarization].real(), dSumP2[polarization].imag());
		}
		
		unsigned long *rfiCount;
		unsigned long *count;
		std::complex<long double> *sum;
		std::complex<long double> *sumP2;
		unsigned long *dCount;
		std::complex<long double> *dSum;
		std::complex<long double> *dSumP2;
		
	private:
		void initialize()
		{
			rfiCount = new unsigned long[_polarizationCount];
			count = new unsigned long[_polarizationCount];
			sum = new std::complex<long double>[_polarizationCount];
			sumP2 = new std::complex<long double>[_polarizationCount];
			dCount = new unsigned long[_polarizationCount];
			dSum = new std::complex<long double>[_polarizationCount];
			dSumP2 = new std::complex<long double>[_polarizationCount];
		}
		
		void destruct()
		{
			delete[] rfiCount;
			delete[] count;
			delete[] sum;
			delete[] sumP2;
			delete[] dCount;
			delete[] dSum;
			delete[] dSumP2;
		}
		
		unsigned _polarizationCount;
};

#endif
