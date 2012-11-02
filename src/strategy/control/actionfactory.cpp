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
#include "actionfactory.h"

#include "../actions/absthresholdaction.h"
#include "../actions/action.h"
#include "../actions/adapter.h"
#include "../actions/addstatisticsaction.h"
#include "../actions/baselineselectionaction.h"
#include "../actions/changeresolutionaction.h"
#include "../actions/collectnoisestatisticsaction.h"
#include "../actions/combineflagresultsaction.h"
#include "../actions/cutareaaction.h"
#include "../actions/directionalcleanaction.h"
#include "../actions/directionprofileaction.h"
#include "../actions/eigenvalueverticalaction.h"
#include "../actions/foreachbaselineaction.h"
#include "../actions/foreachcomplexcomponentaction.h"
#include "../actions/foreachpolarisationaction.h"
#include "../actions/foreachsimulatedbaselineaction.h"
#include "../actions/foreachmsaction.h"
#include "../actions/fouriertransformaction.h"
#include "../actions/frequencyconvolutionaction.h"
#include "../actions/frequencyselectionaction.h"
#include "../actions/fringestopaction.h"
#include "../actions/imageraction.h"
#include "../actions/iterationaction.h"
#include "../actions/highpassfilteraction.h"
#include "../actions/normalizevarianceaction.h"
#include "../actions/plotaction.h"
#include "../actions/quickcalibrateaction.h"
#include "../actions/rawappenderaction.h"
#include "../actions/resamplingaction.h"
#include "../actions/setflaggingaction.h"
#include "../actions/setimageaction.h"
#include "../actions/slidingwindowfitaction.h"
#include "../actions/spatialcompositionaction.h"
#include "../actions/statisticalflagaction.h"
#include "../actions/svdaction.h"
#include "../actions/sumthresholdaction.h"
#include "../actions/timeconvolutionaction.h"
#include "../actions/timeselectionaction.h"
#include "../actions/uvprojectaction.h"
#include "../actions/writedataaction.h"
#include "../actions/writeflagsaction.h"

