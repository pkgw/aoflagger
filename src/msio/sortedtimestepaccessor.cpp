#include "sortedtimestepaccessor.h"

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>

void SortedTimestepAccessor::Open()
{
	assertNotOpen();

	_totalChannelCount = 0;
	_highestFrequency = 0.0;
	_lowestFrequency = 0.0;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;

		casa::MeasurementSet ms(set.path);
		casa::Table mainTable(set.path, casa::Table::Update);

		// Create the sorted table and iterator
		casa::Block<casa::String> names(4);
		names[0] = "TIME";
		names[1] = "ANTENNA1";
		names[2] = "ANTENNA2";
		names[3] = "DATA_DESC_ID";
		casa::Table sortab = mainTable.sort(names);
		names.resize(3, true, true);
		set.tableIter = new casa::TableIterator(sortab, names, casa::TableIterator::Ascending, casa::TableIterator::NoSort);

		// Check number of polarizations
		casa::Table polTable = ms.polarization();
		casa::ROArrayColumn<int> corTypeColumn(polTable, "CORR_TYPE");
		if(_polarizationCount==0 && i==_sets.begin())
			_polarizationCount = corTypeColumn(0).shape()[0];
		else if(_polarizationCount != corTypeColumn(0).shape()[0])
			throw TimestepAccessorException("Number of polarizations don't match!");

		// Find lowest and highest frequency and check order
		set.lowestFrequency = 0.0;
		set.highestFrequency = 0.0;
		casa::Table spectralWindowTable = ms.spectralWindow();
		casa::ROArrayColumn<double> frequencyCol(spectralWindowTable, "CHAN_FREQ");
		for(unsigned b=0;b<spectralWindowTable.nrow();++b)
		{
			casa::Array<double> frequencyArray = frequencyCol(b);
			casa::Array<double>::const_iterator frequencyIterator = frequencyArray.begin();
			while(frequencyIterator != frequencyArray.end())
			{
				double frequency = *frequencyIterator;
				if(set.lowestFrequency == 0.0) set.lowestFrequency = frequency;
				if(frequency < set.lowestFrequency || frequency <= set.highestFrequency)
					throw TimestepAccessorException("Channels or bands are not ordered in increasing frequency");
				set.highestFrequency = frequency;
				++frequencyIterator;
			}
		}
		if(set.lowestFrequency < _highestFrequency)
			throw TimestepAccessorException("Sub-bands are not given in order of increasing frequency");
		if(_lowestFrequency == 0.0) _lowestFrequency = set.lowestFrequency;
		_highestFrequency = set.highestFrequency;

		set.bandCount = spectralWindowTable.nrow();
		set.channelsPerBand = casa::ROArrayColumn<casa::Complex>(mainTable, "DATA")(0).shape()[1];
		_totalChannelCount += set.bandCount * set.channelsPerBand;
	}
	_isOpen = true;
}

void SortedTimestepAccessor::Close()
{
	assertOpen();

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		delete i->tableIter;
	}
	_isOpen = false;
}

bool SortedTimestepAccessor::ReadNext(SortedTimestepAccessor::TimestepIndex &index, SortedTimestepAccessor::TimestepData &data)
{
	assertOpen();

	index.FreeTables();

	double timeStep = 0.0;
	unsigned valIndex = 0;
	casa::Table **tablePtr = index.tables;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;

		if(set.tableIter->pastEnd())
			return false;
		*tablePtr = new casa::Table(set.tableIter->table());
		casa::Table &table(**tablePtr);

		// Check timestep & read u,v coordinates & antenna's
		casa::ROScalarColumn<double> timeColumn = casa::ROScalarColumn<double>(table, "TIME");
		if(timeStep == 0.0) {
			casa::ROArrayColumn<double> uvwColumn = casa::ROArrayColumn<double>(table, "UVW");
			timeStep = timeColumn(0);
			casa::Array<double> uvwArray = uvwColumn(0);
			casa::Array<double>::const_iterator uvwIterator = uvwArray.begin();
			data.u = *uvwIterator;
			++uvwIterator;
			data.v = *uvwIterator;
			casa::ROScalarColumn<int>
				antenna1Column = casa::ROScalarColumn<int>(table, "ANTENNA1"),
				antenna2Column = casa::ROScalarColumn<int>(table, "ANTENNA2");
			data.antenna1 = antenna1Column(0);
			data.antenna2 = antenna2Column(0);
		}
		else if(timeStep != timeColumn(0))
			throw TimestepAccessorException("Sets do not have same time steps");

		// Copy data from tables in arrays
		casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
		for(unsigned band=0;band<set.bandCount;++band)
		{
			casa::Array<casa::Complex> dataArray = dataColumn(band);
			casa::Array<casa::Complex>::const_iterator dataIterator = dataArray.begin();
			for(unsigned f=0;f<set.channelsPerBand;++f)
			{
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					data.realData[p][valIndex] = (*dataIterator).real();
					data.imagData[p][valIndex] = (*dataIterator).imag();
					++dataIterator;
				}
				++valIndex;
			}
		}

		set.tableIter->next();
		++tablePtr;
	}
	return true;
}

void SortedTimestepAccessor::Write(SortedTimestepAccessor::TimestepIndex &index, const SortedTimestepAccessor::TimestepData &data)
{
	assertOpen();

	casa::Table **tablePtr = index.tables;
	unsigned valIndex = 0;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		const SetInfo &set = *i;

		casa::Table &table = **tablePtr;

		// Copy data from arrays in tables
		casa::ArrayColumn<casa::Complex> dataColumn(table, "DATA");
		for(unsigned band=0;band<set.bandCount;++band)
		{
			casa::Array<casa::Complex> dataArray = dataColumn(band);
			casa::Array<casa::Complex>::iterator dataIterator = dataArray.begin();
			for(unsigned f=0;f<set.channelsPerBand;++f)
			{
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					(*dataIterator).real() = data.realData[p][valIndex];
					(*dataIterator).imag() = data.imagData[p][valIndex];
					++dataIterator;
				}
				++valIndex;
			}
			dataColumn.basePut(band, dataArray);
		}
		set.tableIter->next();
	}
	index.FreeTables();
}
