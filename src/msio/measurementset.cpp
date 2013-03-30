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
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSTable.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ExprNode.h>

#include "measurementset.h"
#include "arraycolumniterator.h"
#include "scalarcolumniterator.h"
#include "date.h"

#include "../util/aologger.h"

#include "../strategy/control/strategywriter.h"

MeasurementSet::~MeasurementSet()
{
}

size_t MeasurementSet::BandCount(const std::string &location)
{
	casa::MeasurementSet ms(location);
	return ms.spectralWindow().nrow();
}

void MeasurementSet::initializeOtherData()
{
	casa::MeasurementSet ms(_path);
	initializeAntennas(ms);
	initializeBands(ms);
	initializeFields(ms);
}

void MeasurementSet::initializeAntennas(casa::MeasurementSet &ms)
{
	casa::Table antennaTable = ms.antenna();
	size_t count = antennaTable.nrow();
	casa::ROArrayColumn<double> positionCol(antennaTable, "POSITION"); 
	casa::ROScalarColumn<casa::String> nameCol(antennaTable, "NAME");
	casa::ROScalarColumn<double> diameterCol(antennaTable, "DISH_DIAMETER");
	casa::ROScalarColumn<casa::String> mountCol(antennaTable, "MOUNT");
	casa::ROScalarColumn<casa::String> stationCol(antennaTable, "STATION");

	_antennas.resize(count);
	for(size_t row=0; row!=count; ++row)
	{
		AntennaInfo info;
		info.diameter = diameterCol(row);
		info.id = row;
		info.name = nameCol(row);
		casa::Array<double> position = positionCol(row);
		casa::Array<double>::iterator i = position.begin();
		info.position.x = *i;
		++i;
		info.position.y = *i;
		++i;
		info.position.z = *i;
		info.mount = mountCol(row);
		info.station = stationCol(row);
		_antennas[row] = info;
	}
}

void MeasurementSet::initializeBands(casa::MeasurementSet &ms)
{
	casa::Table spectralWindowTable = ms.spectralWindow();
	casa::ROScalarColumn<int> numChanCol(spectralWindowTable, "NUM_CHAN");
	casa::ROArrayColumn<double> frequencyCol(spectralWindowTable, "CHAN_FREQ");

	_bands.resize(spectralWindowTable.nrow());
	for(size_t bandIndex=0; bandIndex!=spectralWindowTable.nrow(); ++bandIndex)
	{
		BandInfo band;
		band.windowIndex = bandIndex;
		size_t channelCount = numChanCol(bandIndex);

		const casa::Array<double> &frequencies = frequencyCol(bandIndex);
		casa::Array<double>::const_iterator frequencyIterator = frequencies.begin();

		for(unsigned channel=0;channel<channelCount;++channel) {
			ChannelInfo channelInfo;
			channelInfo.frequencyIndex = channel;
			channelInfo.frequencyHz = frequencies(casa::IPosition(1, channel));
			channelInfo.channelWidthHz = 0.0;
			channelInfo.effectiveBandWidthHz = 0.0;
			channelInfo.resolutionHz = 0.0;
			band.channels.push_back(channelInfo);

			++frequencyIterator;
		}

		_bands[bandIndex] = band;
	}
}

void MeasurementSet::initializeFields(casa::MeasurementSet &ms)
{
	casa::MSField fieldTable = ms.field();
	casa::ROArrayColumn<double> delayDirectionCol(fieldTable, fieldTable.columnName(casa::MSFieldEnums::DELAY_DIR) );
	casa::ROScalarColumn<casa::String> nameCol(fieldTable, fieldTable.columnName(casa::MSFieldEnums::NAME) );

	_fields.resize(fieldTable.nrow());
	for(size_t row=0; row!=fieldTable.nrow(); ++row)
	{
		const casa::Array<double> &delayDirection = delayDirectionCol(row);
		casa::Array<double>::const_iterator delayDirectionIterator = delayDirection.begin();
	
		FieldInfo field;
		field.fieldId = row;
		field.delayDirectionRA = *delayDirectionIterator;
		++delayDirectionIterator;
		field.delayDirectionDec = *delayDirectionIterator;
		field.name = nameCol(row);

		_fields[row] = field;
	}
}

void MeasurementSet::GetDataDescToBandVector(std::vector<size_t>& dataDescToBand)
{
	casa::MeasurementSet ms(_path);
	casa::MSDataDescription dataDescTable = ms.dataDescription();
	casa::ScalarColumn<int>
		spwIdCol(dataDescTable, dataDescTable.columnName(casa::MSDataDescriptionEnums::SPECTRAL_WINDOW_ID));
	dataDescToBand.resize(dataDescTable.nrow());
	for(size_t dataDescId=0;dataDescId!=dataDescTable.nrow();++dataDescId)
	{
		dataDescToBand[dataDescId] = spwIdCol(dataDescId);
	}
}

