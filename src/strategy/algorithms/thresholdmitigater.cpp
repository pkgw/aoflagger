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
#include "../../msio/image2d.h"

#include "thresholdmitigater.h"
#include "thresholdtools.h"

template<size_t Length>
void ThresholdMitigater::HorizontalSumThreshold(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	if(Length <= input->Width())
	{
		size_t width = input->Width()-Length+1; 
		for(size_t y=0;y<input->Height();++y) {
			for(size_t x=0;x<width;++x) {
				num_t sum = 0.0;
				size_t count = 0;
				for(size_t i=0;i<Length;++i) {
					if(!mask->Value(x+i, y)) {
						sum += input->Value(x+i, y);
						count++;
					}
				}
				if(count>0 && fabs(sum/count) > threshold) {
					for(size_t i=0;i<Length;++i)
						mask->SetValue(x + i, y, true);
				}
			}
		}
	}
}

template<size_t Length>
void ThresholdMitigater::VerticalSumThreshold(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	if(Length <= input->Height())
	{
		size_t height = input->Height()-Length+1; 
		for(size_t y=0;y<height;++y) {
			for(size_t x=0;x<input->Width();++x) {
				num_t sum = 0.0;
				size_t count = 0;
				for(size_t i=0;i<Length;++i) {
					if(!mask->Value(x, y+i)) {
						sum += input->Value(x, y + i);
						count++;
					}
				}
				if(count>0 && fabs(sum/count) > threshold) {
					for(size_t i=0;i<Length;++i)
					mask->SetValue(x, y + i, true);
				}
			}
		}
	}
}

template<size_t Length>
void ThresholdMitigater::HorizontalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);
	const size_t width = mask->Width(), height = mask->Height();
	if(Length <= width)
	{
		for(size_t y=0;y<height;++y)
		{
			num_t sum = 0.0;
			size_t count = 0, xLeft, xRight;

			for(xRight=0;xRight<Length-1;++xRight)
			{
				if(!mask->Value(xRight, y))
				{
					sum += input->Value(xRight, y);
					count++;
				}
			}

			xLeft = 0;
			while(xRight < width)
			{
				// add the sample at the right
				if(!mask->Value(xRight, y))
				{
					sum += input->Value(xRight, y);
					++count;
				}
				// Check
				if(count>0 && fabs(sum/count) > threshold)
				{
					maskCopy->SetHorizontalValues(xLeft, y, true, Length);
				}
				// subtract the sample at the left
				if(!mask->Value(xLeft, y))
				{
					sum -= input->Value(xLeft, y);
					--count;
				}
				++xLeft;
				++xRight;
			}
		}
	}
	mask->Swap(maskCopy);
}

template<size_t Length>
void ThresholdMitigater::VerticalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);
	const size_t width = mask->Width(), height = mask->Height();
	if(Length <= height)
	{
		for(size_t x=0;x<width;++x)
		{
			num_t sum = 0.0;
			size_t count = 0, yTop, yBottom;

			for(yBottom=0;yBottom<Length-1;++yBottom)
			{
				if(!mask->Value(x, yBottom))
				{
					sum += input->Value(x, yBottom);
					++count;
				}
			}

			yTop = 0;
			while(yBottom < height)
			{
				// add the sample at the bottom
				if(!mask->Value(x, yBottom))
				{
					sum += input->Value(x, yBottom);
					++count;
				}
				// Check
				if(count>0 && fabs(sum/count) > threshold)
				{
					for(size_t i=0;i<Length;++i)
						maskCopy->SetValue(x, yTop + i, true);
				}
				// subtract the sample at the top
				if(!mask->Value(x, yTop))
				{
					sum -= input->Value(x, yTop);
					--count;
				}
				++yTop;
				++yBottom;
			}
		}
	}
	mask->Swap(maskCopy);
}

void ThresholdMitigater::HorizontalSumThresholdLargeReference(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold)
{
	switch(length)
	{
		case 1: HorizontalSumThreshold<1>(input, mask, threshold); break;
		case 2: HorizontalSumThresholdLarge<2>(input, mask, threshold); break;
		case 4: HorizontalSumThresholdLarge<4>(input, mask, threshold); break;
		case 8: HorizontalSumThresholdLarge<8>(input, mask, threshold); break;
		case 16: HorizontalSumThresholdLarge<16>(input, mask, threshold); break;
		case 32: HorizontalSumThresholdLarge<32>(input, mask, threshold); break;
		case 64: HorizontalSumThresholdLarge<64>(input, mask, threshold); break;
		case 128: HorizontalSumThresholdLarge<128>(input, mask, threshold); break;
		case 256: HorizontalSumThresholdLarge<256>(input, mask, threshold); break;
		default: throw BadUsageException("Invalid value for length");
	}	
}

