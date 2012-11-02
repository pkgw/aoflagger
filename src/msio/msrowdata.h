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

#ifndef MS_ROW_DATA_H
#define MS_ROW_DATA_H

#include <string.h>

#include <ms/MeasurementSets/MSColumns.h>

#include "types.h"

#include "../util/serializable.h"

/**
 * Encapsulates the information that is contained in the Data column in one row
 * of a measurement set.
 * If some of the meta data needs to be stored as well, use the MSRowDataExt class.
 * @see MSRowDataExt
 */
class MSRowData : public Serializable
{
	public:
		MSRowData()
		: _polarizationCount(0), _channelCount(0), _realData(0), _imagData(0)
		{
		}
		
		MSRowData(unsigned polarizationCount, unsigned channelCount)
		: _polarizationCount(polarizationCount), _channelCount(channelCount)
		{
			size_t size = polarizationCount * channelCount;
			_realData = new num_t[size*2];
			_imagData = &_realData[size];
		}
		
		/**
		 * Copy construct.
		 */
		MSRowData(const MSRowData &source) :
			_polarizationCount(source._polarizationCount),
			_channelCount(source._channelCount)
		{
			size_t size = _polarizationCount * _channelCount;
			_realData = new num_t[size*2];
			_imagData = &_realData[size];
			memcpy(_realData, source._realData, size*2*sizeof(num_t));
		}
		
		~MSRowData()
		{
			delete[] _realData;
		}
		
		/**
		 * Assignment.
		 */
		MSRowData &operator=(const MSRowData &source)
		{
			size_t size = source._polarizationCount * source._channelCount;
			if(size != _polarizationCount * _channelCount)
			{
				delete[] _realData;
				_realData = new num_t[size*2];
				_imagData = &_realData[size];
			}
			_polarizationCount = source._polarizationCount;
			_channelCount = source._channelCount;
			memcpy(_realData, source._realData, size*2*sizeof(num_t));
			return *this;
		}
		
		virtual void Serialize(std::ostream &stream) const
		{
			SerializeToUInt32(stream, _polarizationCount);
			SerializeToUInt32(stream, _channelCount);
			size_t count = _polarizationCount * _channelCount * 2;
			for(size_t i=0 ; i<count ; ++i)
				SerializeToFloat(stream, _realData[i]);
		}
		
		virtual void Unserialize(std::istream &stream)
		{
			const size_t oldSize = _polarizationCount * _channelCount;
			_polarizationCount = UnserializeUInt32(stream);
			_channelCount = UnserializeUInt32(stream);
			const size_t size = _polarizationCount * _channelCount;
			if(oldSize != size)
			{
					delete[] _realData;
					_realData = new num_t[size*2];
					_imagData = &_realData[size];
			}
			for(size_t i=0 ; i<size * 2; ++i)
				_realData[i] = UnserializeFloat(stream);
		}
		unsigned PolarizationCount() const { return _polarizationCount; }
		unsigned ChannelCount() const { return _channelCount; }
		const num_t *RealPtr() const { return _realData; }
		const num_t *ImagPtr() const { return _imagData; }
		num_t *RealPtr() { return _realData; }
		num_t *ImagPtr() { return _imagData; }
		const num_t *RealPtr(size_t channel) const { return &_realData[_polarizationCount * channel]; }
		const num_t *ImagPtr(size_t channel) const { return &_imagData[_polarizationCount * channel]; }
		num_t *RealPtr(size_t channel) { return &_realData[_polarizationCount * channel]; }
		num_t *ImagPtr(size_t channel) { return &_imagData[_polarizationCount * channel]; }
	private:
		unsigned _polarizationCount, _channelCount;
		num_t *_realData, *_imagData;
};

#endif