namespace rfiStrategy {

const std::vector<std::string> ActionFactory::GetActionList()
{
	std::vector<std::string> list;
	list.push_back("Absolute threshold");
	list.push_back("Add to statistics");
	list.push_back("Baseline selection");
	list.push_back("Change resolution");
	list.push_back("Collect noise statistics");
	list.push_back("Combine flag results");
	list.push_back("Cut area");
	list.push_back("Directional CLEAN");
	list.push_back("Direction profile");
	list.push_back("Eigen value decompisition (vertical)");
	list.push_back("For each baseline");
	list.push_back("For each complex component");
	list.push_back("For each polarisation");
	list.push_back("For each simulated baseline");
	list.push_back("For each measurement set");
	list.push_back("Fourier transformation");
	list.push_back("Frequency convolution");
	list.push_back("Frequency selection");
	list.push_back("Fringe stopping recovery");
	list.push_back("High-pass filter");
	list.push_back("Image");
	list.push_back("Iteration");
	list.push_back("Normalize variance");
	list.push_back("Phase adapter");
	list.push_back("Plot");
	list.push_back("Quickly calibrate");
	list.push_back("Raw appender");
	list.push_back("Resample");
	list.push_back("Set flagging");
	list.push_back("Set image");
	list.push_back("Singular value decomposition");
	list.push_back("Sliding window fit");
	list.push_back("Spatial composition");
	list.push_back("Statistical flagging");
	list.push_back("SumThreshold");
	list.push_back("Time convolution");
	list.push_back("Time selection");
	list.push_back("UV-projection");
	list.push_back("Write data");
	list.push_back("Write flags");
	return list;
}

Action *ActionFactory::CreateAction(const std::string &action)
{
	if(action == "Absolute threshold")
		return new AbsThresholdAction();
	else if(action == "Add to statistics")
		return new AddStatisticsAction();
	else if(action == "Baseline selection")
		return new BaselineSelectionAction();
	else if(action == "Change resolution")
		return new ChangeResolutionAction();
	else if(action == "Collect noise statistics")
		return new CollectNoiseStatisticsAction();
	else if(action == "Combine flag results")
		return new CombineFlagResults();
	else if(action == "Cut area")
		return new CutAreaAction();
	else if(action == "Directional CLEAN")
		return new DirectionalCleanAction();
	else if(action == "Direction profile")
		return new DirectionProfileAction();
	else if(action == "Eigen value decompisition (vertical)")
		return new EigenValueVerticalAction();
	else if(action == "For each baseline")
		return new ForEachBaselineAction();
	else if(action == "For each complex component")
		return new ForEachComplexComponentAction();
	else if(action == "For each measurement set")
		return new ForEachMSAction();
	else if(action == "For each polarisation")
		return new ForEachPolarisationBlock();
	else if(action == "For each simulated baseline")
		return new ForEachSimulatedBaselineAction();
	else if(action == "Frequency convolution")
		return new FrequencyConvolutionAction();
	else if(action == "Frequency selection")
		return new FrequencySelectionAction();
	else if(action == "Fringe stopping recovery")
		return new FringeStopAction();
	else if(action == "Fourier transformation")
		return new FourierTransformAction();
	else if(action == "High-pass filter")
		return new HighPassFilterAction();
	else if(action == "Image")
		return new ImagerAction();
	else if(action == "Iteration")
		return new IterationBlock();
	else if(action == "Normalize variance")
		return new NormalizeVarianceAction();
	else if(action == "Phase adapter")
		return new Adapter();
	else if(action == "Plot")
		return new PlotAction();
	else if(action == "Quickly calibrate")
		return new QuickCalibrateAction();
	else if(action == "Raw appender")
		return new RawAppenderAction();
	else if(action == "Resample")
		return new ResamplingAction();
	else if(action == "Set flagging")
		return new SetFlaggingAction();
	else if(action == "Set image")
		return new SetImageAction();
	else if(action == "Singular value decomposition")
		return new SVDAction();
	else if(action == "Sliding window fit")
		return new SlidingWindowFitAction();
	else if(action == "Spatial composition")
		return new SpatialCompositionAction();
	else if(action == "Statistical flagging")
		return new StatisticalFlagAction();
	else if(action == "SumThreshold")
		return new SumThresholdAction();
	else if(action == "Time convolution")
		return new TimeConvolutionAction();
	else if(action == "Time selection")
		return new TimeSelectionAction();
	else if(action == "UV-projection")
		return new UVProjectAction();
	else if(action == "Write data")
		return new WriteDataAction();
	else if(action == "Write flags")
		return new WriteFlagsAction();
	else
		throw BadUsageException(std::string("Trying to create unknown action \"") + action + "\"");
}

const char *ActionFactory::GetDescription(ActionType action)
{
	switch(action)
	{
		case AdapterType:
			return
				"The adapter calculates the amplitude from the complex value. Most algorithmic "
				"actions like the SumThreshold method work only on single values, not on complex "
				"values, hence the need for this action. This should normally not be changed. It "
				"has currently no parameters.";
		case ChangeResolutionActionType:
			return
				"Changes the resolution of the time frequency data currently in memory. This is "
				"part of the algorithm and should normally not be changed. Currently changes only "
				"the time direction.";
		case CombineFlagResultsType:
			return
				"Runs each of its children and combines the flags (by OR-ing) afterwards.";
		case ForEachBaselineActionType:
			return
				"Iterate over baselines in the measurement set. Parameters: selection : which "
				"baselines to iterate over (0. All, 1. CrossCorrelations, 2. AutoCorrelations, "
				"3. Baselines that are redundant to current selected, 4. Auto correlations of "
				"the current antenna, 5. don't iterate, only process current), thread-count : "
				"number of threads to spawn simultaneously.";
		case ForEachPolarisationBlockType:
			return
				"Iterate over the polarisations that are already in memory. Note that the "
				"LoadImageAction actually defines which polarisations are read in memory in "
				"the first place. This action has no parameters.";
		case FrequencySelectionActionType:
			return
				"Flag frequency channels that are very different from other channels.";
		case SetFlaggingActionType:
			return
				"This is an action that is part of the algorithm and should normally not be "
				"changed. "
				"It initializes or changes the flags in memory. Its single parameter new-flagging "
				"defines how to initialize or set the flags (0. None, 1. Everything, 2. FromOriginal, "
				"3. Invert, 4. PolarisationsEqual, 5. FlagZeros).";
		case SetImageActionType:
			return
				"(Re-)initialize the time frequency data in memory. Parameter 'new-image' defines "
				"how to initialize (0 means set the entire image to zero, 1 means that, if the "
				"image has been changed by e.g. a surface fit, restore the image (a copy "
				"of the original data is left in memory, so this does not perform IO). This is "
				"part of the algorithm and should normally not be changed.";
		case SlidingWindowFitActionType:
			return
				"Performs a surface fit / smoothing operation and subtracts the fit from the data. "
				"The window size parameters are currently absolute values, and the default values "
				"have been optimized for LOFAR 1 Hz 1 sec resolution. Nevertheless, these "
				"settings work well for other configurations as well (and surface fitting is not "
				"the most crucial part of the flagger - because of the SumThreshold method, even "
				"bad fits produce reasonable results). Note that the ChangeResolutionAction"
				"changes the meaning of these absolute values as well.";
		case TimeSelectionActionType:
			return
				"Flag time scans that are very different from other time steps";
		case SumThresholdActionType:
			return
				"This executes the SumThreshold method. It has parameters to change its sensitivity and "
				"to set whether the method is allowed to look to combinations of time and frequency directions. "
				"If you expect strong transient effects that you do not want to flag, set frequency-direction-flagging "
				"to 0 to make sure that frequencies are not combined. Not that the algorithm normally iterates "
				"and executes the SumThreshold method several times with different sensitivities. Therefore, if you "
				"like to change the sensitivity "
				"of the algorithm in order to flag less or more samples, you have to change the sensitivity of all "
				"Threshold actions by the same factor. A higher sensitivity-value means that less values are flagged.";
		case StatisticalFlagActionType:
			return
				"This action implements a novel and rather complex method to flag samples that are likely RFI "
				"based on its surrounding flags. It is sort of a dillation.";
		case WriteFlagsActionType:
			return
				"Write the newly constructed flags to the measurement set. Normally it is executed once at the end "
				"of the algorithm.";
		default: return 0;
	}
}

} // namespace
