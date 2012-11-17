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
#ifndef HISTOGRAM_COLLECTION_H
#define HISTOGRAM_COLLECTION_H

#include "loghistogram.h"

#include <complex>
#include <map>
#include <vector>

#include "../msio/image2d.h"
#include "../msio/mask2d.h"

#include "../util/serializable.h"

class HistogramCollection : public Serializable
{
	public:
		typedef std::pair<unsigned, unsigned> AntennaPair;
		
		HistogramCollection() : _polarizationCount(0)
		{
		}
		
		explicit HistogramCollection(unsigned polarizationCount) : _polarizationCount(polarizationCount)
		{
			init();
		}
		
		HistogramCollection(const HistogramCollection &source) : _polarizationCount(source._polarizationCount)
		{
			init();
			for(unsigned i=0;i<_polarizationCount;++i)
			{
				copy(_totalHistograms[i], source._totalHistograms[i]);
				copy(_rfiHistograms[i], source._rfiHistograms[i]);
			}
		}
		
		~HistogramCollection()
		{
			destruct();
		}
		
		void SetPolarizationCount(unsigned polarizationCount)
		{
			destruct();
			_polarizationCount = polarizationCount;
			init();
		}
		
		void Add(const unsigned antenna1, const unsigned antenna2, const unsigned polarization, const std::complex<float> *values, const bool *isRFI, size_t sampleCount)
		{
			LogHistogram &totalHistogram = GetTotalHistogram(antenna1, antenna2, polarization);
			LogHistogram &rfiHistogram = GetRFIHistogram(antenna1, antenna2, polarization);
			
			for(size_t i=0;i<sampleCount;++i)
			{
				const double amplitude = sqrt(values[i].real()*values[i].real() + values[i].imag()*values[i].imag());
				totalHistogram.Add(amplitude);
				if(isRFI[i])
					rfiHistogram.Add(amplitude);
			}
		}
		
