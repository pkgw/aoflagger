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
#include "statisticalflagger.h"

StatisticalFlagger::StatisticalFlagger()
{
}


StatisticalFlagger::~StatisticalFlagger()
{
}

bool StatisticalFlagger::SquareContainsFlag(Mask2DCPtr mask, size_t xLeft, size_t yTop, size_t xRight, size_t yBottom)
{
	for(size_t y=yTop;y<=yBottom;++y)
	{
		for(size_t x=xLeft;x<=xRight;++x)
		{
			if(mask->Value(x, y))
				return true;
		}
	}
	return false;
}

void StatisticalFlagger::EnlargeFlags(Mask2DPtr mask, size_t timeSize, size_t frequencySize)
{
	Mask2DCPtr old = Mask2D::CreateCopy(mask);
	for(size_t y=0;y<mask->Height();++y)
	{
		size_t top, bottom;
		if(y > frequencySize)
			top = y - frequencySize;
		else
			top = 0;
		if(y + frequencySize < mask->Height() - 1)
			bottom = y + frequencySize;
		else
			bottom = mask->Height() - 1;
		
		for(size_t x=0;x<mask->Width();++x)
		{
			size_t left, right;
			if(x > timeSize)
				left = x - timeSize;
			else
				left = 0;
			if(x + timeSize < mask->Width() - 1)
				right = x + timeSize;
			else
				right = mask->Width() - 1;
			
			if(SquareContainsFlag(old, left, top, right, bottom))
				mask->SetValue(x, y, true);
		}
	}
}

void StatisticalFlagger::DilateFlagsHorizontally(Mask2DPtr mask, size_t timeSize)
{
	if(timeSize != 0)
	{
		Mask2DPtr destination = Mask2D::CreateUnsetMaskPtr(mask->Width(), mask->Height());
		if(timeSize > mask->Width()) timeSize = mask->Width();
		const int intSize = (int) timeSize;
		
		for(size_t y=0;y<mask->Height();++y)
		{
			int dist = intSize + 1;
			for(size_t x=0;x<timeSize;++x)
			{
				if(mask->Value(x, y))
					dist = - intSize;
				dist++;
			}
			for(size_t x=0;x<mask->Width() - timeSize;++x)
			{
				if(mask->Value(x + timeSize, y))
					dist = -intSize;
				if(dist <= intSize)
				{
					destination->SetValue(x, y, true);
					dist++;
				} else {
					destination->SetValue(x, y, false);
				}
			}
			for(size_t x=mask->Width() - timeSize;x<mask->Width();++x)
			{
				if(dist <= intSize)
				{
					destination->SetValue(x, y, true);
					dist++;
				} else {
					destination->SetValue(x, y, false);
				}
			}
		}
		mask->Swap(destination);
	}
}

void StatisticalFlagger::DilateFlagsVertically(Mask2DPtr mask, size_t frequencySize)
{
	if(frequencySize != 0)
	{
		Mask2DPtr destination = Mask2D::CreateUnsetMaskPtr(mask->Width(), mask->Height());
		if(frequencySize > mask->Height()) frequencySize = mask->Height();
		const int intSize = (int) frequencySize;
		
		for(size_t x=0;x<mask->Width();++x)
		{
			int dist = intSize + 1;
			for(size_t y=0;y<frequencySize;++y)
			{
				if(mask->Value(x, y))
					dist = - intSize;
				dist++;
			}
			for(size_t y=0;y<mask->Height() - frequencySize;++y)
			{
				if(mask->Value(x, y + frequencySize))
					dist = -intSize;
				if(dist <= intSize)
				{
					destination->SetValue(x, y, true);
					dist++;
				} else {
					destination->SetValue(x, y, false);
				}
			}
			for(size_t y=mask->Height() - frequencySize;y<mask->Height();++y)
			{
				if(dist <= intSize)
				{
					destination->SetValue(x, y, true);
					dist++;
				} else {
					destination->SetValue(x, y, false);
				}
			}
		}
		mask->Swap(destination);
	}
}

