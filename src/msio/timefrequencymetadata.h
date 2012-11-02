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

#ifndef MSIO_TIME_FREQUENCY_META_DATA_H
#define MSIO_TIME_FREQUENCY_META_DATA_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "antennainfo.h"

typedef boost::shared_ptr<class TimeFrequencyMetaData> TimeFrequencyMetaDataPtr;
typedef boost::shared_ptr<const class TimeFrequencyMetaData> TimeFrequencyMetaDataCPtr;

class TimeFrequencyMetaData
{
	public:
		TimeFrequencyMetaData()
			: _antenna1(0), _antenna2(0), _band(0), _field(0), _observationTimes(0), _uvw(0), _dataDescription("Visibility"), _dataUnits("Jy")
		{
		}
		TimeFrequencyMetaData(const AntennaInfo &antenna1, const AntennaInfo &antenna2, const BandInfo &band, const FieldInfo &field, const std::vector<double> &observationTimes)
		:
			_antenna1(new AntennaInfo(antenna1)),
			_antenna2(new AntennaInfo(antenna2)),
			_band(new BandInfo(band)),
			_field(new FieldInfo(field)),
			_observationTimes(new std::vector<double>(observationTimes)),
			_uvw(0),
			_dataDescription("Visibility"),
			_dataUnits("Jy")
		{
		}
		TimeFrequencyMetaData(const TimeFrequencyMetaData &source)
			: _antenna1(0), _antenna2(0), _band(0), _field(0), _observationTimes(0), _uvw(0),
			_dataDescription(source._dataDescription),
			_dataUnits(source._dataUnits)
		{
			if(source._antenna1 != 0)
				_antenna1 = new AntennaInfo(*source._antenna1);
			if(source._antenna2 != 0)
				_antenna2 = new AntennaInfo(*source._antenna2);
			if(source._band != 0)
				_band = new BandInfo(*source._band);
			if(source._field != 0)
				_field = new FieldInfo(*source._field);
			if(source._observationTimes != 0)
				_observationTimes = new std::vector<double>(*source._observationTimes);
			if(source._uvw != 0)
				_uvw = new std::vector<class UVW>(*source._uvw);
		}
		~TimeFrequencyMetaData()
		{
			ClearAntenna1();
			ClearAntenna2();
			ClearBand();
			ClearField();
			ClearObservationTimes();
			ClearUVW();
		}

		const AntennaInfo &Antenna1() const { return *_antenna1; }
		void ClearAntenna1()
		{
			if(_antenna1 != 0)
			{
				delete _antenna1;
				_antenna1 = 0;
			}
		}
		void SetAntenna1(const AntennaInfo &antenna1)
		{
			ClearAntenna1();
			_antenna1 = new AntennaInfo(antenna1);
		}
		bool HasAntenna1() const { return _antenna1 != 0; }

		const AntennaInfo &Antenna2() const { return *_antenna2; }
		void ClearAntenna2()
		{
			if(_antenna2 != 0)
			{
				delete _antenna2;
				_antenna2 = 0;
			}
		}
		void SetAntenna2(const AntennaInfo &antenna2)
		{
			ClearAntenna2();
			_antenna2 = new AntennaInfo(antenna2);
		}
		bool HasAntenna2() const { return _antenna2 != 0; }

		const BandInfo &Band() const { return *_band; }
		void ClearBand()
		{
			if(_band != 0)
			{
				delete _band;
				_band = 0;
			}
		}
		void SetBand(const BandInfo &band)
		{
			ClearBand();
			_band = new BandInfo(band);
		}
		bool HasBand() const { return _band != 0; }

		const FieldInfo &Field() const { return *_field; }
		void ClearField()
		{
			if(_field != 0)
			{
				delete _field;
				_field = 0;
			}
		}
		void SetField(const FieldInfo &field)
		{
			ClearField();
			_field = new FieldInfo(field);
		}
		bool HasField() const { return _field != 0; }

		const std::vector<double> &ObservationTimes() const {
			return *_observationTimes;
		}
		void ClearObservationTimes()
		{
			if(_observationTimes != 0)
			{
				delete _observationTimes;
				_observationTimes = 0;
			}
		}
		void SetObservationTimes(const std::vector<double> &times)
		{
			ClearObservationTimes();
			_observationTimes = new std::vector<double>(times);
		}
		bool HasObservationTimes() const { return _observationTimes != 0; }

		const std::vector<class UVW> &UVW() const { return *_uvw; }
		void ClearUVW()
		{
			if(_uvw != 0)
			{
				delete _uvw;
				_uvw = 0;
			}
		}
		void SetUVW(const std::vector<class UVW> &uvw)
		{
			ClearUVW();
			_uvw = new std::vector<class UVW>(uvw);
		}
		bool HasUVW() const { return _uvw != 0; }

		bool HasBaseline() const {
			return HasAntenna1() && HasAntenna2();
		}
		class Baseline Baseline() const {
			return ::Baseline(*_antenna1, *_antenna2);
		}
		const std::string &DataDescription() const { return _dataDescription; }
		void SetDataDescription(const std::string &dataDescription)
		{
			_dataDescription = dataDescription;
		}
		
		const std::string &DataUnits() const { return _dataUnits; }
		void SetDataUnits(const std::string &dataUnits)
		{
			_dataUnits = dataUnits;
		}
	private:
		void operator=(const TimeFrequencyMetaData &) { }
		
		class AntennaInfo *_antenna1;
		class AntennaInfo *_antenna2;
		class BandInfo *_band;
		class FieldInfo *_field;
		std::vector<double> *_observationTimes;
		std::vector<class UVW> *_uvw;
		std::string _dataDescription, _dataUnits;
};

#endif // MSIO_TIME_FREQUENCY_META_DATA_H
