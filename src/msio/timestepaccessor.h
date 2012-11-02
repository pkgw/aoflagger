/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa   *
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

#ifndef MSIO_TIMESTEP_ACCESSOR_H
#define MSIO_TIMESTEP_ACCESSOR_H

#include <stdexcept>
#include <vector>

#include <ms/MeasurementSets/MSTable.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <tables/Tables/TableIter.h>

#include "types.h"

class TimestepAccessorException : public std::runtime_error
{
	public: TimestepAccessorException(const std::string &str) : std::runtime_error(str) { }
};

class TimestepAccessor
{
	public:
		class TimestepIndex
		{
			public:
				TimestepIndex()
				{
				}
				~TimestepIndex()
				{
				}
				unsigned long row;
			private:
				TimestepIndex(TimestepIndex &) { }
				void operator=(TimestepIndex &) { }
		};
		struct TimestepData
		{
			num_t **realData, **imagData;
			unsigned antenna1, antenna2;
			double u,v;
			double timestep;

			void Allocate(unsigned polarizationCount, unsigned channelCount)
			{
				realData = new num_t*[polarizationCount];
				imagData = new num_t*[polarizationCount];
				for(unsigned p=0;p<polarizationCount;++p)
				{
					realData[p] = new num_t[channelCount];
					imagData[p] = new num_t[channelCount];
				}
			}
			void Free(unsigned polarizationCount)
			{
				for(unsigned p=0;p<polarizationCount;++p)
				{
					delete[] realData[p];
					delete[] imagData[p];
				}
				delete[] realData;
				delete[] imagData;
			}
			void CopyTo(TimestepData &dest, unsigned polarizationCount, unsigned channelCount) const
			{
				for(unsigned p=0;p<polarizationCount;++p)
				{
					for(unsigned c=0;c<channelCount;++c)
					{
						dest.realData[p][c] = realData[p][c];
						dest.imagData[p][c] = imagData[p][c];
					}
				}
				dest.antenna1 = antenna1;
				dest.antenna2 = antenna2;
				dest.u = u;
				dest.v = v;
				dest.timestep = timestep;
			}
		};

		TimestepAccessor(bool performLocking=true) : _isOpen(false), _polarizationCount(0), _totalChannelCount(0), _startRow(0), _endRow(0), _performLocking(performLocking), _writeActionCount(0), _columnName("DATA")
		{
		}

		~TimestepAccessor()
		{
			if(_isOpen)
				Close();
		}

		void Open();

		void Close();

		void AddMS(const std::string &path)
		{
			assertNotOpen();
			SetInfo newInfo(_sets.size());
			newInfo.path = path;
			_sets.push_back(newInfo);
		}

		unsigned TotalChannelCount() const
		{
			assertOpen();
			return _totalChannelCount;
		}

		unsigned TotalRowCount() const
		{
			assertOpen();
			return _totalRowCount;
		}

		void SetStartRow(unsigned long startRow)
		{
			assertOpen();
			_currentRow = startRow;
			_startRow = startRow;
		}

		void SetEndRow(unsigned long endRow)
		{
			assertOpen();
			if(endRow < _totalRowCount)
				_endRow = endRow;
			else
				_endRow = _totalRowCount;
		}

		unsigned PolarizationCount() const
		{
			assertOpen();
			return _polarizationCount;
		}

		double LowestFrequency() const
		{
			assertOpen();
			return _lowestFrequency;
		}

		double HighestFrequency() const
		{
			assertOpen();
			return _highestFrequency;
		}

		unsigned TableCount() const
		{
			return _sets.size();
		}
		
		bool ReadNext(TimestepIndex &index, TimestepData &data);

		void Write(TimestepIndex &index, const TimestepData &data);
		
		unsigned long WriteActionCount() const { return _writeActionCount; }

		void SetColumnName(const std::string columnName)
		{
			assertNotOpen();
			_columnName = columnName;
		}

	private:
		struct SetInfo
		{
			SetInfo(unsigned index_) : index(index_), table(0)
			{
			}
			SetInfo(const SetInfo &source) :
				index(source.index),
				path(source.path),
				table(source.table),
				bandCount(source.bandCount),
				channelsPerBand(source.channelsPerBand),
				highestFrequency(source.highestFrequency),
				lowestFrequency(source.lowestFrequency)
			{
			}
			void operator=(const SetInfo &source)
			{
				index = source.index;
				path = source.path;
				table = source.table;
				bandCount = source.bandCount;
				channelsPerBand = source.channelsPerBand;
				highestFrequency = source.highestFrequency;
				lowestFrequency = source.lowestFrequency;
			}
			unsigned index;
			std::string path;
			casa::Table *table;
			unsigned bandCount, channelsPerBand;
			double highestFrequency, lowestFrequency;
			casa::ROScalarColumn<int> *antenna1Column, *antenna2Column;
			casa::ROScalarColumn<double> *timeColumn;
			casa::ROArrayColumn<casa::Complex> *dataColumn;
			casa::ArrayColumn<casa::Complex> *updateDataColumn;
			casa::ROArrayColumn<double> *uvwColumn;
		};
		struct BufferItem
		{
			unsigned row;
			TimestepData data;
		};

		typedef std::vector<SetInfo> SetInfoVector;

		bool _isOpen;
		unsigned _polarizationCount, _totalChannelCount;
		SetInfoVector _sets;
		double _highestFrequency, _lowestFrequency;
		unsigned long _totalRowCount, _currentRow;
		unsigned long _startRow, _endRow;
		BufferItem *_readBuffer, *_writeBuffer;
		unsigned _bufferSize;
		unsigned _inReadBuffer, _readBufferPtr;
		unsigned _inWriteBuffer;
		bool _performLocking;
		
		unsigned long _writeActionCount;
		std::string _columnName;

		void assertOpen() const
		{
			if(!_isOpen)
				throw TimestepAccessorException("Timestep accessor has not been opened yet");
		}
		void assertNotOpen() const
		{
			if(_isOpen)
				throw TimestepAccessorException("Timestep accessor has already been opened");
		}
		void lock(unsigned setIndex);
		void unlock(unsigned setIndex);
		
		bool fillReadBuffer();
		void emptyWriteBuffer();
		void openSet(SetInfo &set, bool update=false);
		void closeSet(SetInfo &set);
};

#endif