void StatisticalFlagger::LineRemover(Mask2DPtr mask, size_t maxTimeContamination, size_t maxFreqContamination)
{
	for(size_t x=0;x<mask->Width();++x)
	{
		size_t count = 0;
		for(size_t y=0;y<mask->Height();++y)
		{
			if(mask->Value(x,y))
				++count;
		}
		if(count > maxFreqContamination)
			FlagTime(mask, x);
	}

	for(size_t y=0;y<mask->Height();++y)
	{
		size_t count = 0;
		for(size_t x=0;x<mask->Width();++x)
		{
			if(mask->Value(x,y))
				++count;
		}
		if(count > maxTimeContamination)
			FlagFrequency(mask, y);
	}
}

void StatisticalFlagger::FlagTime(Mask2DPtr mask, size_t x)
{
	for(size_t y=0;y<mask->Height();++y)
	{
		mask->SetValue(x, y, true);
	}
}

void StatisticalFlagger::FlagFrequency(Mask2DPtr mask, size_t y)
{
	for(size_t x=0;x<mask->Width();++x)
	{
		mask->SetValue(x, y, true);
	}
}

void StatisticalFlagger::MaskToInts(Mask2DCPtr mask, int **maskAsInt)
{
	for(size_t y=0;y<mask->Height();++y)
	{
		int *column = maskAsInt[y];
		for(size_t x=0;x<mask->Width();++x)
		{
			column[x] = mask->Value(x, y) ? 1 : 0;
		}
	}
}

void StatisticalFlagger::SumToLeft(Mask2DCPtr mask, int **sums, size_t width, size_t step, bool reverse)
{
	if(reverse)
	{
		for(size_t y=0;y<mask->Height();++y)
		{
			int *column = sums[y];
			for(size_t x=width;x<mask->Width();++x)
			{
				if(mask->Value(x - width/2, y))
					column[x] += step;
			}
		}
	} else {
		for(size_t y=0;y<mask->Height();++y)
		{
			int *column = sums[y];
			for(size_t x=0;x<mask->Width() - width;++x)
			{
				if(mask->Value(x + width/2, y))
					column[x] += step;
			}
		}
	}
}

void StatisticalFlagger::SumToTop(Mask2DCPtr mask, int **sums, size_t width, size_t step, bool reverse)
{
	if(reverse)
	{
		for(size_t y=width;y<mask->Height();++y)
		{
			int *column = sums[y];
			for(size_t x=0;x<mask->Width();++x)
			{
				if(mask->Value(x, y - width/2))
					column[x] += step;
			}
		}
	} else {
		for(size_t y=0;y<mask->Height() - width;++y)
		{
			int *column = sums[y];
			for(size_t x=0;x<mask->Width();++x)
			{
				if(mask->Value(x, y + width/2))
					column[x] += step;
			}
		}
	}
}

void StatisticalFlagger::ThresholdTime(Mask2DCPtr mask, int **flagMarks, int **sums, int thresholdLevel, int width)
{
	int halfWidthL = (width-1) / 2;
	int halfWidthR = (width-1) / 2;
	for(size_t y=0;y<mask->Height();++y)
	{
		const int *column = sums[y];
		for(size_t x=halfWidthL;x<mask->Width() - halfWidthR;++x)
		{
			if(column[x] > thresholdLevel)
			{
				const unsigned right = x+halfWidthR+1;
				++flagMarks[y][x-halfWidthL];
				if(right < mask->Width())
					--flagMarks[y][right];
			}
		}
	}
}

void StatisticalFlagger::ThresholdFrequency(Mask2DCPtr mask, int **flagMarks, int **sums, int thresholdLevel, int width)
{
	int halfWidthT = (width-1) / 2;
	int halfWidthB = (width-1) / 2;
	for(size_t y=halfWidthT;y<mask->Height() - halfWidthB;++y)
	{
		int *column = sums[y];
		for(size_t x=0;x<mask->Width();++x)
		{
			if(column[x] > thresholdLevel)
			{
				const unsigned bottom = y+halfWidthB+1;
				++flagMarks[y-halfWidthT][x];
				if(bottom < mask->Height())
					--flagMarks[bottom][x];
			}
		}
	}
}

