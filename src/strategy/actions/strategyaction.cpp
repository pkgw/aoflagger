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
#include "strategyaction.h"

#include "adapter.h"
#include "baselineselectionaction.h"
#include "changeresolutionaction.h"
#include "combineflagresultsaction.h"
#include "foreachbaselineaction.h"
#include "foreachcomplexcomponentaction.h"
#include "foreachmsaction.h"
#include "foreachpolarisationaction.h"
#include "frequencyselectionaction.h"
#include "highpassfilteraction.h"
#include "iterationaction.h"
#include "plotaction.h"
#include "setflaggingaction.h"
#include "setimageaction.h"
#include "slidingwindowfitaction.h"
#include "statisticalflagaction.h"
#include "sumthresholdaction.h"
#include "timeselectionaction.h"
#include "writeflagsaction.h"

#include "../control/strategyiterator.h"

namespace rfiStrategy {

	Strategy *Strategy::CreateDefaultSingleStrategy()
	{
		Strategy *strategy = new Strategy();
		strategy->LoadDefaultSingleStrategy();
		return strategy;
	}

	void Strategy::LoadDefaultSingleStrategy(bool pedantic, bool pulsar)
	{
		LoadDefaultSingleStrategy(*this, pedantic, pulsar);
	}

	void Strategy::LoadDefaultSingleStrategy(ActionBlock &block, bool pedantic, bool pulsar)
	{
		ActionBlock *current;

		block.Add(new SetFlaggingAction());

		ForEachPolarisationBlock *fepBlock = new ForEachPolarisationBlock();
		block.Add(fepBlock);
		current = fepBlock;

		ForEachComplexComponentAction *focAction = new ForEachComplexComponentAction();
		focAction->SetOnAmplitude(true);
		focAction->SetOnImaginary(false);
		focAction->SetOnReal(false);
		focAction->SetOnPhase(false);
		focAction->SetRestoreFromAmplitude(false);
		current->Add(focAction);
		current = focAction;

		IterationBlock *iteration = new IterationBlock();
		iteration->SetIterationCount(2);
		iteration->SetSensitivityStart(4.0);
		current->Add(iteration);
		current = iteration;
		
		SumThresholdAction *t1 = new SumThresholdAction();
		t1->SetBaseSensitivity(1.0);
		if(pulsar)
			t1->SetFrequencyDirectionFlagging(false);
		current->Add(t1);

		CombineFlagResults *cfr1 = new CombineFlagResults();
		current->Add(cfr1);

		cfr1->Add(new FrequencySelectionAction());
		if(!pulsar)
			cfr1->Add(new TimeSelectionAction());
	
		current->Add(new SetImageAction());
		ChangeResolutionAction *changeResAction = new ChangeResolutionAction();
		if(pulsar)
			changeResAction->SetTimeDecreaseFactor(1);
		else
			changeResAction->SetTimeDecreaseFactor(3);
		changeResAction->SetFrequencyDecreaseFactor(3);

		/*
		SlidingWindowFitAction *swfAction2 = new SlidingWindowFitAction();
		if(pulsar)
		{
			swfAction2->Parameters().timeDirectionWindowSize = 1;
		} else {
			swfAction2->Parameters().timeDirectionKernelSize = 2.5;
			swfAction2->Parameters().timeDirectionWindowSize = 10;
		}
		swfAction2->Parameters().frequencyDirectionKernelSize = 5.0;
		swfAction2->Parameters().frequencyDirectionWindowSize = 15;
		changeResAction2->Add(swfAction2);
		
		Replaced the sliding window fit action by the faster (SSE) high-pass
		filter on 26-08-2011
		*/
		HighPassFilterAction *hpAction = new HighPassFilterAction();
		if(pulsar)
		{
			hpAction->SetWindowWidth(1);
		} else {
			hpAction->SetHKernelSigmaSq(2.5);
			hpAction->SetWindowWidth(21);
		}
		hpAction->SetVKernelSigmaSq(5.0);
		hpAction->SetWindowHeight(31);
		hpAction->SetMode(HighPassFilterAction::StoreRevised);
		changeResAction->Add(hpAction);

		current->Add(changeResAction);

		current = focAction;
		SumThresholdAction *t2 = new SumThresholdAction();
		if(pulsar)
			t2->SetFrequencyDirectionFlagging(false);
		current->Add(t2);
		
		PlotAction *plotPolarizationStatistics = new PlotAction();
		plotPolarizationStatistics->SetPlotKind(PlotAction::PolarizationStatisticsPlot);
		block.Add(plotPolarizationStatistics);
		
		SetFlaggingAction *setFlagsInAllPolarizations = new SetFlaggingAction();
		setFlagsInAllPolarizations->SetNewFlagging(SetFlaggingAction::PolarisationsEqual);
		
		block.Add(setFlagsInAllPolarizations);
		block.Add(new StatisticalFlagAction());

		if(pedantic)
		{
			CombineFlagResults *cfr2 = new CombineFlagResults();
			block.Add(cfr2);
			cfr2->Add(new FrequencySelectionAction());
			if(!pulsar)
				cfr2->Add(new TimeSelectionAction());
		} else {
			if(!pulsar)
				block.Add(new TimeSelectionAction());
		}

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(true);
		block.Add(baselineSelection);

		SetFlaggingAction *orWithOriginals = new SetFlaggingAction();
		orWithOriginals->SetNewFlagging(SetFlaggingAction::OrOriginal);
		block.Add(orWithOriginals);
	}

