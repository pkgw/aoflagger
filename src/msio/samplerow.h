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
#ifndef SAMPLEROW_H
#define SAMPLEROW_H

#include <limits>
#include <cmath>

#include <boost/shared_ptr.hpp>

#include "image2d.h"
#include "mask2d.h"

typedef boost::shared_ptr<class SampleRow> SampleRowPtr;
typedef boost::shared_ptr<const class SampleRow> SampleRowCPtr;

#include "../strategy/algorithms/convolutions.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class SampleRow {
	public:
		~SampleRow()
		{
			delete[] _values;
		}

		static SampleRowPtr CreateEmpty(size_t size) {
			return SampleRowPtr(new SampleRow(size));
		}
		static SampleRowPtr CreateZero(size_t size) {
			SampleRow *row = new SampleRow(size);
			for(size_t i=0;i<size;++i)
				row->_values[i] = 0.0;
			return SampleRowPtr(row);
		}
		static SampleRowPtr CreateFromRow(Image2DCPtr image, size_t y)
		{
			SampleRow *row = new SampleRow(image->Width());
			for(size_t x=0;x<image->Width();++x)
				row->_values[x] = image->Value(x, y);
			return SampleRowPtr(row);
		}
		static SampleRowPtr CreateFromRow(Image2DCPtr image, size_t xStart, size_t length, size_t y)
		{
			SampleRow *row = new SampleRow(length);
			for(size_t x=0;x<length;++x)
				row->_values[x] = image->Value(x+xStart, y);
			return SampleRowPtr(row);
		}
		static SampleRowPtr CreateFromRowWithMissings(Image2DCPtr image, Mask2DCPtr mask, size_t y)
		{
			SampleRow *row = new SampleRow(image->Width());
			for(size_t x=0;x<image->Width();++x)
			{
				if(mask->Value(x, y))
					row->_values[x] = std::numeric_limits<num_t>::quiet_NaN();
				else
					row->_values[x] = image->Value(x, y);
			}
			return SampleRowPtr(row);
		}
		static SampleRowPtr CreateAmplitudeFromRow(Image2DCPtr real, Image2DCPtr imaginary, size_t y)
		{
			SampleRow *row = new SampleRow(real->Width());
			for(size_t x=0;x<real->Width();++x)
			{
				row->_values[x] = sqrtn(real->Value(x,y)*real->Value(x,y) + imaginary->Value(x,y)*imaginary->Value(x,y));
			}
			return SampleRowPtr(row);
		}
		static SampleRowPtr CreateFromColumn(Image2DCPtr image, size_t x)
		{
			SampleRow *row = new SampleRow(image->Height());
			for(size_t y=0;y<image->Height();++y)
				row->_values[y] = image->Value(x, y);
			return SampleRowPtr(row);
		}
		static SampleRowPtr CreateFromColumnWithMissings(Image2DCPtr image, Mask2DCPtr mask, size_t x)
		{
			SampleRow *row = new SampleRow(image->Height());
			for(size_t y=0;y<image->Height();++y)
			{
				if(mask->Value(x, y))
					row->_values[y] = std::numeric_limits<num_t>::quiet_NaN();
				else
					row->_values[y] = image->Value(x, y);
			}
			return SampleRowPtr(row);
		}
		static SampleRowPtr CreateFromRowSum(Image2DCPtr image, size_t yStart, size_t yEnd)
		{
			if(yEnd > yStart) {
				SampleRowPtr row = CreateFromRow(image, yStart);
	
				for(size_t y=yStart+1;y<yEnd;++y) {
					for(size_t x=0;x<image->Width();++x)
						row->_values[x] += image->Value(x, y);
				}
				return row;
			} else {
				return CreateZero(image->Width());
			}
		}
		static SampleRowPtr CreateFromColumnSum(Image2DCPtr image, size_t xStart, size_t xEnd)
		{
			if(xEnd > xStart) {
				SampleRowPtr row = CreateFromColumn(image, xStart);
	
				for(size_t x=xStart+1;x<xEnd;++x) {
					for(size_t y=0;y<image->Width();++y)
						row->_values[y] += image->Value(x, y);
				}
				return row;
			} else {
				return CreateZero(image->Width());
			}
		}
		static SampleRowPtr CreateCopy(SampleRowCPtr source)
		{
			return SampleRowPtr(new SampleRow(*source));
		}
		SampleRowPtr Copy() const
		{
			return SampleRowPtr(new SampleRow(*this));
		}
		
		void SetHorizontalImageValues(Image2DPtr image, unsigned y) const
		{
			for(size_t i=0;i<_size;++i)
			{
				image->SetValue(i, y, _values[i]);
			}
		}
		void SetHorizontalImageValues(Image2DPtr image, unsigned xStart, unsigned y) const
		{
			for(size_t i=0;i<_size;++i)
			{
				image->SetValue(i+xStart, y, _values[i]);
			}
		}
		void SetVerticalImageValues(Image2DPtr image, unsigned x) const
		{
			for(size_t i=0;i<_size;++i)
			{
				image->SetValue(x, i, _values[i]);
			}
		}
		void SetVerticalImageValues(Image2DPtr image, unsigned x, unsigned yStart) const
		{
			for(size_t i=0;i<_size;++i)
			{
				image->SetValue(x, i+yStart, _values[i]);
			}
		}
		
		num_t Value(size_t i) const { return _values[i]; }
		void SetValue(size_t i, num_t newValue) { _values[i] = newValue; }

		size_t Size() const { return _size; }
		
		size_t IndexOfMax() const
		{
			size_t maxIndex = 0;
			num_t maxValue = _values[0];
			for(size_t i = 1; i<_size;++i)
			{
				if(_values[i] > maxValue)
				{
					maxIndex = i;
					maxValue = _values[i];
				}
			}
			return maxIndex;
		}

		numl_t RMS() const
		{
			if(_size == 0) return std::numeric_limits<numl_t>::quiet_NaN();
			numl_t sum = 0.0;
			for(size_t i=0;i<_size;++i)
				sum += _values[i] * _values[i];
			return sqrtnl(sum / _size);
		}
		num_t Median() const
		{
			if(_size == 0) return std::numeric_limits<num_t>::quiet_NaN();

			num_t *copy = new num_t[_size];
			for(size_t i=0;i<_size;++i)
				copy[i] = _values[i];
			if(_size % 2 == 0)
			{
				size_t
					m = _size / 2 - 1;
				std::nth_element(copy, copy + m, copy + _size);
				num_t leftMid = *(copy + m);
				std::nth_element(copy, copy + m + 1, copy + _size);
				num_t rightMid = *(copy + m + 1);
				delete[] copy;
				return (leftMid + rightMid) / 2;
			} else {
				size_t
					m = _size / 2;
				std::nth_element(copy, copy + m, copy + _size);
				num_t mid = *(copy + m);
				delete[] copy;
				return mid;
			}
		}
		numl_t Mean() const
		{
			numl_t mean = 0.0;
			for(size_t i = 0; i<_size;++i)
				mean += _values[i];
			return mean / _size;
		}
		numl_t MeanWithMissings() const
		{
			numl_t mean = 0.0;
			size_t count = 0;
			for(size_t i = 0; i<_size;++i)
			{
				if(std::isfinite(_values[i]))
				{
					mean += _values[i];
					++count;
				}
			}
			return mean / count;
		}
		numl_t StdDev(double mean) const
		{
			numl_t stddev = 0.0;
			for(size_t i = 0; i<_size;++i)
				stddev += (_values[i] - mean) * (_values[i] - mean);
			return sqrtnl(stddev / _size);
		}
		num_t RMSWithMissings() const
		{
			SampleRowPtr row = CreateWithoutMissings();
			return row->RMS();
		}
		num_t MedianWithMissings() const
		{
			SampleRowPtr row = CreateWithoutMissings();
			return row->Median();
		}
		num_t StdDevWithMissings(double mean) const
		{
			SampleRowPtr row = CreateWithoutMissings();
			return row->StdDev(mean);
		}
		SampleRowPtr CreateWithoutMissings() const;
		bool ValueIsMissing(size_t i) const
		{
			return !std::isfinite(Value(i));
		}
		void SetValueMissing(size_t i)
		{
			SetValue(i, std::numeric_limits<num_t>::quiet_NaN());
		}
		void ConvolveWithGaussian(num_t sigma)
		{
			Convolutions::OneDimensionalGausConvolution(_values, _size, sigma);
		}
		void ConvolveWithSinc(num_t frequency)
		{
			Convolutions::OneDimensionalSincConvolution(_values, _size, frequency);
		}
		void Subtract(SampleRowCPtr source)
		{
			for(unsigned i=0;i<_size;++i)
				_values[i] -= source->_values[i];
		}
		num_t WinsorizedMean() const
		{
			num_t *data = new num_t[_size];
			memcpy(data, _values, sizeof(num_t) * _size);
			std::sort(data, data + _size, numLessThanOperator);
			size_t lowIndex = (size_t) floor(0.1 * _size);
			size_t highIndex = (size_t) ceil(0.9 * _size)-1;
			num_t lowValue = data[lowIndex];
			num_t highValue = data[highIndex];
			delete[] data;

			// Calculate mean
			num_t mean = 0.0;
			for(size_t x = 0;x < _size; ++x) {
				const num_t value = data[x];
				if(value < lowValue)
					mean += lowValue;
				else if(value > highValue)
					mean += highValue;
				else
					mean += value;
			}
			return mean / (num_t) _size;
		}
		num_t WinsorizedMeanWithMissings() const
		{
			SampleRowPtr row = CreateWithoutMissings();
			return row->WinsorizedMean();
		}
	private:
		explicit SampleRow(size_t size) :
			_size(size), _values(new num_t[_size])
		{
		}
		SampleRow(const SampleRow &source) :
			_size(source._size), _values(new num_t[_size])
		{
			for(unsigned i=0;i<_size;++i)
				_values[i] = source._values[i];
		}
		SampleRow &operator=(const SampleRow &)
		{
			return *static_cast<SampleRow*>(0);
		}
		size_t _size;
		num_t *_values;
		
		// We need this less than operator, because the normal operator
		// does not enforce a strictly ordered set, because a<b != !(b<a) in the case
		// of nans/infs.
		static bool numLessThanOperator(const num_t &a, const num_t &b) {
			if(std::isfinite(a)) {
				if(std::isfinite(b))
					return a < b;
				else
					return true;
			}
			return false;
		}
};

#endif
