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
#ifndef STATISTICALFLAGGER_H
#define STATISTICALFLAGGER_H

#include <string>

#include "../../msio/mask2d.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class StatisticalFlagger{
	public:
		StatisticalFlagger();
		~StatisticalFlagger();
		
		static inline bool SquareContainsFlag(Mask2DCPtr mask, size_t xLeft, size_t yTop, size_t xRight, size_t yBottom);
		/**
		 * @deprecated Very slow, use DilateFlags
		 */
		static void EnlargeFlags(Mask2DPtr mask, size_t timeSize, size_t frequencySize);
		static void DilateFlags(Mask2DPtr mask, size_t timeSize, size_t frequencySize)
		{
			DilateFlagsHorizontally(mask, timeSize);
			DilateFlagsVertically(mask, frequencySize);
		}
		static void DilateFlagsHorizontally(Mask2DPtr mask, size_t timeSize);
		static void DilateFlagsVertically(Mask2DPtr mask, size_t frequencySize);
		static void LineRemover(Mask2DPtr mask, size_t maxTimeContamination, size_t maxFreqContamination);
		static void DensityTimeFlagger(Mask2DPtr mask, num_t minimumGoodDataRatio);
		static void DensityFrequencyFlagger(Mask2DPtr mask, num_t minimumGoodDataRatio);
		
		/**
		 * Performs an accurate scale invariant, but very slow. This algorithm is O(n^2),
		 * and has been superseded by a much quicker O(n) algorithm. See 
		 * ScaleInvariantDilation::Dilate().
		 * @deprecated Not efficient, use ScaleInvariantDilation::Dilate().
		 */
		static void ScaleInvDilationFull(bool *flags, const unsigned n, num_t minimumGoodDataRatio);
		
		/**
		 * Performs an inaccurate version of the scale invariant. This algorithm is O(n x log n),
		 * and has been superseded by a much quicker O(n) algorithm. See 
		 * ScaleInvariantDilation::Dilate().
		 * @deprecated Not efficient, use ScaleInvariantDilation::Dilate().
		 */
		static void ScaleInvDilationQuick(bool *flags, const unsigned n, num_t minimumGoodDataRatio);
	private:
		static void FlagTime(Mask2DPtr mask, size_t x);
		static void FlagFrequency(Mask2DPtr mask, size_t y);
		static void MaskToInts(Mask2DCPtr mask, int **maskAsInt);
		static void SumToLeft(Mask2DCPtr mask, int **sums, size_t width, size_t step, bool reverse);
		static void SumToTop(Mask2DCPtr mask, int **sums, size_t width, size_t step, bool reverse);
		static void ThresholdTime(Mask2DCPtr mask, int **flagMarks, int **sums, int thresholdLevel, int width);
		static void ThresholdFrequency(Mask2DCPtr mask, int **flagMarks, int **sums, int thresholdLevel, int width);
		static void ApplyMarksInTime(Mask2DPtr mask, int **flagMarks);
		static void ApplyMarksInFrequency(Mask2DPtr mask, int **flagMarks);
};

#endif
