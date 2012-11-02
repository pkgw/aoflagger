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

MeasurementSet::MeasurementSet(const std::string &newLocation, const MeasurementSet &formatExample)
	: _location(newLocation), _maxSpectralBandIndex(-1),
	_maxFrequencyIndex(-1), _maxScanIndex(-1), _cacheInitialized(false)
{
	casa::Table *table = formatExample.OpenTable(false);
	table->copy(newLocation, casa::Table::New, true);
	delete table;
}

MeasurementSet::~MeasurementSet()
{
}

casa::Table *MeasurementSet::OpenTable(bool update) const
{
	std::string tableLocation = _location;
	casa::Table *table;
	if(update)
		table = new casa::Table(tableLocation, casa::Table::Update);
	else
		table = new casa::Table(tableLocation);
	return table;
}

size_t MeasurementSet::MaxSpectralBandIndex()
{
	if(_maxSpectralBandIndex==-1) {
		casa::Table *table = OpenTable();
		casa::ROScalarColumn<int> windowColumn(*table, "DATA_DESC_ID");
		ScalarColumnIterator<int> windowIter = ScalarColumnIterator<int>::First(windowColumn);
		for(size_t i=0;i<table->nrow();++i,++windowIter) {
			if((*windowIter) > _maxSpectralBandIndex)
				_maxSpectralBandIndex = (*windowIter);
		}
		delete table;
	}
	return _maxSpectralBandIndex;
}

size_t MeasurementSet::FrequencyCount()
{
	if(_maxFrequencyIndex==-1) {
		casa::Table *table = OpenTable();
		casa::ROArrayColumn<casa::Complex> dataColumn(*table, "DATA");
		if(table->nrow() > 0) {
			const casa::IPosition &shape = dataColumn.shape(0);
			if(shape.nelements() > 1)
				_maxFrequencyIndex = shape[1];
			else
				_maxFrequencyIndex = 0;
		} else 
			_maxFrequencyIndex = 0;
		delete table;
	}
	return _maxFrequencyIndex;
}

size_t MeasurementSet::BandCount(const std::string &location)
{
	casa::MeasurementSet ms(location);
	casa::Table spwTable = ms.spectralWindow();
	size_t count = spwTable.nrow();
	return count;
}

void MeasurementSet::CalculateScanCounts()
{
	if(_maxScanIndex==-1) {
		casa::Table *table = OpenTable();
		casa::ROScalarColumn<int> scanColumn(*table, "SCAN_NUMBER");
		ScalarColumnIterator<int> scanIter = ScalarColumnIterator<int>::First(scanColumn);
		for(size_t i=0;i<table->nrow();++i,++scanIter) {
			if((*scanIter) + 1 > _maxScanIndex)
				_maxScanIndex = (*scanIter) + 1;
			if((*scanIter) + 1 < _minScanIndex)
				_minScanIndex = (*scanIter) + 1;
		}
		delete table;
	}
}

void MeasurementSet::DataMerge(const MeasurementSet &source)
{
	casa::Table *sourceTable = source.OpenTable();

	unsigned newRows = sourceTable->nrow();
	unsigned sourceCols = sourceTable->tableDesc().ncolumn();

	casa::Table *destTable = OpenTable(true);
	unsigned rowIndex = destTable->nrow();

	AOLogger::Debug << "Adding " << newRows << " new rows...\n";
	destTable->addRow (newRows, false);

	AOLogger::Debug << "Copying cells " << rowIndex << "-" << (newRows+rowIndex) << " for all columns ...\n";

	for(unsigned i=0;i<sourceCols;++i)
	{
		const std::string name = sourceTable->tableDesc().columnNames()[i];
		if(name != "FLAG_CATEGORY" && name != "WEIGHT_SPECTRUM") {
			if(i>0)
				AOLogger::Debug << ",";
			AOLogger::Debug << name;
			casa::ROTableColumn sourceColumn = casa::ROTableColumn(*sourceTable, name);
			casa::TableColumn destColumn = casa::TableColumn(*destTable, name);
			for(unsigned j=0;j<newRows;++j)
				destColumn.put(rowIndex+j, sourceColumn, j);
		}
	}
	AOLogger::Debug << '\n';

	delete destTable;
	delete sourceTable;
}

size_t MeasurementSet::AntennaCount()
{
	casa::MeasurementSet ms(_location);
	casa::Table antennaTable = ms.antenna();
	size_t count = antennaTable.nrow();
	return count;
}

struct AntennaInfo MeasurementSet::GetAntennaInfo(unsigned antennaId)
{
	casa::MeasurementSet ms(_location);
	casa::Table antennaTable = ms.antenna();
	unsigned count = antennaTable.nrow();
	if(antennaId >= count) {
		throw;
	}
	casa::ROArrayColumn<double> positionCol(antennaTable, "POSITION"); 
	casa::ROScalarColumn<casa::String> nameCol(antennaTable, "NAME");
	casa::ROScalarColumn<double> diameterCol(antennaTable, "DISH_DIAMETER");
	casa::ROScalarColumn<casa::String> mountCol(antennaTable, "MOUNT");
	casa::ROScalarColumn<casa::String> stationCol(antennaTable, "STATION");

