#include "timestepaccessor.h"

#include <ms/MeasurementSets/MeasurementSet.h>

#include <sstream>

void TimestepAccessor::Open()
{
	assertNotOpen();

	_totalChannelCount = 0;
	_highestFrequency = 0.0;
	_lowestFrequency = 0.0;
	_totalRowCount = 0;
	_currentRow = 0;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;

		casa::MeasurementSet ms(set.path);

		openSet(set);

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
		
		// Set some general values
		set.bandCount = spectralWindowTable.nrow();
		set.channelsPerBand = (*set.dataColumn)(0).shape()[1];
		_totalChannelCount += set.bandCount * set.channelsPerBand;
		if(_totalRowCount == 0)
			_totalRowCount = set.table->nrow();
		else
			if(_totalRowCount != set.table->nrow())
				throw TimestepAccessorException("Sets do not have equal number of rows");

		closeSet(set);
	}
	if(_startRow < _totalRowCount)
		_currentRow = _startRow;
	else
		_currentRow = _totalRowCount;
	_endRow = _totalRowCount;
	_bufferSize = 20000;
	_readBuffer = new BufferItem[_bufferSize];
	_readBufferPtr = 0;
	_inReadBuffer = 0;
	_writeBuffer = new BufferItem[_bufferSize];
	_inWriteBuffer = 0;
	for(unsigned i=0;i<_bufferSize;++i)
	{
		_readBuffer[i].data.Allocate(_polarizationCount, _totalChannelCount);
		_writeBuffer[i].data.Allocate(_polarizationCount, _totalChannelCount);
	}
	_isOpen = true;
}

void TimestepAccessor::Close()
{
	assertOpen();
	
	emptyWriteBuffer();
	
	_isOpen = false;

	for(unsigned i=0;i<_bufferSize;++i)
	{
		_readBuffer[i].data.Free(_polarizationCount);
		_writeBuffer[i].data.Free(_polarizationCount);
	}
	delete[] _readBuffer;
	delete[] _writeBuffer;
}

bool TimestepAccessor::ReadNext(TimestepAccessor::TimestepIndex &index, TimestepAccessor::TimestepData &data)
{
	assertOpen();

	if(_readBufferPtr >= _inReadBuffer)
	{
		if(!fillReadBuffer())
			return false;
	}

	const BufferItem &item = _readBuffer[_readBufferPtr];
	index.row = item.row;
	item.data.CopyTo(data, _polarizationCount, _totalChannelCount);
	
	++_readBufferPtr;
	return true;
}

void TimestepAccessor::openSet(TimestepAccessor::SetInfo &set, bool update)
{
	lock(set.index);
	
	set.table = 0;
	do {
		try {
			if(update)
				set.table = new casa::Table(set.path, casa::Table::Update);
			else
				set.table = new casa::Table(set.path);
		} catch(std::exception &e) {
			std::cout << "WARNING: exception was thrown:\n"
				<< e.what() << '\n'
				<< "Will try again in 30 sec.\n";
			sleep(30);
			set.table = 0;
		}
	} while(set.table == 0);
	set.antenna1Column = new casa::ROScalarColumn<int>(*set.table, "ANTENNA1");
	set.antenna2Column = new casa::ROScalarColumn<int>(*set.table, "ANTENNA2");
	set.timeColumn = new casa::ROScalarColumn<double>(*set.table, "TIME");
	if(update)
	{
		set.dataColumn = 0;
		set.updateDataColumn = new casa::ArrayColumn<casa::Complex>(*set.table, _columnName);
	}
	else
	{
		set.dataColumn = new casa::ROArrayColumn<casa::Complex>(*set.table, _columnName);
		set.updateDataColumn = 0;
	}
	set.uvwColumn = new casa::ROArrayColumn<double>(*set.table, "UVW");
}

void TimestepAccessor::closeSet(TimestepAccessor::SetInfo &set)
{
	delete set.antenna1Column;
	delete set.antenna2Column;
	delete set.timeColumn;
	//bool update = set.updateDataColumn != 0;
	if(set.dataColumn != 0)
		delete set.dataColumn;
	if(set.updateDataColumn != 0)
		delete set.updateDataColumn;
	delete set.uvwColumn;
	delete set.table;

	unlock(set.index);
}