	void Strategy::LoadOldDefaultSingleStrategy()
	{
		Add(new SetFlaggingAction());
		ForEachPolarisationBlock *fepBlock = new ForEachPolarisationBlock();
		Add(fepBlock);

		CombineFlagResults *cfr = new CombineFlagResults();
		fepBlock->Add(cfr);
	
		Adapter *adapter = new Adapter();
		cfr->Add(adapter);
	
		IterationBlock *iteration = new IterationBlock();
		adapter->Add(iteration);
	
		iteration->Add(new SumThresholdAction());
		iteration->Add(new SetImageAction());
		iteration->Add(new SlidingWindowFitAction());

		adapter->Add(new SumThresholdAction());
	}

	void Strategy::LoadDefaultStrategy()
	{
		LoadAverageStrategy();
	}

	void Strategy::LoadFastStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());

		PlotAction *antennaPlotAction = new PlotAction();
		antennaPlotAction->SetPlotKind(PlotAction::AntennaFlagCountPlot);
		feBaseBlock->Add(antennaPlotAction);

		PlotAction *frequencyPlotAction = new PlotAction();
		frequencyPlotAction->SetPlotKind(PlotAction::FrequencyFlagCountPlot);
		feBaseBlock->Add(frequencyPlotAction);

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(false);
		Add(baselineSelection);
	}

	void Strategy::LoadAverageStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());

		PlotAction *antennaPlotAction = new PlotAction();
		antennaPlotAction->SetPlotKind(PlotAction::AntennaFlagCountPlot);
		feBaseBlock->Add(antennaPlotAction);

		PlotAction *frequencyPlotAction = new PlotAction();
		frequencyPlotAction->SetPlotKind(PlotAction::FrequencyFlagCountPlot);
		feBaseBlock->Add(frequencyPlotAction);

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(false);
		Add(baselineSelection);
	}

	void Strategy::LoadBestStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);
		
		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());

		PlotAction *antennaPlotAction = new PlotAction();
		antennaPlotAction->SetPlotKind(PlotAction::AntennaFlagCountPlot);
		feBaseBlock->Add(antennaPlotAction);

		PlotAction *frequencyPlotAction = new PlotAction();
		frequencyPlotAction->SetPlotKind(PlotAction::FrequencyFlagCountPlot);
		feBaseBlock->Add(frequencyPlotAction);

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(false);
		Add(baselineSelection);
	}
	
	ArtifactSet *Strategy::JoinThread()
	{
		ArtifactSet *artifact = 0;
		if(_thread != 0)
		{
			_thread->join();
			delete _thread;
			artifact = new ArtifactSet(*_threadFunc->_artifacts);
			delete _threadFunc->_artifacts;
			delete _threadFunc;
		}
		_thread = 0;
		return artifact;
	}

	void Strategy::StartPerformThread(const ArtifactSet &artifacts, ProgressListener &progress)
	{
		JoinThread();
		_threadFunc = new PerformFunc(this, new ArtifactSet(artifacts), &progress);
		_thread = new boost::thread(*_threadFunc);
	}

	void Strategy::PerformFunc::operator()()
	{
		_strategy->Perform(*_artifacts, *_progress);
	}

	void Strategy::SetThreadCount(Strategy &strategy, size_t threadCount)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachBaselineActionType)
			{
				ForEachBaselineAction &fobAction = static_cast<ForEachBaselineAction&>(*i);
				fobAction.SetThreadCount(threadCount);
			}
			if(i->Type() == WriteFlagsActionType)
			{
				WriteFlagsAction &writeAction = static_cast<WriteFlagsAction&>(*i);
				writeAction.SetMaxBufferItems(threadCount*5);
				writeAction.SetMinBufferItemsForWriting(threadCount*4);
			}
			++i;
		}
	}

	void Strategy::SetDataColumnName(Strategy &strategy, const std::string &dataColumnName)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachMSActionType)
			{
				ForEachMSAction &action = static_cast<ForEachMSAction&>(*i);
				action.SetDataColumnName(dataColumnName);
			}
			++i;
		}
	}

	/** TODO : implement, also in ForEachPolarisation */
	void Strategy::SetPolarisations(Strategy &, enum PolarisationType)
	{
	}

	void Strategy::SetBaselines(Strategy &strategy, enum BaselineSelection baselineSelection)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachBaselineActionType)
			{
				ForEachBaselineAction &fobAction = static_cast<ForEachBaselineAction&>(*i);
				fobAction.SetSelection(baselineSelection);
			}
			++i;
		}
	}

	void Strategy::SetFlagStokes(Strategy &strategy, bool newValue)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		bool hasBeenAdapted = false;
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachPolarisationBlockType)
			{
				if(hasBeenAdapted && newValue)
					throw std::runtime_error("Flagging on Stokes components was requested, but the separate real/imaginary values have already been converted to amplitude values before the polarization iteration.");

				ForEachPolarisationBlock &fopAction = static_cast<ForEachPolarisationBlock&>(*i);
				fopAction.SetIterateStokesValues(newValue);
			}
			else if(i->Type() == AdapterType)
			{
				hasBeenAdapted = true;
			}
			++i;
		}
	}

	void Strategy::SetTransientCompatibility(Strategy &strategy)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == SumThresholdActionType)
			{
				SumThresholdAction &action = static_cast<SumThresholdAction&>(*i);
				action.SetFrequencyDirectionFlagging(false);
			} else if(i->Type() == SlidingWindowFitActionType)
			{
				SlidingWindowFitAction &action = static_cast<SlidingWindowFitAction&>(*i);
				action.Parameters().timeDirectionWindowSize = 1;
			}
			++i;
		}
	}

	void Strategy::SetMultiplySensitivity(Strategy &strategy, num_t factor)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == SumThresholdActionType)
			{
				SumThresholdAction &action = static_cast<SumThresholdAction&>(*i);
				action.SetBaseSensitivity(action.BaseSensitivity() * factor);
			}
			++i;
		}
	}

	void Strategy::SetFittingWindowSize(Strategy &strategy, size_t windowWidth, size_t windowHeight)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == SlidingWindowFitActionType)
			{
				SlidingWindowFitAction &action = static_cast<SlidingWindowFitAction&>(*i);
				action.Parameters().timeDirectionWindowSize = windowWidth;
				action.Parameters().frequencyDirectionWindowSize = windowHeight;
			}
			++i;
		}
	}

	void Strategy::SetFittingKernelSize(Strategy &strategy, num_t kernelWidth, num_t kernelHeight)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == SlidingWindowFitActionType)
			{
				SlidingWindowFitAction &action = static_cast<SlidingWindowFitAction&>(*i);
				action.Parameters().timeDirectionKernelSize = kernelWidth;
				action.Parameters().frequencyDirectionKernelSize = kernelHeight;
			}
			++i;
		}
	}

	void Strategy::DisableOptimizations(Strategy &strategy)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == AdapterType)
			{
				Adapter &adapter = static_cast<Adapter&>(*i);
				adapter.SetRestoreOriginals(true);
			}
			++i;
		}
	}

	/*void Strategy::SetIndirectReader(Strategy &strategy, bool newValue)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachMSActionType)
			{
				ForEachMSAction &action = static_cast<ForEachMSAction&>(*i);
				action.SetIndirectReader(newValue);
			}
			++i;
		}
	}*/

	void Strategy::SyncAll(ActionContainer &root)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(root);
		while(!i.PastEnd())
		{
			i->Sync();
			++i;
		}
	}
}
