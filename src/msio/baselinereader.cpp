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
#include "baselinereader.h"

#include <set>
#include <stdexcept>

#include <ms/MeasurementSets/MeasurementSet.h>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/IncrStManAccessor.h>
#include <tables/Tables/StandardStManAccessor.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TiledStManAccessor.h>

#include "timefrequencydata.h"

#include "../util/aologger.h"

BaselineReader::BaselineReader(const std::string &msFile)
	: _measurementSet(msFile), _dataColumnName("DATA"), _subtractModel(false), _readData(true), _readFlags(true),
	_polarizationCount(0)
{
	AOLogger::Debug << "Baselinereader constructed.\n";
	_frequencyCount = _measurementSet.FrequencyCount();
	try {
		_table = _measurementSet.OpenTable(true);
	} catch(std::exception &e)
	{
		AOLogger::Warn << "Read-write opening of file " << msFile << " failed, trying read-only...\n";
		_table = _measurementSet.OpenTable(false);
		AOLogger::Warn << "Table opened in read-only: writing not possible.\n";
	}
}
// 
BaselineReader::~BaselineReader()
{
	delete _table;
}

void BaselineReader::initObservationTimes()
{
	if(_observationTimes.size() == 0)
	{
		AOLogger::Debug << "Initializing observation times...\n";
		const std::set<double> &times = _measurementSet.GetObservationTimesSet();
		unsigned index = 0;
		for(std::set<double>::const_iterator i=times.begin();i!=times.end();++i)
		{
			_observationTimes.insert(std::pair<double,size_t>(*i, index));
			_observationTimesVector.push_back(*i);
			++index;
		}
	}
}

void BaselineReader::AddReadRequest(size_t antenna1, size_t antenna2, size_t spectralWindow)
{
	initObservationTimes();
	
	addReadRequest(antenna1, antenna2, spectralWindow, 0, _observationTimes.size());
}

TimeFrequencyData BaselineReader::GetNextResult(std::vector<class UVW> &uvw)
{
	size_t requestIndex = 0;
	TimeFrequencyData data;
	if(_polarizationCount == 4)
	{
		data = TimeFrequencyData(
			_results[requestIndex]._realImages[0], _results[requestIndex]._imaginaryImages[0],
			_results[requestIndex]._realImages[1], _results[requestIndex]._imaginaryImages[1],
			_results[requestIndex]._realImages[2], _results[requestIndex]._imaginaryImages[2],
			_results[requestIndex]._realImages[3], _results[requestIndex]._imaginaryImages[3]
			);
		data.SetIndividualPolarisationMasks(
			_results[requestIndex]._flags[0],
			_results[requestIndex]._flags[1],
			_results[requestIndex]._flags[2],
			_results[requestIndex]._flags[3]);
	} else if(_polarizationCount == 2)
	{
		data = TimeFrequencyData(AutoDipolePolarisation,
			_results[requestIndex]._realImages[0], _results[requestIndex]._imaginaryImages[0],
			_results[requestIndex]._realImages[1], _results[requestIndex]._imaginaryImages[1]);
		data.SetIndividualPolarisationMasks(
			_results[requestIndex]._flags[0],
			_results[requestIndex]._flags[1]);
	} else if(_polarizationCount == 1)
	{
		data = TimeFrequencyData(StokesIPolarisation,
			_results[requestIndex]._realImages[0], _results[requestIndex]._imaginaryImages[0]);
		data.SetGlobalMask(_results[requestIndex]._flags[0]);
	}
	uvw = _results[0]._uvw;
	
	_results.erase(_results.begin() + requestIndex);

	return data;
}

void BaselineReader::PartInfo(size_t maxTimeScans, size_t &timeScanCount, size_t &partCount)
{
	initObservationTimes();

	timeScanCount = _observationTimes.size();
	if(maxTimeScans == 0)
		partCount = 1;
	else
	{
		partCount = (timeScanCount + maxTimeScans - 1) / maxTimeScans;
		if(partCount == 0)
			partCount = 1;
	}
}

void BaselineReader::initializePolarizations()
{
	if(_polarizationCount == 0)
	{
		casa::MeasurementSet ms(_measurementSet.Location());
		casa::Table polTable = ms.polarization();
		casa::ROArrayColumn<int> corTypeColumn(polTable, "CORR_TYPE"); 
		casa::Array<int> corType = corTypeColumn(0);
		casa::Array<int>::iterator iterend(corType.end());
		int polarizationCount = 0;
		for (casa::Array<int>::iterator iter=corType.begin(); iter!=iterend; ++iter)
		{
			switch(*iter) {
				case 1: //_stokesIIndex = polarizationCount; break;
				case 5:
				case 9: //_xxIndex = polarizationCount; break;
				case 6:
				case 10:// _xyIndex = polarizationCount; break;
				case 7:
				case 11:// _yxIndex = polarizationCount; break;
				case 8:
				case 12: //_yyIndex = polarizationCount; break;
				break;
				default:
				{
					std::stringstream s;
					s << "There is a polarization in the measurement set that I can not handle (" << *iter << ", polarization index " << polarizationCount << ").";
					throw std::runtime_error(s.str());
				}
			}
			++polarizationCount;
		}
		_polarizationCount = polarizationCount;
	}
}

void BaselineReader::clearTableCaches()
{
	/*
	try {
		casa::ROTiledStManAccessor accessor(*Table(), "LofarStMan");
		accessor.clearCaches();
		AOLogger::Debug << "LofarStMan Caches cleared with ROTiledStManAccessor.\n";
	} catch(std::exception &e)
	{
		try {
			AOLogger::Debug << e.what() << '\n';
			casa::ROStandardStManAccessor accessor(*Table(), "LofarStMan");
			accessor.clearCache();
			AOLogger::Debug << "LofarStMan Caches cleared with ROStandardStManAccessor.\n";
		} catch(std::exception &e)
		{
			try {
				AOLogger::Debug << e.what() << '\n';
				casa::ROIncrementalStManAccessor accessor(*Table(), "LofarStMan");
				accessor.clearCache();
				AOLogger::Debug << "LofarStMan Caches cleared with ROIncrementalStManAccessor.\n";
			} catch(std::exception &e)
			{
				AOLogger::Debug << e.what() << '\n';
				AOLogger::Debug << "Could not clear LofarStMan caches; don't know how to access it.\n";
			}
		}
	}*/
}
