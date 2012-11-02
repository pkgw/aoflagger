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
#include "mask2d.h"
#include "image2d.h"

#include <iostream>

Mask2D::Mask2D(size_t width, size_t height) :
	_width(width),
	_height(height),
	_stride((((width-1)/4)+1)*4)
{
	if(_width == 0) _stride=0;
	unsigned allocHeight = ((((height-1)/4)+1)*4);
	if(height == 0) allocHeight = 0;
	_valuesConsecutive = new bool[_stride * allocHeight * sizeof(bool)];
	
	_values = new bool*[allocHeight];
	for(size_t y=0;y<height;++y)
	{
		_values[y] = &_valuesConsecutive[_stride * y];
		// Even though the values after the requested width are never relevant, we will
		// initialize them to true to prevent valgrind to report unset values when they
		// are used in SSE instructions.
		for(size_t x=_width;x<_stride;++x)
		{
			_values[y][x] = true;
		}
	}
	for(size_t y=height;y<allocHeight;++y)
	{
		_values[y] = &_valuesConsecutive[_stride * y];
		// (see remark above about initializing to true)
		for(size_t x=0;x<_stride;++x)
		{
			_values[y][x] = true;
		}
	}
}

Mask2D::~Mask2D()
{
	delete[] _values;
	delete[] _valuesConsecutive;
}

Mask2D *Mask2D::CreateUnsetMask(const Image2D &templateImage)
{
	return new Mask2D(templateImage.Width(), templateImage.Height());
}

template <bool InitValue>
Mask2D *Mask2D::CreateSetMask(const class Image2D &templateImage)
{
	size_t
		width = templateImage.Width(),
		height = templateImage.Height();

	Mask2D *newMask = new Mask2D(width, height);
	memset(newMask->_valuesConsecutive, InitValue, newMask->_stride * height * sizeof(bool));
	return newMask;
}

template Mask2D *Mask2D::CreateSetMask<false>(const class Image2D &templateImage);
template Mask2D *Mask2D::CreateSetMask<true>(const class Image2D &templateImage);

Mask2D *Mask2D::CreateCopy(const Mask2D &source)
{
	size_t
		width = source.Width(),
		height = source.Height();

	Mask2D *newMask = new Mask2D(width, height);
	memcpy(newMask->_valuesConsecutive, source._valuesConsecutive, source._stride * height * sizeof(bool));
	return newMask;
}

Mask2DPtr Mask2D::ShrinkHorizontally(int factor) const
{
	size_t newWidth = (_width + factor - 1) / factor;

	Mask2D *newMask= new Mask2D(newWidth, _height);

	for(size_t x=0;x<newWidth;++x)
	{
		size_t binSize = factor;
		if(binSize + x*factor > _width)
			binSize = _width - x*factor;

		for(size_t y=0;y<_height;++y)
		{
			bool value = false;
			for(size_t binX=0;binX<binSize;++binX)
			{
				size_t curX = x*factor + binX;
				value = value | Value(curX, y);
			}
			newMask->SetValue(x, y, value);
		}
	}
	return Mask2DPtr(newMask);
}

Mask2DPtr Mask2D::ShrinkHorizontallyForAveraging(int factor) const
{
	size_t newWidth = (_width + factor - 1) / factor;

	Mask2D *newMask= new Mask2D(newWidth, _height);

	for(size_t x=0;x<newWidth;++x)
	{
		size_t binSize = factor;
		if(binSize + x*factor > _width)
			binSize = _width - x*factor;

		for(size_t y=0;y<_height;++y)
		{
			bool value = true;
			for(size_t binX=0;binX<binSize;++binX)
			{
				size_t curX = x*factor + binX;
				value = value & Value(curX, y);
			}
			newMask->SetValue(x, y, value);
		}
	}
	return Mask2DPtr(newMask);
}

Mask2DPtr Mask2D::ShrinkVertically(int factor) const
{
	size_t newHeight = (_height + factor - 1) / factor;

	Mask2D *newMask= new Mask2D(_width, newHeight);

	for(size_t y=0;y<newHeight;++y)
	{
		size_t binSize = factor;
		if(binSize + y*factor > _height)
			binSize = _height - y*factor;

		for(size_t x=0;x<_width;++x)
		{
			bool value = false;
			for(size_t binY=0;binY<binSize;++binY)
			{
				size_t curY = y*factor + binY;
				value = value | Value(x, curY);
			}
			newMask->SetValue(x, y, value);
		}
	}
	return Mask2DPtr(newMask);
}

void Mask2D::EnlargeHorizontallyAndSet(Mask2DCPtr smallMask, int factor)
{
	for(size_t x=0;x<smallMask->Width();++x)
	{
		size_t binSize = factor;
		if(binSize + x*factor > _width)
			binSize = _width - x*factor;

		for(size_t y=0;y<_height;++y)
		{
			for(size_t binX=0;binX<binSize;++binX)
			{
				size_t curX = x*factor + binX;
				SetValue(curX, y, smallMask->Value(x, y));
			}
		}
	}
}

void Mask2D::EnlargeVerticallyAndSet(Mask2DCPtr smallMask, int factor)
{
	for(size_t y=0;y<smallMask->Height();++y)
	{
		size_t binSize = factor;
		if(binSize + y*factor > _height)
			binSize = _height - y*factor;

		for(size_t x=0;x<_width;++x)
		{
			for(size_t binY=0;binY<binSize;++binY)
			{
				size_t curY = y*factor + binY;
				SetValue(x, curY, smallMask->Value(x, y));
			}
		}
	}
}