void StatisticalFlagger::ApplyMarksInTime(Mask2DPtr mask, int **flagMarks)
{
	for(size_t y=0;y<mask->Height();++y)
	{
		int startedCount = 0;
		for(size_t x=0;x<mask->Width();++x)
		{
			startedCount += flagMarks[y][x];
			if(startedCount > 0)
				mask->SetValue(x, y, true);
		}
	}
}

void StatisticalFlagger::ApplyMarksInFrequency(Mask2DPtr mask, int **flagMarks)
{
	for(size_t x=0;x<mask->Width();++x)
	{
		int startedCount = 0;
		for(size_t y=0;y<mask->Height();++y)
		{
			startedCount += flagMarks[y][x];
			if(startedCount > 0)
				mask->SetValue(x, y, true);
		}
	}
}

void StatisticalFlagger::DensityTimeFlagger(Mask2DPtr mask, num_t minimumGoodDataRatio)
{
	num_t width = 2.0;
	size_t iterations = 0, step = 1;
	bool reverse = false;
	
	//"sums represents the number of flags in a certain range
	int **sums = new int*[mask->Height()];
	
	// flagMarks are integers that represent the number of times an area is marked as the
	// start or end of a flagged area. For example, if flagMarks[0][0] = 0, it is not the start or
	// end of an area. If it is 1, it is the start. If it is -1, it is the end. A range of
	// [2 0 -1 -1 0] produces a flag mask [T T T T F].
	int **flagMarks = new int*[mask->Height()];
	
	for(size_t y=0;y<mask->Height();++y)
	{
		sums[y] = new int[mask->Width()];
		flagMarks[y] = new int[mask->Width()];
		for(size_t x=0;x<mask->Width();++x)
			flagMarks[y][x] = 0;
	}
	
	MaskToInts(mask, sums);
	
	while(width < mask->Width())
	{
		++iterations;
		SumToLeft(mask, sums, (size_t) width, step, reverse);
		const int maxFlagged = (int) floor((1.0-minimumGoodDataRatio)*(num_t)(width));
		ThresholdTime(mask, flagMarks, sums, maxFlagged, (size_t) width);
	
		num_t newWidth = width * 1.05;
		if((size_t) newWidth == (size_t) width)
			newWidth = width + 1.0;
		step = (size_t) (newWidth - width);
		width = newWidth;
		reverse = !reverse;
	}
	
	ApplyMarksInTime(mask, flagMarks);

	for(size_t y=0;y<mask->Height();++y)
	{
		delete[] sums[y];
		delete[] flagMarks[y];
	}
	delete[] sums;
	delete[] flagMarks;
}

void StatisticalFlagger::DensityFrequencyFlagger(Mask2DPtr mask, num_t minimumGoodDataRatio)
{
	num_t width = 2.0;
	size_t iterations = 0, step = 1;
	bool reverse = false;
	
	Mask2DPtr newMask = Mask2D::CreateCopy(mask);
	
	int **sums = new int*[mask->Height()];
	int **flagMarks = new int*[mask->Height()];
	
	for(size_t y=0;y<mask->Height();++y)
	{
		sums[y] = new int[mask->Width()];
		flagMarks[y] = new int[mask->Width()];
		for(size_t x=0;x<mask->Width();++x)
			flagMarks[y][x] = 0;
	}
	
	MaskToInts(mask, sums);
	
	while(width < mask->Height())
	{
		++iterations;
		SumToTop(mask, sums, (size_t) width, step, reverse);
		const int maxFlagged = (int) floor((1.0-minimumGoodDataRatio)*(num_t)(width));
		ThresholdFrequency(mask, flagMarks, sums, maxFlagged, (size_t) width);
	
		num_t newWidth = width * 1.05;
		if((size_t) newWidth == (size_t) width)
			newWidth = width + 1.0;
		step = (size_t) (newWidth - width);
		width = newWidth;
		reverse = !reverse;
	}

	ApplyMarksInFrequency(mask, flagMarks);

	for(size_t y=0;y<mask->Height();++y)
	{
		delete[] sums[y];
		delete[] flagMarks[y];
	}
	delete[] sums;
	delete[] flagMarks;
}

