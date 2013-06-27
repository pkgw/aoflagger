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
#ifndef THRESHOLDTOOLS_H
#define THRESHOLDTOOLS_H

#include <vector>
#include <cmath>

#include "../../msio/image2d.h"
#include "../../msio/mask2d.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ThresholdTools {
	public:
		static void MeanAndStdDev(const Image2DCPtr &image, const Mask2DCPtr &mask, num_t &mean, num_t &stddev);
		static numl_t Sum(const Image2DCPtr &image, const Mask2DCPtr &mask);
		static numl_t RMS(const Image2DCPtr &image, const Mask2DCPtr &mask);
		static num_t Mode(const Image2DCPtr &input, const Mask2DCPtr &mask);
		static num_t WinsorizedMode(const Image2DCPtr &image, const Mask2DCPtr &mask);
		static num_t WinsorizedMode(const Image2DCPtr &image);
		template<typename T>
		static void TrimmedMeanAndStdDev(const std::vector<T> &input, T &mean, T &stddev);
		template<typename T>
		static void WinsorizedMeanAndStdDev(const std::vector<T> &input, T &mean, T &stddev);
		static void WinsorizedMeanAndStdDev(const Image2DCPtr &image, const Mask2DCPtr &mask, num_t &mean, num_t &variance);
		static void WinsorizedMeanAndStdDev(const Image2DCPtr &image, num_t &mean, num_t &variance);
		static num_t MinValue(const Image2DCPtr &image, const Mask2DCPtr &mask);
		static num_t MaxValue(const Image2DCPtr &image, const Mask2DCPtr &mask);
		static void SetFlaggedValuesToZero(const Image2DPtr &dest, const Mask2DCPtr &mask);
		static void CountMaskLengths(const Mask2DCPtr &mask, int *lengths, size_t lengthsSize);
		
		static void FilterConnectedSamples(Mask2DPtr mask, size_t minConnectedSampleArea, bool eightConnected=true);
		static void FilterConnectedSample(Mask2DPtr mask, size_t x, size_t y, size_t minConnectedSampleArea, bool eightConnected=true);
		static void UnrollPhase(Image2DPtr image);
		static Image2DPtr ShrinkHorizontally(size_t factor, const Image2DCPtr &input, const Mask2DCPtr &mask);

		static Image2DPtr FrequencyRectangularConvolution(const Image2DCPtr &source, size_t convolutionSize)
		{
			Image2DPtr image = Image2D::CreateCopy(source);
			const size_t upperWindowHalf = (convolutionSize+1) / 2;
			for(size_t x=0;x<image->Width();++x)
			{
				num_t sum = 0.0;
				for(size_t y=0;y<upperWindowHalf;++y)
					sum += image->Value(x, y);
				for(size_t y=upperWindowHalf;y<convolutionSize;++y)
				{
					image->SetValue(x, y-upperWindowHalf, sum/(num_t) y);
					sum += image->Value(x, y);
				}
				size_t count = convolutionSize;
				for(size_t y=convolutionSize;y!=image->Height();++y)
				{
					image->SetValue(x, y-upperWindowHalf, sum/(num_t) count);
					sum += image->Value(x, y) - image->Value(x, y - convolutionSize);
				}
				for(size_t y=image->Height();y!=image->Height() + upperWindowHalf;++y)
				{
					image->SetValue(x, y-upperWindowHalf, sum/(num_t) count);
					sum -= image->Value(x, y - convolutionSize);
					--count;
				}
			}
			return image;
		}
		
		static Mask2DPtr Threshold(const Image2DCPtr &image, num_t threshold)
		{
			Mask2DPtr mask = Mask2D::CreateUnsetMaskPtr(image->Width(), image->Height());
			for(size_t y=0;y<image->Height();++y)
			{
				for(size_t x=0;x<image->Width();++x)
				{
					mask->SetValue(x, y, image->Value(x, y) >= threshold);
				}
			}
			return mask;
		}
	private:
		ThresholdTools() { }

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
