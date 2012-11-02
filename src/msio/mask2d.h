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
#ifndef MASK2D_H
#define MASK2D_H

#include <string.h>

#include <boost/shared_ptr.hpp>

#include "image2d.h"

typedef boost::shared_ptr<class Mask2D> Mask2DPtr;
typedef boost::shared_ptr<const class Mask2D> Mask2DCPtr;

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Mask2D {
	public:
		~Mask2D();

		// This method assumes equal height and width.
		void operator=(Mask2DCPtr source)
		{
			memcpy(_valuesConsecutive, source->_valuesConsecutive, _stride * _height * sizeof(bool));
		}

		// This method assumes equal height and width.
		void operator=(const Mask2D &source)
		{
			memcpy(_valuesConsecutive, source._valuesConsecutive, _stride * _height * sizeof(bool));
		}
		
		/**
		 * Swaps the contents of the two masks. This can be used as a move assignment operator, as it
		 * only swaps pointers; hence it is fast.
		 */
		void Swap(Mask2D &source)
		{
			std::swap(source._width, _width);
			std::swap(source._stride, _stride);
			std::swap(source._height, _height);
			std::swap(source._values, _values);
			std::swap(source._valuesConsecutive, _valuesConsecutive);
		}

		/**
		 * Swaps the contents of the two masks. This can be used as a move assignment operator, as it
		 * only swaps pointers; hence it is fast.
		 */
		void Swap(Mask2DPtr source)
		{
			Swap(*source);
		}
		
		static Mask2D *CreateUnsetMask(size_t width, size_t height)
		{
			return new Mask2D(width, height);
		}
		static Mask2DPtr CreateUnsetMaskPtr(size_t width, size_t height)
		{
			return Mask2DPtr(new Mask2D(width, height));
		}

		static Mask2D *CreateUnsetMask(const class Image2D &templateImage);
		static Mask2DPtr CreateUnsetMask(Image2DCPtr templateImage)
		{
			return Mask2DPtr(CreateUnsetMask(*templateImage));
		}

		template<bool InitValue>
		static Mask2D *CreateSetMask(const class Image2D &templateImage);

		template<bool InitValue>
		static Mask2DPtr CreateSetMask(Image2DCPtr templateImage)
		{
			return Mask2DPtr(CreateSetMask<InitValue>(*templateImage));
		}

		template<bool InitValue>
		static Mask2D *CreateSetMask(size_t width, size_t height)
		{
			Mask2D *newMask = new Mask2D(width, height);
			memset(newMask->_valuesConsecutive, InitValue, newMask->_stride * height * sizeof(bool));
			return newMask;
		}

		template<bool InitValue>
		static Mask2DPtr CreateSetMaskPtr(size_t width, size_t height)
		{
			return Mask2DPtr(CreateSetMask<InitValue>(width, height));
		}

		static Mask2D *CreateCopy(const Mask2D &source);
		static Mask2DPtr CreateCopy(Mask2DCPtr source)
		{
			return Mask2DPtr(CreateCopy(*source));
		}

		inline bool Value(size_t x, size_t y) const
		{
			return _values[y][x];
		}
		
		inline void SetValue(size_t x, size_t y, bool newValue)
		{
			_values[y][x] = newValue;
		}
		
		inline void SetHorizontalValues(size_t x, size_t y, bool newValue, size_t count)
		{
			memset(&_values[y][x], newValue, count * sizeof(bool));
		}
		
		inline size_t Width() const { return _width; }
		
		inline size_t Height() const { return _height; }

		bool AllFalse() const
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
				{
					if(_values[y][x])
						return false;
				}
			}
			return true;
		}

		/**
		 * Returns a pointer to one row of data. This can be used to step
		 * quickly over the data in x direction. Note that the next row
		 * is not exactly at "one times width", because the number of
		 * samples in a row is made divisable by four. This makes it
		 * possible to execute SSE instruction easily.
		 * 
		 * If you want to skip over a whole row, use the Stride() method
		 * to determine the intrinsicly used width of one row.
		 * 
		 * @see Stride()
		 */
		inline bool *ValuePtr(size_t x, size_t y)
		{
			return &_values[y][x];
		}
		
		/**
		 * Returns a constant pointer to one row of data. This can be used to
		 * step quickly over the data in x direction. Note that the next row
		 * is not exactly at "one times width", because the number of
		 * samples in a row is made divisable by four. This makes it
		 * possible to execute SSE instruction easily.
		 * 
		 * If you want to skip over a whole row, use the Stride() method
		 * to determine the intrinsicly used width of one row.
		 * 
		 * @see Stride()
		 */
		inline const bool *ValuePtr(size_t x, size_t y) const
		{
			return &_values[y][x];
		}
		
		/**
		 * This value specifies the intrinsic width of one row. It is
		 * normally the first number that is >= Width() and divisable by
		 * four. When using the ValuePtr(unsigned, unsigned) method,
		 * this value can be used to step over one row.
		 * 
		 * @see ValuePtr(unsigned, unsigned)
		 */
		inline size_t Stride() const
		{
			return _stride;
		}
		
		template<bool NewValue>
		void SetAll()
		{
			memset(_valuesConsecutive, NewValue, _stride  * _height * sizeof(bool));
		}

		template<bool NewValue>
		void SetAllVertically(size_t x)
		{
			for(size_t y=0;y<_height;++y)
				_values[y][x] = NewValue;
		}

		template<bool NewValue>
		void SetAllVertically(size_t startX, size_t endX)
		{
			for(size_t x=startX;x<endX;++x)
			{
				for(size_t y=0;y<_height;++y)
					_values[y][x] = NewValue;
			}
		}

		template<bool NewValue>
		void SetAllHorizontally(size_t y)
		{
			memset(_values[y], NewValue, _width * sizeof(bool));
		}

		template<bool BoolValue>
		void SetAllHorizontally(size_t startY, size_t endY)
		{
			memset(_values[startY], BoolValue, _width * sizeof(bool) * (endY - startY));
		}

		void Invert()
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					_values[y][x] = !_values[y][x];
			}
		}
		
		/**
		 * Flips the image round the diagonal, i.e., x becomes y and y becomes x.
		 */
		Mask2DPtr CreateXYFlipped() const
		{
			Mask2D *mask = new Mask2D(_height, _width);
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					mask->_values[x][y] = _values[y][x];
			}
			return Mask2DPtr(mask);
		}

		template<bool BoolValue>
		size_t GetCount() const
		{
			size_t count = 0;
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					if(BoolValue == _values[y][x])
						++count;
			}
			return count;
		}
		
		bool Equals(Mask2DCPtr other) const
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					if(_values[y][x] != other->_values[y][x])
						return false;
			}
			return true;
		}

		Mask2DPtr ShrinkHorizontally(int factor) const;
		Mask2DPtr ShrinkHorizontallyForAveraging(int factor) const;
		
		Mask2DPtr ShrinkVertically(int factor) const;

		void EnlargeHorizontallyAndSet(Mask2DCPtr smallMask, int factor);
		void EnlargeVerticallyAndSet(Mask2DCPtr smallMask, int factor);

		void Join(Mask2DCPtr other)
		{
			for(size_t y=0;y<_height;++y) {
				for(size_t x=0;x<_width;++x)
					SetValue(x, y, other->Value(x, y) || Value(x, y));
			}
		}
		
		void Intersect(Mask2DCPtr other)
		{
			for(size_t y=0;y<_height;++y) {
				for(size_t x=0;x<_width;++x)
					SetValue(x, y, other->Value(x, y) && Value(x, y));
			}
		}
		
		Mask2DPtr Trim(size_t startX, size_t startY, size_t endX, size_t endY) const
		{
			size_t
				width = endX - startX,
				height = endY - startY;
			Mask2D *mask = new Mask2D(width, height);
			for(size_t y=startY;y<endY;++y)
			{
				for(size_t x=startX;x<endX;++x)
					mask->SetValue(x-startX, y-startY, Value(x, y));
			}
			return Mask2DPtr(mask);
		}
		
		void CopyFrom(Mask2DCPtr source, size_t destX, size_t destY)
		{
			size_t
				x2 = source->_width + destX,
				y2 = source->_height + destY;
			if(x2 > _width) x2 = _width;
			if(y2 > _height) y2 = _height;
			for(size_t y=destY;y<y2;++y)
			{
				for(size_t x=destX;x<x2;++x)
					SetValue(x, y, source->Value(x-destX, y-destY));
			}
		}
		
		void SwapXY()
		{
			Mask2D *tempMask = new Mask2D(_height, _width);
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
				{
					tempMask->SetValue(y, x, Value(x, y));
				}
			}
			Swap(*tempMask);
			delete tempMask;
		}
	private:
		Mask2D(size_t width, size_t height);

		size_t _width, _height;
		size_t _stride;
		
		bool **_values;
		bool *_valuesConsecutive;
};

#endif