		void Add(const HistogramCollection &collection)
		{
			if(collection._polarizationCount != _polarizationCount)
				throw std::runtime_error("Polarization counts of histogram collections don't match");
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				add(collection, p, p);
			}
		}
		
		bool Empty() const
		{
			if(_polarizationCount == 0)
				return true;
			for(unsigned p=0;p!=_polarizationCount;++p)
			{
				if(!_totalHistograms[p].empty() || !_rfiHistograms[p].empty())
					return false;
			}
			return true;
		}
		
		void Add(const unsigned antenna1, const unsigned antenna2, const unsigned polarization, Image2DCPtr image, Mask2DCPtr mask);
		
		LogHistogram &GetTotalHistogram(const unsigned a1, const unsigned a2, const unsigned polarization)
		{
			return getHistogram(_totalHistograms, a1, a2, polarization);
		}
		
		LogHistogram &GetRFIHistogram(const unsigned a1, const unsigned a2, const unsigned polarization)
		{
			return getHistogram(_rfiHistograms, a1, a2, polarization);
		}
		
		const std::map<AntennaPair, LogHistogram*> &GetTotalHistogram(const unsigned polarization) const
		{
			return _totalHistograms[polarization];
		}
		
		const std::map<AntennaPair, LogHistogram*> &GetRFIHistogram(const unsigned polarization) const
		{
			return _rfiHistograms[polarization];
		}
		
		void GetTotalHistogramForCrossCorrelations(const unsigned polarization, LogHistogram &target) const
		{
			getHistogramForCrossCorrelations(_totalHistograms, polarization, target);
		}
		
		void GetRFIHistogramForCrossCorrelations(const unsigned polarization, LogHistogram &target) const
		{
			getHistogramForCrossCorrelations(_rfiHistograms, polarization, target);
		}
		
		void Clear()
		{
			destruct();
			init();
		}
		
		void Save(class HistogramTablesFormatter &histogramTables);
		
		void Load(class HistogramTablesFormatter &histogramTables);
		
		unsigned PolarizationCount() const { return _polarizationCount; }
		
		virtual void Serialize(std::ostream &stream) const
		{
			SerializeToUInt64(stream, _polarizationCount);
			serializeMapArray(stream, _totalHistograms);
			serializeMapArray(stream, _rfiHistograms);
		}
		
		virtual void Unserialize(std::istream &stream)
		{
			destruct();
			_polarizationCount = UnserializeUInt64(stream);
			init();
			unserializeMapArray(stream, _totalHistograms);
			unserializeMapArray(stream, _rfiHistograms);
		}
		
		HistogramCollection *CreateSummedPolarizationCollection() const
		{
			HistogramCollection *newCollection = new HistogramCollection(1);
			for(unsigned p=0;p<_polarizationCount;++p)
				newCollection->add(*this, p, 0);
			return newCollection;
		}
		
		void Rescale(double factor)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				for(std::map<AntennaPair, LogHistogram*>::iterator i=_totalHistograms[p].begin(); i!=_totalHistograms[p].end(); ++i)
				{
					i->second->Rescale(factor);
				}
				for(std::map<AntennaPair, LogHistogram*>::iterator i=_rfiHistograms[p].begin(); i!=_rfiHistograms[p].end(); ++i)
				{
					i->second->Rescale(factor);
				}
			}
		}
	private:
		unsigned _polarizationCount;
		std::map<AntennaPair, LogHistogram*> *_totalHistograms;
		std::map<AntennaPair, LogHistogram*> *_rfiHistograms;
		
		void init()
		{
			if(_polarizationCount != 0)
			{
				_totalHistograms = new std::map<AntennaPair, LogHistogram*>[_polarizationCount];
				_rfiHistograms = new std::map<AntennaPair, LogHistogram*>[_polarizationCount];
			} else {
				_totalHistograms = 0;
				_rfiHistograms = 0;
			}
		}
		
		void destruct()
		{
			if(_polarizationCount != 0)
			{
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					for(std::map<AntennaPair, LogHistogram*>::iterator i=_totalHistograms[p].begin(); i!=_totalHistograms[p].end(); ++i)
					{
						delete i->second;
					}
					for(std::map<AntennaPair, LogHistogram*>::iterator i=_rfiHistograms[p].begin(); i!=_rfiHistograms[p].end(); ++i)
					{
						delete i->second;
					}
				}
				delete[] _totalHistograms;
				delete[] _rfiHistograms;
			}
		}
		
		void serializeMapArray(std::ostream &stream, const std::map<AntennaPair, LogHistogram*> *map) const
		{
			for(unsigned p=0;p<_polarizationCount;++p)
				serializeMap(stream, map[p]);
		}
		
		void unserializeMapArray(std::istream &stream, std::map<AntennaPair, LogHistogram*> *map)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
				unserializeMap(stream, map[p]);
		}
		
		void serializeMap(std::ostream &stream, const std::map<AntennaPair, LogHistogram*> &map) const
		{
			SerializeToUInt64(stream, map.size());
			for(std::map<AntennaPair, LogHistogram*>::const_iterator i=map.begin();i!=map.end();++i)
			{
				const AntennaPair &antennae = i->first;
				const LogHistogram *histogram = i->second;
				SerializeToUInt32(stream, antennae.first);
				SerializeToUInt32(stream, antennae.second);
				histogram->Serialize(stream);
			}
		}
		
		void unserializeMap(std::istream &stream, std::map<AntennaPair, LogHistogram*> &map)
		{
			map.clear();
			size_t mapSize = UnserializeUInt64(stream);
			std::map<AntennaPair, LogHistogram*>::iterator insertPos = map.begin();
			for(size_t i=0;i!=mapSize;++i)
			{
				std::pair<AntennaPair, LogHistogram*> p;
				p.first.first = UnserializeUInt32(stream);
				p.first.second = UnserializeUInt32(stream);
				p.second = new LogHistogram();
				p.second->Unserialize(stream);
				insertPos = map.insert(insertPos, p);
			}
		}
		
		void copy(std::map<AntennaPair, LogHistogram*> &destination, const std::map<AntennaPair, LogHistogram*> &source)
		{
			for(std::map<AntennaPair, LogHistogram*>::const_iterator i=source.begin();i!=source.end();++i)
			{
				destination.insert(std::pair<AntennaPair, LogHistogram*>(i->first, new LogHistogram(*i->second)));
			}
		}

		void add(const HistogramCollection &collection, unsigned fromPolarization, unsigned toPolarization)
		{
			for(std::map<AntennaPair, LogHistogram*>::const_iterator i=collection._totalHistograms[fromPolarization].begin(); i!=collection._totalHistograms[fromPolarization].end(); ++i)
			{
				LogHistogram &histogram = GetTotalHistogram(i->first.first, i->first.second, toPolarization);
				histogram.Add(*i->second);
			}
			
			for(std::map<AntennaPair, LogHistogram*>::const_iterator i=collection._rfiHistograms[fromPolarization].begin(); i!=collection._rfiHistograms[fromPolarization].end(); ++i)
			{
				LogHistogram &histogram = GetRFIHistogram(i->first.first, i->first.second, toPolarization);
				histogram.Add(*i->second);
			}
		}
		
		LogHistogram &getHistogram(std::map<AntennaPair, LogHistogram*> *histograms, const unsigned a1, const unsigned a2, const unsigned polarization)
		{
			const AntennaPair antennae(a1, a2);
			std::map<AntennaPair, LogHistogram*>::iterator i = histograms[polarization].find(antennae);
			if(i == histograms[polarization].end())
			{
				i = histograms[polarization].insert(std::pair<AntennaPair, LogHistogram*>(antennae, new LogHistogram())).first;
			}
			return *i->second;
		}
		
		void getHistogramForCrossCorrelations(std::map<AntennaPair, LogHistogram*> *histograms, const unsigned polarization, LogHistogram &target) const
		{
			for(std::map<AntennaPair, LogHistogram*>::const_iterator i=histograms[polarization].begin(); i!=histograms[polarization].end(); ++i)
			{
				if(i->first.first != i->first.second)
					target.Add(*i->second);
			}
		}
};

#endif