MSIterator::MSIterator(class MeasurementSet &ms, bool hasCorrectedData) : _row(0)
{
	_table = new casa::MeasurementSet(ms.Path());
	_antenna1Col = new casa::ROScalarColumn<int>(*_table, "ANTENNA1");
	_antenna2Col = new casa::ROScalarColumn<int>(*_table, "ANTENNA2");
	_dataCol = new casa::ROArrayColumn<casa::Complex>(*_table, "DATA");
	_flagCol = new casa::ROArrayColumn<bool>(*_table, "FLAG");
	if(hasCorrectedData)
		_correctedDataCol = new casa::ROArrayColumn<casa::Complex>(*_table, "CORRECTED_DATA");
	else
		_correctedDataCol = 0;
	_fieldCol = new casa::ROScalarColumn<int>(*_table, "FIELD_ID");
	_timeCol = new casa::ROScalarColumn<double>(*_table, "TIME");
	_scanNumberCol = new casa::ROScalarColumn<int>(*_table, "SCAN_NUMBER");
	_uvwCol = new casa::ROArrayColumn<double>(*_table, "UVW");
	_windowCol = new casa::ROScalarColumn<int>(*_table, "DATA_DESC_ID");
}

MSIterator::~MSIterator()
{
	delete _antenna1Col;
	delete _antenna2Col;
	delete _dataCol;
	if(_correctedDataCol != 0)
		delete _correctedDataCol;
	delete _flagCol;
	delete _timeCol;
	delete _fieldCol;
	delete _table;
	delete _scanNumberCol;
	delete _uvwCol;
	delete _windowCol;
}

void MeasurementSet::initializeMainTableData()
{
	if(!_isMainTableDataInitialized)
	{
		AOLogger::Debug << "Initializing ms cache data...\n"; 
		// we use a ptr to last, for faster insertion
		std::set<double>::iterator obsTimePos = _observationTimes.end();
		MSIterator iterator(*this, false);
		double time = -1.0;
		std::set<std::pair<size_t, size_t> > baselineSet;
		std::set<Sequence> sequenceSet;
		size_t prevFieldId = size_t(-1), sequenceId = size_t(-1);
		for(size_t row=0;row<iterator.TotalRows();++row)
		{
			size_t a1 = iterator.Antenna1();
			size_t a2 = iterator.Antenna2();
			size_t fieldId = iterator.Field();
			size_t spw = iterator.Window();
			double cur_time = iterator.Time();
			
			if(fieldId != prevFieldId)
			{
				prevFieldId = fieldId;
				sequenceId++;
				_observationTimesPerSequence.push_back(std::set<double>());
			}
			if(cur_time != time)
			{
				obsTimePos = _observationTimes.insert(obsTimePos, cur_time);
				_observationTimesPerSequence[sequenceId].insert(cur_time);
				time = cur_time;
			}
			
			baselineSet.insert(std::pair<size_t,size_t>(a1, a2));
			sequenceSet.insert(Sequence(a1, a2, spw, sequenceId, fieldId));
			
			++iterator;
		}
		for(std::set<std::pair<size_t, size_t> >::const_iterator i=baselineSet.begin(); i!=baselineSet.end(); ++i)
			_baselines.push_back(*i);
		for(std::set<Sequence>::const_iterator i=sequenceSet.begin(); i!=sequenceSet.end(); ++i)
			_sequences.push_back(*i);
		
		_isMainTableDataInitialized = true;
	}
}

size_t MeasurementSet::PolarizationCount()
{
	return PolarizationCount(Path());
}

size_t MeasurementSet::PolarizationCount(const std::string &filename)
{
	casa::MeasurementSet ms(filename);
	casa::Table polTable = ms.polarization();
	casa::ROArrayColumn<int> corTypeColumn(polTable, "CORR_TYPE"); 
	casa::Array<int> corType = corTypeColumn(0);
	casa::Array<int>::iterator iterend(corType.end());
	size_t polarizationCount = 0;
	for (casa::Array<int>::iterator iter=corType.begin(); iter!=iterend; ++iter)
	{
		++polarizationCount;
	}
	return polarizationCount;
}

bool MeasurementSet::HasRFIConsoleHistory()
{
	casa::MeasurementSet ms(_path);
	casa::Table histtab(ms.history());
	casa::ROScalarColumn<casa::String> application (histtab, "APPLICATION");
	for(unsigned i=0;i<histtab.nrow();++i)
	{
		if(application(i) == "AOFlagger")
			return true;
	}
	return false;
}