void ThresholdMitigater::VerticalSumThresholdLargeReference(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold)
{
	switch(length)
	{
		case 1: VerticalSumThreshold<1>(input, mask, threshold); break;
		case 2: VerticalSumThresholdLarge<2>(input, mask, threshold); break;
		case 4: VerticalSumThresholdLarge<4>(input, mask, threshold); break;
		case 8: VerticalSumThresholdLarge<8>(input, mask, threshold); break;
		case 16: VerticalSumThresholdLarge<16>(input, mask, threshold); break;
		case 32: VerticalSumThresholdLarge<32>(input, mask, threshold); break;
		case 64: VerticalSumThresholdLarge<64>(input, mask, threshold); break;
		case 128: VerticalSumThresholdLarge<128>(input, mask, threshold); break;
		case 256: VerticalSumThresholdLarge<256>(input, mask, threshold); break;
		default: throw BadUsageException("Invalid value for length");
	}
}

void ThresholdMitigater::VerticalSumThresholdLargeSSE(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold)
{
	switch(length)
	{
		case 1: VerticalSumThreshold<1>(input, mask, threshold); break;
		case 2: VerticalSumThresholdLargeSSE<2>(input, mask, threshold); break;
		case 4: VerticalSumThresholdLargeSSE<4>(input, mask, threshold); break;
		case 8: VerticalSumThresholdLargeSSE<8>(input, mask, threshold); break;
		case 16: VerticalSumThresholdLargeSSE<16>(input, mask, threshold); break;
		case 32: VerticalSumThresholdLargeSSE<32>(input, mask, threshold); break;
		case 64: VerticalSumThresholdLargeSSE<64>(input, mask, threshold); break;
		case 128: VerticalSumThresholdLargeSSE<128>(input, mask, threshold); break;
		case 256: VerticalSumThresholdLargeSSE<256>(input, mask, threshold); break;
		default: throw BadUsageException("Invalid value for length");
	}	
}

void ThresholdMitigater::HorizontalSumThresholdLargeSSE(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold)
{
	switch(length)
	{
		case 1: HorizontalSumThreshold<1>(input, mask, threshold); break;
		case 2: HorizontalSumThresholdLargeSSE<2>(input, mask, threshold); break;
		case 4: HorizontalSumThresholdLargeSSE<4>(input, mask, threshold); break;
		case 8: HorizontalSumThresholdLargeSSE<8>(input, mask, threshold); break;
		case 16: HorizontalSumThresholdLargeSSE<16>(input, mask, threshold); break;
		case 32: HorizontalSumThresholdLargeSSE<32>(input, mask, threshold); break;
		case 64: HorizontalSumThresholdLargeSSE<64>(input, mask, threshold); break;
		case 128: HorizontalSumThresholdLargeSSE<128>(input, mask, threshold); break;
		case 256: HorizontalSumThresholdLargeSSE<256>(input, mask, threshold); break;
		default: throw BadUsageException("Invalid value for length");
	}	
}

void ThresholdMitigater::HorizontalVarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold)
{
	size_t width = input->Width()-length+1;
	for(size_t y=0;y<input->Height();++y) {
		for(size_t x=0;x<width;++x) {
			bool flag = true;
			for(size_t i=0;i<length;++i) {
				if(input->Value(x+i, y) < threshold && input->Value(x+i, y) > -threshold) {
					flag = false;
					break;
				}
			}
			if(flag) {
				for(size_t i=0;i<length;++i)
					mask->SetValue(x + i, y, true);
			}
		}
	}
}

void ThresholdMitigater::VerticalVarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold)
{
	size_t height = input->Height()-length+1; 
	for(size_t y=0;y<height;++y) {
		for(size_t x=0;x<input->Width();++x) {
			bool flag = true;
			for(size_t i=0;i<length;++i) {
				if(input->Value(x, y+i) <= threshold && input->Value(x, y+i) >= -threshold) {
					flag = false;
					break;
				}
			}
			if(flag) {
				for(size_t i=0;i<length;++i)
					mask->SetValue(x, y + i, true);
			}
		}
	}
}

void ThresholdMitigater::VarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold)
{
	HorizontalVarThreshold(input, mask, length, threshold);
	VerticalVarThreshold(input, mask, length, threshold);
}
