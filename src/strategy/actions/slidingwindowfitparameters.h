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

#include "../../msio/types.h"

#ifndef RFI_SLIDINGWINDOWFITPARAMETERS_H
#define RFI_SLIDINGWINDOWFITPARAMETERS_H

namespace rfiStrategy {

	struct SlidingWindowFitParameters
	{
		enum Method { None, Average, GaussianWeightedAverage, Median, Minimum };

		/**
		 * The method used for fitting.
		 */
		enum Method method;

		/**
		 * The window size in the time direction.
		 */
		size_t timeDirectionWindowSize;

		/**
		 * The window size in the frequency direction.
		 */
		size_t frequencyDirectionWindowSize;
		
		/**
		 * In the case of weighted average, the kernel size of the Gaussian function in the time direction.
		 */
		num_t timeDirectionKernelSize;

		/**
		 * In the case of weighted average, the kernel size of the Gaussian function in the frequency direction.
		 */
		num_t frequencyDirectionKernelSize;
	};
}

#endif // RFI_SLIDINGWINDOWFITPARAMETERS_H