void MeasurementSet::GetAOFlaggerHistory(std::ostream &stream)
{
	casa::MeasurementSet ms(_path);
	casa::Table histtab(ms.history());
	casa::ROScalarColumn<double>       time        (histtab, "TIME");
	casa::ROScalarColumn<casa::String> application (histtab, "APPLICATION");
	casa::ROArrayColumn<casa::String>  cli         (histtab, "CLI_COMMAND");
	casa::ROArrayColumn<casa::String>  parms       (histtab, "APP_PARAMS");
	for(unsigned i=0;i<histtab.nrow();++i)
	{
		if(application(i) == "AOFlagger")
		{
			stream << "====================\n"
				"Command: " << cli(i)[0] << "\n"
				"Date: " << Date::AipsMJDToDateString(time(i)) << "\n"
				"Time: " << Date::AipsMJDToTimeString(time(i)) << "\n"
				"Strategy: \n     ----------     \n";
			const casa::Vector<casa::String> appParamsVec = parms(i);
			for(casa::Vector<casa::String>::const_iterator j=appParamsVec.begin();j!=appParamsVec.end();++j)
			{
				stream << *j << '\n';
			}
			stream << "     ----------     \n";
		}
	}
}

void MeasurementSet::AddAOFlaggerHistory(const rfiStrategy::Strategy &strategy, const std::string &commandline)
{
	// This has been copied from MSWriter.cc of NDPPP and altered (thanks, Ger!)
	casa::MeasurementSet ms(_path);
	casa::Table histtab(ms.keywordSet().asTable("HISTORY"));
	histtab.reopenRW();
	casa::ScalarColumn<double>       time        (histtab, "TIME");
	casa::ScalarColumn<int>          obsId       (histtab, "OBSERVATION_ID");
	casa::ScalarColumn<casa::String> message     (histtab, "MESSAGE");
	casa::ScalarColumn<casa::String> application (histtab, "APPLICATION");
	casa::ScalarColumn<casa::String> priority    (histtab, "PRIORITY");
	casa::ScalarColumn<casa::String> origin      (histtab, "ORIGIN");
	casa::ArrayColumn<casa::String>  parms       (histtab, "APP_PARAMS");
	casa::ArrayColumn<casa::String>  cli         (histtab, "CLI_COMMAND");
	// Put all parset entries in a Vector<String>.
	// Some WSRT MSs have a FixedShape APP_PARAMS and CLI_COMMAND column.
	// For them, put the xml file in a single vector element (with newlines).
	bool fixedShaped =
		(parms.columnDesc().options() & casa::ColumnDesc::FixedShape) != 0;

	std::ostringstream ostr;
	rfiStrategy::StrategyWriter writer;
	writer.WriteToStream(strategy, ostr);

	casa::Vector<casa::String> appParamsVec;
	casa::Vector<casa::String> clivec;
	clivec.resize(1);
	clivec[0] = commandline;
	if (fixedShaped) {
		appParamsVec.resize(1);
		appParamsVec[0] = ostr.str();
	} else {
		// Tokenize the string on '\n'
		const std::string str = ostr.str();
		size_t lineCount = std::count(str.begin(), str.end(), '\n');
		appParamsVec.resize(lineCount+1);
		casa::Array<casa::String>::contiter viter = appParamsVec.cbegin();
		size_t curStringPos = 0;
		for(size_t i=0;i<lineCount;++i)
		{
			size_t endPos = str.find('\n', curStringPos);
			*viter = str.substr(curStringPos, endPos-curStringPos);
			++viter;
			curStringPos = endPos + 1;
		}
		if(curStringPos < str.size())
		{
			*viter = str.substr(curStringPos, str.size()-curStringPos);
		}
	}
	uint rownr = histtab.nrow();
	histtab.addRow();
	time.put        (rownr, casa::Time().modifiedJulianDay()*24.0*3600.0);
	obsId.put       (rownr, 0);
	message.put     (rownr, "parameters");
	application.put (rownr, "AOFlagger");
	priority.put    (rownr, "NORMAL");
	origin.put      (rownr, "standalone");
	parms.put       (rownr, appParamsVec);
	cli.put         (rownr, clivec);
}

std::string MeasurementSet::GetStationName() const
{
	casa::MeasurementSet ms(_path);
	casa::Table antennaTable(ms.antenna());
	if(antennaTable.nrow() == 0)
		throw std::runtime_error("GetStationName() : no rows in Antenna table");
	casa::ROScalarColumn<casa::String> stationColumn(antennaTable, "STATION");
	return stationColumn(0);
}

bool MeasurementSet::IsChannelZeroRubish()
{
	try
	{
		const std::string station = GetStationName();
		if(station != "LOFAR") return false;
		// This is of course a hack, but its the best estimate we can make :-/ (easily)
		const BandInfo bandInfo = GetBandInfo(0);
		return (bandInfo.channels.size() == 256 || bandInfo.channels.size()==64);
	} catch(std::exception &e)
	{
		return false;
	}
}