void StatisticalFlagger::ScaleInvDilationFull(bool *flags, const unsigned n, num_t minimumGoodDataRatio)
{
	num_t width = 2.0;
	bool reverse = false;
	
	int *sums = new int[n];
	int *flagMarks = new int[n];
	for(size_t x=0;x<n;++x)
		flagMarks[x] = flags[x] ? 1 : 0;
	
	while(width < n)
	{
		// SumToLeft
		if(reverse)
		{
			for(unsigned x=width;x<n;++x)
			{
				if(flags[x - ((unsigned) width)/2])
					sums[x]++;
			}
		} else {
			for(unsigned x=0;x<n - width;++x)
			{
				if(flags[x + ((unsigned) width)/2])
					sums[x]++;
			}
		}
		
		const int maxFlagged = (int) floor((1.0-minimumGoodDataRatio)*(num_t)(width));
		//ThresholdTime
		int halfWidthL = (width-1) / 2;
		int halfWidthR = (width-1) / 2;
		for(unsigned x=halfWidthL;x<n - halfWidthR;++x)
		{
			if(sums[x] > maxFlagged)
			{
				const unsigned right = x+halfWidthR+1;
				++flagMarks[x-halfWidthL];
				if(right < n)
					--flagMarks[right];
			}
		}
	
		++width;
		reverse = !reverse;
	}
	
	//ApplyMarksInTime
	int startedCount = 0;
	for(size_t x=0;x<n;++x)
	{
		startedCount += flagMarks[x];
		if(startedCount > 0)
			flags[x] = true;
	}

	delete[] sums;
	delete[] flagMarks;
}

void StatisticalFlagger::ScaleInvDilationQuick(bool *flags, const unsigned n, num_t minimumGoodDataRatio)
{
	num_t width = 2.0;
	unsigned iterations = 0, step = 1;
	bool reverse = false;
	
	int *sums = new int[n];
	int *flagMarks = new int[n];
	for(size_t x=0;x<n;++x)
		flagMarks[x] = flags[x] ? 1 : 0;
	
	while(width < n)
	{
		++iterations;
		
		// SumToLeft
		if(reverse)
		{
			for(unsigned x=width;x<n;++x)
			{
				if(flags[x - ((unsigned) width)/2])
					sums[x] += step;
			}
		} else {
			for(unsigned x=0;x<n - width;++x)
			{
				if(flags[x + ((unsigned) width)/2])
					sums[x] += step;
			}
		}
		
		const int maxFlagged = (int) floor((1.0-minimumGoodDataRatio)*(num_t)(width));
		//ThresholdTime
		int halfWidthL = (width-1) / 2;
		int halfWidthR = (width-1) / 2;
		for(unsigned x=halfWidthL;x<n - halfWidthR;++x)
		{
			if(sums[x] > maxFlagged)
			{
				const unsigned right = x+halfWidthR+1;
				++flagMarks[x-halfWidthL];
				if(right < n)
					--flagMarks[right];
			}
		}
	
		num_t newWidth = width * 1.05;
		if((unsigned) newWidth == width)
			newWidth = width + 1.0;
		step = (size_t) (newWidth - width);
		width = newWidth;
		reverse = !reverse;
	}
	
	//ApplyMarksInTime
	int startedCount = 0;
	for(size_t x=0;x<n;++x)
	{
		startedCount += flagMarks[x];
		if(startedCount > 0)
			flags[x] = true;
	}

	delete[] sums;
	delete[] flagMarks;
}