	ROArrayColumnIterator<double> p = ROArrayColumnIterator<double>::First(positionCol);
	ScalarColumnIterator<casa::String> n = ScalarColumnIterator<casa::String>::First(nameCol);
	ScalarColumnIterator<double> d = ScalarColumnIterator<double>::First(diameterCol);
	ScalarColumnIterator<casa::String> m = ScalarColumnIterator<casa::String>::First(mountCol);
	ScalarColumnIterator<casa::String> s = ScalarColumnIterator<casa::String>::First(stationCol);
	unsigned index = 0;
	while(index != antennaId)
	{
		++index; ++p; ++n; ++d; ++m; ++s;
	}
	AntennaInfo info;
	info.diameter = *d;
	info.id = antennaId;
	info.name = *n;
	casa::Array<double> position = *p;
	casa::Array<double>::iterator i = position.begin();
	info.position.x = *i;
	++i;
	info.position.y = *i;
	++i;
	info.position.z = *i;
	info.mount = *m;
	info.station = *s;

	return info;
}

BandInfo MeasurementSet::GetBandInfo(const std::string &filename, unsigned bandIndex)
{
	BandInfo band;
	casa::MeasurementSet ms(filename);
	casa::Table spectralWindowTable = ms.spectralWindow();
	casa::ROScalarColumn<int> numChanCol(spectralWindowTable, "NUM_CHAN");
	casa::ROArrayColumn<double> frequencyCol(spectralWindowTable, "CHAN_FREQ");

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

	return band;
}

size_t MeasurementSet::FieldCount()
{
	casa::MeasurementSet ms(_location);
	casa::Table fieldTable = ms.field();
	size_t fieldCount = fieldTable.nrow();
	return fieldCount;
}

struct FieldInfo MeasurementSet::GetFieldInfo(unsigned fieldIndex)
{
	casa::MeasurementSet ms(_location);
	casa::Table fieldTable = ms.field();
	casa::ROArrayColumn<double> delayDirectionCol(fieldTable, "DELAY_DIR");
	const casa::Array<double> &delayDirection = delayDirectionCol(fieldIndex);
	casa::Array<double>::const_iterator delayDirectionIterator = delayDirection.begin();

	FieldInfo field;
	field.delayDirectionRA = *delayDirectionIterator;
	++delayDirectionIterator;
	field.delayDirectionDec = *delayDirectionIterator;
	field.delayDirectionDecNegCos = cosn(-field.delayDirectionDec);
	field.delayDirectionDecNegSin = sinn(-field.delayDirectionDec);

	return field;
}

MSIterator::MSIterator(class MeasurementSet &ms, bool hasCorrectedData) : _row(0)
{
	_table = ms.OpenTable(false);
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

void MeasurementSet::InitCacheData()
{
	if(!_cacheInitialized)
	{
		AOLogger::Debug << "Initializing ms cache data...\n"; 
		std::set<double>::iterator obsTimePos = _observationTimes.end();
		MSIterator iterator(*this, false);
		size_t antenna1=0xFFFFFFFF, antenna2 = 0xFFFFFFFF;
		double time = nan("");
		std::set<std::pair<size_t, size_t> > baselineSet;
		for(size_t row=0;row<iterator.TotalRows();++row)
		{
			size_t cur_a1 = iterator.Antenna1();
			size_t cur_a2 = iterator.Antenna2();
			double cur_time = iterator.Time();
			if(cur_a1 != antenna1 || cur_a2 != antenna2)
			{
				baselineSet.insert(std::pair<size_t,size_t>(cur_a1, cur_a2));
				antenna1 = cur_a1;
				antenna2 = cur_a2;
			}
			if(cur_time != time)
			{
				obsTimePos = _observationTimes.insert(obsTimePos, cur_time);
				time = cur_time;
			}
			++iterator;
		}
		for(std::set<std::pair<size_t, size_t> >::const_iterator i=baselineSet.begin(); i!=baselineSet.end(); ++i)
			_baselines.push_back(*i);
	}
	
	_cacheInitialized = true;
}

size_t MeasurementSet::GetPolarizationCount()
{
	return GetPolarizationCount(Location());
}

size_t MeasurementSet::GetPolarizationCount(const std::string &filename)
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
	casa::MeasurementSet ms(_location);
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
	casa::MeasurementSet ms(_location);
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
	casa::MeasurementSet ms(_location);
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
	casa::MeasurementSet ms(_location);
	casa::Table antennaTable(ms.antenna());
	if(antennaTable.nrow() == 0)
		throw std::runtime_error("GetStationName() : no rows in Antenna table");
	casa::ROScalarColumn<casa::String> stationColumn(antennaTable, "STATION");
	return stationColumn(0);
}

bool MeasurementSet::ChannelZeroIsRubish()
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
