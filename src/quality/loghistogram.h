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
#ifndef LOGHISTOGRAM_H
#define LOGHISTOGRAM_H

#include <map>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "histogramtablesformatter.h"

#include "../util/serializable.h"

//# pow10 seems to be undefined on OS-X
#ifdef __APPLE__
# define pow10(x) pow(10., (x))
#endif


class LogHistogram : public Serializable
{
	private:
		class AmplitudeBin : public Serializable
		{
		public:
			AmplitudeBin() :
				count(0)
			{
			}
			long unsigned count;
			
			long unsigned GetCount() const
			{
				return count;
			}
			
			AmplitudeBin &operator+=(const AmplitudeBin &other)
			{
				count += other.count;
				return *this;
			}
			AmplitudeBin &operator-=(const AmplitudeBin &other)
			{
				if(count >= other.count)
					count -= other.count;
				else
					count = 0;
				return *this;
			}
			
			virtual void Serialize(std::ostream &stream) const
			{
				SerializeToUInt64(stream, count);
			}
			virtual void Unserialize(std::istream &stream)
			{
				count = UnserializeUInt64(stream);
			}
		};
		
	public:
		LogHistogram()
		{
		}
		
		LogHistogram(const LogHistogram &source) : _amplitudes(source._amplitudes)
		{
		}
		
		void Add(const double amplitude)
		{
			if(std::isfinite(amplitude))
			{
				const double centralAmp = getCentralAmplitude(amplitude);
				AmplitudeBin &bin = getBin(centralAmp);
				++bin.count;
			}
		}
		
		void Add(const LogHistogram &histogram)
		{
			for(std::map<double, AmplitudeBin>::const_iterator i=histogram._amplitudes.begin(); i!=histogram._amplitudes.end();++i)
			{
				AmplitudeBin &bin = getBin(i->first);
				bin += i->second;
			}
		}
		
		void operator-=(const LogHistogram &histogram)
		{
			for(std::map<double, AmplitudeBin>::const_iterator i=histogram._amplitudes.begin(); i!=histogram._amplitudes.end();++i)
			{
				AmplitudeBin &bin = getBin(i->first);
				bin -= i->second;
			}
		}
		
		double MaxAmplitude() const
		{
			if(_amplitudes.empty())
				return 0.0;
			return _amplitudes.rbegin()->first;
		}
		
		double MinPositiveAmplitude() const
		{
			std::map<double, AmplitudeBin>::const_iterator i = _amplitudes.begin();
			if(i == _amplitudes.end())
				return 0.0;
			while(i->first <= 0.0)
			{
				++i;
				if(i == _amplitudes.end())
					return 0.0;
			}
			return i->first;
		}
		
