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

#ifndef RFIACTION_H
#define RFIACTION_H 

#include <string>

#include "../../util/types.h"

namespace rfiStrategy {

	enum ActionType
	{
		AbsThresholdActionType,
		ActionBlockType,
		AdapterType,
		AddStatisticsActionType,
		BaselineSelectionActionType,
		ChangeResolutionActionType,
		CollectNoiseStatisticsActionType,
		CombineFlagResultsType,
		CutAreaActionType,
		DirectionProfileActionType,
		DirectionalCleanActionType,
		EigenValueVerticalActionType,
		ForEachBaselineActionType,
		ForEachComplexComponentActionType,
		ForEachMSActionType,
		ForEachPolarisationBlockType,
		ForEachSimulatedBaselineActionType,
		FourierTransformActionType,
		FrequencyConvolutionActionType,
		FrequencySelectionActionType,
		FringeStopActionType,
		HighPassFilterActionType,
		ImagerActionType,
		IterationBlockType,
		NormalizeVarianceActionType,
		PlotActionType,
		QuickCalibrateActionType,
		RawAppenderActionType,
		ResamplingActionType,
		SetFlaggingActionType,
		SetImageActionType,
		SlidingWindowFitActionType,
		SpatialCompositionActionType,
		StatisticalFlagActionType,
		StrategyType,
		SumThresholdActionType,
		SVDActionType,
		TimeConvolutionActionType,
		TimeSelectionActionType,
		UVProjectActionType,
		WriteDataActionType,
		WriteFlagsActionType
	};

	class Action
	{
		friend class ActionContainer;

		public:
			Action() : _parent(0) { }
			virtual ~Action() { }
			virtual std::string Description() = 0;
			virtual void Initialize() { }
			virtual void Finish() { }
			/**
			 * Write any cached / delayed data to disk
			 */
			virtual void Sync() { }
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &progress) = 0;
			class ActionContainer *Parent() const { return _parent; }
			virtual ActionType Type() const = 0;

			inline ActionContainer *GetRoot() const;
			virtual unsigned int Weight() const { return 1; }
		private:
			class ActionContainer *_parent;
	};
}

#include "../control/actioncontainer.h"

namespace rfiStrategy {

	ActionContainer *Action::GetRoot() const
	{
		if(_parent == 0)
			return static_cast<ActionContainer*>(_parent);
		else
		{
			ActionContainer *iter = _parent;
			while(iter->_parent != 0) iter = iter->_parent;
			return iter;
		}
	}

}

#endif // RFIACTION_H