bool TimestepAccessor::fillReadBuffer()
{
	if(_currentRow >= _endRow)
		return false;
	
	for(unsigned i=0;i<_bufferSize;++i)
	{
		_readBuffer[i].data.timestep = 0.0;
	}
	
	unsigned valIndex = 0;
	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;
		_inReadBuffer = 0;

		openSet(set);
		
		while(_inReadBuffer < _bufferSize && _currentRow + _inReadBuffer < _endRow)
		{
			TimestepData &data = _readBuffer[_inReadBuffer].data;
			unsigned long row = _currentRow + _inReadBuffer;
			_readBuffer[_inReadBuffer].row = row;
			
			// Check timestep & read u,v coordinates & antenna's
			if(data.timestep == 0.0) {
				data.timestep = (*set.timeColumn)(row);
				casa::Array<double> uvwArray = (*set.uvwColumn)(row);
				casa::Array<double>::const_iterator uvwIterator = uvwArray.begin();
				data.u = *uvwIterator;
				++uvwIterator;
				data.v = *uvwIterator;
				data.antenna1 = (*set.antenna1Column)(row);
				data.antenna2 = (*set.antenna2Column)(row);
			}
			else {
				if(data.timestep != ((*set.timeColumn)(row)))
				  {
				    std::stringstream s;
				    bool onlyWarn;
				    double thisTimestep = (*set.timeColumn)(row);
				    if(fabs(data.timestep - (*set.timeColumn)(row)) < 1e-3)
				      onlyWarn = true;
				    else
				      onlyWarn = false;
				    s << "Sets do not have equal time steps; first set has " << data.timestep << " while set " << set.index << " has " << thisTimestep << " (row=" << row << ", difference=" << (data.timestep-thisTimestep) << ")";
				    if(onlyWarn)
				      std::cout << "WARNING: " << s.str() << "\nIgnoring difference because it is < 1e-3\n";
				    else
				      throw TimestepAccessorException(s.str());
					
				  }
				if(data.antenna1 != (unsigned) ((*set.antenna1Column)(row)))
					throw TimestepAccessorException("Sets do not have same antenna1 ordering");
				if(data.antenna2 != (unsigned) ((*set.antenna2Column)(row)))
					throw TimestepAccessorException("Sets do not have same antenna2 ordering");
			}

			// Copy data from tables in arrays
			casa::Array<casa::Complex> dataArray = (*set.dataColumn)(row);
			casa::Array<casa::Complex>::const_iterator dataIterator = dataArray.begin();
			unsigned currentIndex = valIndex;
			for(unsigned f=0;f<set.channelsPerBand;++f)
			{
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					data.realData[p][currentIndex] = (*dataIterator).real();
					data.imagData[p][currentIndex] = (*dataIterator).imag();
					++dataIterator;
				}
				++currentIndex;
			}
			++_inReadBuffer;
		}
		valIndex += set.channelsPerBand;

		closeSet(set);
	}
	_currentRow += _inReadBuffer;
	_readBufferPtr = 0;
	return true;
}


void TimestepAccessor::Write(TimestepAccessor::TimestepIndex &index, const TimestepAccessor::TimestepData &data)
{
	assertOpen();

	if(_inWriteBuffer >= _bufferSize)
		emptyWriteBuffer();
	
	BufferItem &item = _writeBuffer[_inWriteBuffer];
	data.CopyTo(item.data, _polarizationCount, _totalChannelCount);
	item.row = index.row;
	
	++_inWriteBuffer;
	++_writeActionCount;
}

void TimestepAccessor::emptyWriteBuffer()
{
	unsigned valIndex = 0;
	
	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;

		openSet(set, true);

		for(unsigned writeBufferIndex = 0; writeBufferIndex < _inWriteBuffer; ++writeBufferIndex)
		{
			const BufferItem &item = _writeBuffer[writeBufferIndex];

			// Copy data from arrays in tables
			casa::Array<casa::Complex> dataArray = (*set.updateDataColumn)(item.row);
			casa::Array<casa::Complex>::iterator dataIterator = dataArray.begin();
			unsigned currentIndex = valIndex;
			for(unsigned f=0;f<set.channelsPerBand;++f)
			{
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					(*dataIterator).real() = item.data.realData[p][currentIndex];
					(*dataIterator).imag() = item.data.imagData[p][currentIndex];
					++dataIterator;
				}
				++currentIndex;
			}
			set.updateDataColumn->basePut(item.row, dataArray);
		}
		valIndex += set.channelsPerBand;

		closeSet(set);
	}
	_inWriteBuffer = 0;
}

void TimestepAccessor::lock(unsigned setIndex)
{
	if(_performLocking)
	{
		std::ostringstream s;
		s << "ssh node079 -C \"~/LOFAR-build/bin/aosynchronisation lock-unique \"" << setIndex << " 2> /dev/null\n";
		std::string str = s.str();
		if(system(str.c_str()) != 0)
			throw std::runtime_error("system() returned nonzero");
	}
}

void TimestepAccessor::unlock(unsigned setIndex)
{
	if(_performLocking)
	{
		std::ostringstream s;
		s << "ssh node079 -C \"~/LOFAR-build/bin/aosynchronisation release-unique \"" << setIndex << " 2> /dev/null\n";
		std::string str = s.str();
		if(system(str.c_str()) != 0)
			throw std::runtime_error("system() returned nonzero");
	}
}

