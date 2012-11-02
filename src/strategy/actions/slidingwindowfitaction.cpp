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

#include "../../util/progresslistener.h"

#include "slidingwindowfitaction.h"

#include "../algorithms/localfitmethod.h"

#include "../control/artifactset.h"

namespace rfiStrategy {

	void SlidingWindowFitAction::LoadDefaults()
	{
		_parameters.frequencyDirectionKernelSize = 15.0;
		_parameters.frequencyDirectionWindowSize = 40;
		_parameters.method = SlidingWindowFitParameters::GaussianWeightedAverage;
		_parameters.timeDirectionKernelSize = 7.5;
		_parameters.timeDirectionWindowSize = 20;
	}

	void SlidingWindowFitAction::Perform(ArtifactSet &artifacts, class ProgressListener &listener)
	{
		LocalFitMethod method;
		switch(_parameters.method)
		{
			case SlidingWindowFitParameters::None:
				method.SetToNone();
				break;
			case SlidingWindowFitParameters::Average:
				method.SetToAverage(
					_parameters.timeDirectionWindowSize,
					_parameters.frequencyDirectionWindowSize);
				break;
			case SlidingWindowFitParameters::GaussianWeightedAverage:
				method.SetToWeightedAverage(
					_parameters.timeDirectionWindowSize,
					_parameters.frequencyDirectionWindowSize,
					_parameters.timeDirectionKernelSize,
					_parameters.frequencyDirectionKernelSize);
				break;
			case SlidingWindowFitParameters::Median:
				method.SetToMedianFilter(
					_parameters.timeDirectionWindowSize,
					_parameters.frequencyDirectionWindowSize);
				break;
			case SlidingWindowFitParameters::Minimum:
				method.SetToMinimumFilter(
					_parameters.timeDirectionWindowSize,
					_parameters.frequencyDirectionWindowSize);
				break;
		}

		method.Initialize(artifacts.ContaminatedData());
		
		size_t taskCount = method.TaskCount();
		for(size_t i=0;i<taskCount;++i)
		{
			method.PerformFit(i);
			listener.OnProgress(*this, i+1, taskCount);
		}
		TimeFrequencyData newRevisedData = method.Background();
		newRevisedData.SetMask(artifacts.RevisedData());

		TimeFrequencyData *contaminatedData =
			TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), newRevisedData);
		contaminatedData->SetMask(artifacts.ContaminatedData());

		artifacts.SetRevisedData(newRevisedData);
		artifacts.SetContaminatedData(*contaminatedData);

		delete contaminatedData;
	}
} // namespace rfiStrategy
