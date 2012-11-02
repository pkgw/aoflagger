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
#ifndef MEASUREMENTSET_H
#define MEASUREMENTSET_H

#include <string>
#include <vector>
#include <utility>
#include <set>

#include <ms/MeasurementSets/MSColumns.h>
#include <tables/Tables/DataManager.h>

#include "../strategy/control/types.h"

#include "antennainfo.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/

class MSIterator {
	public:
		MSIterator(class MeasurementSet &ms, bool hasCorrectedData=true);
		~MSIterator();
		MSIterator &operator++() { _row++; return *this; }
		size_t TotalRows() { return _table->nrow(); }

		casa::Complex Data(unsigned frequencyIndex, unsigned polarisation)
		{
			return (*_dataCol)(_row)(casa::IPosition(2, frequencyIndex, polarisation));
		}

		bool Flag(unsigned frequencyIndex, unsigned polarisation) { return (*_flagCol)(_row)(casa::IPosition(2, frequencyIndex, polarisation)); }
		casa::Array<bool>::const_iterator FlagIterator()
		{
			return (*_flagCol)(_row).begin();
		}

		casa::Complex CorrectedData(unsigned frequencyIndex, unsigned polarisation)
		{
			return (*_correctedDataCol)(_row)(casa::IPosition(2, frequencyIndex, polarisation));
		}

		casa::Array<casa::Complex>::const_iterator CorrectedDataIterator()
		{
			return (*_correctedDataCol)(_row).begin();
		}

		int Field() { return (*_fieldCol)(_row); }
		double Time() { return (*_timeCol)(_row); }
		unsigned Antenna1() { return (*_antenna1Col)(_row); }
		unsigned Antenna2() { return (*_antenna2Col)(_row); }
		unsigned ScanNumber() { return (*_scanNumberCol)(_row); }
		class UVW UVW() {
			class UVW uvw;
			casa::Array<double> arr = (*_uvwCol)(_row);
			casa::Array<double>::const_iterator i = arr.begin();
			uvw.u = *i; ++i;
			uvw.v = *i; ++i;
			uvw.w = *i;
			return uvw;
		}
		unsigned Window() { return (*_windowCol)(_row); }
	private:
		unsigned long _row;
		casa::ROScalarColumn<int> *_antenna1Col;
		casa::ROScalarColumn<int> *_antenna2Col;
		casa::ROArrayColumn<casa::Complex> *_dataCol;
		casa::ROArrayColumn<bool> *_flagCol;
		casa::ROArrayColumn<casa::Complex> *_correctedDataCol;
		casa::ROScalarColumn<double> *_timeCol;
		casa::ROScalarColumn<int> *_fieldCol;
		casa::ROScalarColumn<int> *_scanNumberCol;
		casa::ROArrayColumn<double> *_uvwCol;
		casa::ROScalarColumn<int> *_windowCol;
		casa::Table *_table;
};

class MeasurementSet {
	public:
		MeasurementSet(const std::string &location) throw()
			: _location(location), _maxSpectralBandIndex(-1),
			_maxFrequencyIndex(-1), _maxScanIndex(-1), _cacheInitialized(false)
		{
		}
		MeasurementSet(const std::string &newLocation, const MeasurementSet &formatExample);
		~MeasurementSet();
		casa::Table *OpenTable(bool update = false) const;
		size_t MaxSpectralBandIndex();
		size_t FrequencyCount();
		size_t TimestepCount()
		{
			if(_maxScanIndex==-1)
				CalculateScanCounts();
			return _maxScanIndex;
		}
		size_t MaxScanIndex()
		{
			if(_maxScanIndex==-1)
				CalculateScanCounts();
			return _maxScanIndex;
		}
		size_t MinScanIndex()
		{
			if(_maxScanIndex==-1)
				CalculateScanCounts();
			return _minScanIndex;
		}
		size_t GetPolarizationCount();
		static size_t GetPolarizationCount(const std::string &filename);
		static struct BandInfo GetBandInfo(const std::string &filename, unsigned bandIndex);
		size_t AntennaCount();
		size_t FieldCount();
		size_t BandCount() { return BandCount(_location); }
		static size_t BandCount(const std::string &filename);
		struct AntennaInfo GetAntennaInfo(unsigned antennaId);
		struct BandInfo GetBandInfo(unsigned bandIndex) {return GetBandInfo(_location, bandIndex);}
		struct FieldInfo GetFieldInfo(unsigned fieldIndex);
		void DataMerge(const MeasurementSet &source);
		std::string Location() const throw() { return _location; }
		void GetBaselines(std::vector<std::pair<size_t,size_t> > &baselines)
		{
			if(!_cacheInitialized)
				InitCacheData();
			baselines = _baselines;
		}
		const std::set<double> &GetObservationTimesSet()
		{
			if(!_cacheInitialized)
				InitCacheData();
			return _observationTimes;
		}
		std::vector<double> *CreateObservationTimesVector()
		{
			if(!_cacheInitialized)
				InitCacheData();
			std::vector<double> *times = new std::vector<double>();
			for(std::set<double>::const_iterator i=_observationTimes.begin();i!=_observationTimes.end();++i)
				times->push_back(*i);
			return times;
		}
		bool HasRFIConsoleHistory();
		void GetAOFlaggerHistory(std::ostream &stream);
		void AddAOFlaggerHistory(const class rfiStrategy::Strategy &strategy, const std::string &commandline);
		std::string GetStationName() const;
		bool ChannelZeroIsRubish();
	private:
		void InitCacheData();
		void CalculateScanCounts();

		const std::string _location;
		int _maxSpectralBandIndex;
		int _maxFrequencyIndex;
		int _maxScanIndex, _minScanIndex;

		bool _cacheInitialized;
		std::vector<std::pair<size_t,size_t> > _baselines;
		std::set<double> _observationTimes;
};

#endif