		double NormalizedCount(double startAmplitude, double endAmplitude) const
		{
			unsigned long count = 0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=_amplitudes.begin();i!=_amplitudes.end();++i)
			{
				if(i->first >= startAmplitude && i->first < endAmplitude)
					count += i->second.GetCount();
			}
			return (double) count / (endAmplitude - startAmplitude);
		}
		
		double NormalizedCount(double centreAmplitude) const
		{
			const double key = getCentralAmplitude(centreAmplitude);
			std::map<double, AmplitudeBin>::const_iterator i = _amplitudes.find(key);
			if(i == _amplitudes.end()) return 0.0;
			return (double) i->second.GetCount() / (binEnd(centreAmplitude) - binStart(centreAmplitude));
		}
		
		double MinNormalizedCount() const
		{
			const_iterator i = begin();
			if(i == end())
				return 0.0;
			double minCount = i.normalizedCount();
			do
			{
				const double c = i.normalizedCount();
				if(c < minCount) minCount = c;
				++i;
			} while(i != end());
			return minCount;
		}
		
		double MaxNormalizedCount() const
		{
			double maxCount = 0.0;
			for (LogHistogram::const_iterator i=begin(); i!=end(); ++i)
			{
				if(i.normalizedCount() > maxCount && i.value() > 0 && std::isfinite(i.value()))
					maxCount = i.normalizedCount();
			}
			return maxCount;
		}
		
		double NormalizedTotalCount() const
		{
			unsigned long count = 0;
			for (LogHistogram::const_iterator i=begin(); i!=end(); ++i)
				count += i.unnormalizedCount();
			return count;
		}
		
		double NormalizedCountAbove(double lowerLimitingAmplitude) const
		{
			unsigned long count = 0;
			LogHistogram::const_iterator i=begin();
			while(i!=end() && i.value() <= lowerLimitingAmplitude)
			{
				++i;
			}
			while(i!=end())
			{
				count += i.unnormalizedCount();
				++i;
			}
			return count;
		}
		
		double AmplitudeWithMaxNormalizedCount() const
		{
			double maxCount = 0.0, maxPosition = 0.0;
			for (LogHistogram::const_iterator i=begin(); i!=end(); ++i)
			{
				if(i.normalizedCount() > maxCount && i.value() > 0 && std::isfinite(i.value()))
				{
					maxCount = i.normalizedCount();
					maxPosition = i.value();
				}
			}
			return maxPosition;
		}
		
		double MinPosNormalizedCount() const
		{
			const_iterator i = begin();
			if(i == end())
				return 0.0;
			double minCount = std::isfinite(i.normalizedCount()) ? i.normalizedCount() + 1.0 : 1.0;
			do
			{
				const double c = i.normalizedCount();
				if(c < minCount && c > 0.0 && std::isfinite(c)) minCount = c;
				++i;
			} while(i != end());
			return minCount;
		}
		
		double NormalizedSlope(double startAmplitude, double endAmplitude) const
		{
			unsigned long n = 0;
			long double sumX = 0.0, sumXY = 0.0, sumY = 0.0, sumXSquare = 0.0;
			for(const_iterator i=begin();i!=end();++i)
			{
				if(i.value() >= startAmplitude && i.value() < endAmplitude)
				{
					double x = log10(i.value());
					double y = log10(i.normalizedCount());
					++n;
					sumX += x;
					sumXSquare += x * x;
					sumY += y;
					sumXY += x * y;
				}
			}
			return (sumXY - sumX*sumY/n)/(sumXSquare - (sumX*sumX/n));
		}
		
		
		double PowerLawExponent(double startAmplitude) const
		{
			const long double xMin = startAmplitude;
			long double termSum = 0.0;
			long double termCount = 0.0;
			for(const_iterator i=begin();i!=end();++i)
			{
				const long double x = i.value();
				if(x >= startAmplitude)
				{
					const long double count = i.unnormalizedCount();
					const long double thisTerm = logl(x / xMin);
					termCount += count;
					termSum += thisTerm * count;
				}
			}
			return (double) (-1.0L - termCount / termSum);
		}
		
		double PowerLawExponentStdError(double startAmplitude, double expEstimator) const
		{
			long double termCount = 0.0;
			for(const_iterator i=begin();i!=end();++i)
			{
				if(i.value() >= startAmplitude)
				{
					const long double count = i.unnormalizedCount();
					termCount += count;
				}
			}
			return (double) ((-expEstimator - 1.0L) / sqrtl(termCount));
		}
		
		double NormalizedSlopeOffset(double startAmplitude, double endAmplitude, double slope) const
		{
			unsigned long n = 0;
			long double sumOffset = 0.0;
			for(const_iterator i=begin();i!=end();++i)
			{
				if(i.value() >= startAmplitude && i.value() < endAmplitude)
				{
					double y = log10(i.normalizedCount());
					double x = log10(i.value());
					double ySlope = x*slope;
					++n;
					sumOffset += (y - ySlope);
				}
			}
			return (double) (sumOffset/(long double) n);
		}
		
		double NormalizedSlopeStdError(double startAmplitude, double endAmplitude, double slope) const
		{
			long double ssxx = 0.0, ssxy = 0.0, ssyy = 0.0, xSum = 0.0, ySum = 0.0;
			unsigned long n = 0;
			// determine the 'average' x and y
			for(const_iterator i=begin();i!=end();++i)
			{
				if(i.value() >= startAmplitude && i.value() < endAmplitude)
				{
					xSum += log10(i.value());
					ySum += log10(i.normalizedCount());
					++n;
				}
			}
			const long double avgX = xSum / (long double) n, avgY = ySum / (long double) n;
			for(const_iterator i=begin();i!=end();++i)
			{
				if(i.value() >= startAmplitude && i.value() < endAmplitude)
				{
					long double y = log10(i.normalizedCount());
					long double x = log10(i.value());
					ssxx += (x-avgX)*(x-avgX);
					ssxy += (x-avgX)*(y-avgY);
					ssyy += (y-avgY)*(y-avgY);
				}
			}
			return (double) sqrtl((ssyy-slope*ssxy)/(ssxx * (long double) (n-2)));
		}
		
		double NormalizedSlopeStdDevBySampling(double startAmplitude, double endAmplitude, double slope, double stepFactor) const
		{
			long double sum = 0.0;
			unsigned long n = 0;
			if(stepFactor <= 1.0001) stepFactor = 1.0001;
			while(startAmplitude < endAmplitude)
			{
				const double stepEnd = startAmplitude * stepFactor;
				double sampledSlope = NormalizedSlope(startAmplitude, stepEnd);
				double sampleError = sampledSlope - slope;
				sum += sampleError * sampleError;
				++n;
				
				startAmplitude = stepEnd;
			}
			
			return (double) sqrtl(sum / ((long double) n*n - n));
		}
		
		double PowerLawUpperLimit(double constrainingAmplitude, double exponent, double factor) const
		{
			const double count = NormalizedCountAbove(constrainingAmplitude);
			const double term = count * (exponent+1.0)/factor + pow(constrainingAmplitude, exponent+1.0);
			return pow(term, 1.0/(exponent+1.0));
		}
		
		double PowerLawLowerLimit(double constrainingAmplitude, double exponent, double factor, double rfiRatio) const
		{
			const double countPart = NormalizedCountAbove(constrainingAmplitude);
			const double countTotal = NormalizedTotalCount() * rfiRatio;
			const double term = (countPart - countTotal) * (exponent+1.0)/factor + pow(constrainingAmplitude, exponent+1.0);
			return pow(term, 1.0/(exponent+1.0));
		}
		
		double PowerLawLowerLimit2(double constrainingAmplitude, double exponent, double factor, double rfiRatio) const
		{
			const double countPart = NormalizedCountAbove(constrainingAmplitude);
			const double countTotal = NormalizedTotalCount() * rfiRatio;
			const double term = (countPart - countTotal) * (exponent+1.0)/factor + pow(constrainingAmplitude, exponent+1.0);
			return pow(term/-exponent, 1.0/(exponent+1.0));
		}
		
		void GetRFIRegion(double &start, double &end) const
		{
			double sigmaEstimate = AmplitudeWithMaxNormalizedCount();
			double maxAmplitude = MaxAmplitude();
			start = sigmaEstimate * 20.0;
			double halfWay = exp((log(start) + log(maxAmplitude)) * 0.5);
			end = halfWay;
		}

		double NormalizedSlopeInRFIRegion() const
		{
			double start, end;
			GetRFIRegion(start ,end);
			return NormalizedSlope(start, end);
		}
		
		void SetData(std::vector<HistogramTablesFormatter::HistogramItem> &histogramData)
		{
			for(std::vector<HistogramTablesFormatter::HistogramItem>::const_iterator i=histogramData.begin(); i!=histogramData.end();++i)
			{
				const double b = (i->binStart + i->binEnd) * 0.5; // TODO somewhat inefficient...
				getBin(getCentralAmplitude(b)).count = (unsigned long) i->count;
			}
		}
		
		void Rescale(double factor)
		{
			std::map<double, AmplitudeBin> newAmplitudes;
			std::map<double, AmplitudeBin>::iterator position = newAmplitudes.begin();
			for(std::map<double, AmplitudeBin>::const_iterator i=_amplitudes.begin();
					i!=_amplitudes.end();++i)
			{
				position = newAmplitudes.insert(position, std::pair<double, AmplitudeBin>(getCentralAmplitude(i->first * factor), i->second));
			}
			_amplitudes = newAmplitudes;
		}
		
		class const_iterator
		{
			public:
				const_iterator(const LogHistogram &histogram, std::map<double, AmplitudeBin>::const_iterator iter) :
					_iterator(iter)
				{ }
				const_iterator(const const_iterator &source) :
					_iterator(source._iterator)
				{ }
				const_iterator &operator=(const const_iterator &source)
				{
					_iterator = source._iterator;
					return *this;
				}
				bool operator==(const const_iterator &other) const { return other._iterator == _iterator; }
				bool operator!=(const const_iterator &other) const { return other._iterator != _iterator; }
				const_iterator &operator++() { ++_iterator; return *this; }
				const_iterator &operator--() { --_iterator; return *this; }
				double value() const { return _iterator->first; }
				double normalizedCount() const { return _iterator->second.GetCount() / (binEnd() - binStart()); }
				long unsigned unnormalizedCount() const { return _iterator->second.GetCount(); }
				double binStart() const
				{
					return _iterator->first>0.0 ?
						pow10(log10(_iterator->first)-0.005) :
						-pow10(log10(-_iterator->first)-0.005);
				}
				double binEnd() const
				{
					return _iterator->first>0.0 ?
						pow10(log10(_iterator->first)+0.005) :
						-pow10(log10(-_iterator->first)+0.005);
				}
			private:
				std::map<double, AmplitudeBin>::const_iterator _iterator;
		};
		typedef const_iterator iterator;
		
		const_iterator begin() const
		{
			return const_iterator(*this, _amplitudes.begin());
		}
		
		const_iterator end() const
		{
			return const_iterator(*this, _amplitudes.end());
		}
		
		virtual void Serialize(std::ostream &stream) const
		{
			SerializeToUInt64(stream, _amplitudes.size());
			for(std::map<double, AmplitudeBin>::const_iterator i=_amplitudes.begin();i!=_amplitudes.end();++i)
			{
				SerializeToDouble(stream, i->first);
				i->second.Serialize(stream);
			}
		}
		
		virtual void Unserialize(std::istream &stream)
		{
			_amplitudes.clear();
			size_t mapSize = UnserializeUInt64(stream);
			std::map<double, AmplitudeBin>::iterator insertPos = _amplitudes.begin();
			for(size_t i=0;i!=mapSize;++i)
			{
				std::pair<double, AmplitudeBin> p;
				p.first = UnserializeDouble(stream);
				p.second.Unserialize(stream);
				insertPos = _amplitudes.insert(insertPos, p);
			}
		}
	private:
		std::map<double, AmplitudeBin> _amplitudes;
		
		AmplitudeBin &getBin(double centralAmplitude)
		{
			std::map<double, class AmplitudeBin>::iterator element =
				_amplitudes.find(centralAmplitude);
			
			if(element == _amplitudes.end())
			{
				element = _amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmplitude, AmplitudeBin())).first;
			}
			return element->second;
		}
		double binStart(double x) const
		{
			return x>0.0 ?
				pow10(log10(x)-0.005) :
				-pow10(log10(x)-0.005);
		}
		double binEnd(double x) const
		{
			return x>0.0 ?
				pow10(log10(x)+0.005) :
				-pow10(log10(x)+0.005);
		}
		
		static double getCentralAmplitude(const double amplitude)
		{
			if(amplitude>=0.0)
				return pow10(round(100.0*log10(amplitude))/100.0);
			else
				return -pow10(round(100.0*log10(-amplitude))/100.0);
		}
};

#endif
