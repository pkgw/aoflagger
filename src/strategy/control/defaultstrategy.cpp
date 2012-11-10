#include "defaultstrategy.h"

#include "../actions/adapter.h"
#include "../actions/baselineselectionaction.h"
#include "../actions/calibratepassbandaction.h"
#include "../actions/changeresolutionaction.h"
#include "../actions/combineflagresultsaction.h"
#include "../actions/foreachbaselineaction.h"
#include "../actions/foreachcomplexcomponentaction.h"
#include "../actions/foreachmsaction.h"
#include "../actions/foreachpolarisationaction.h"
#include "../actions/frequencyselectionaction.h"
#include "../actions/highpassfilteraction.h"
#include "../actions/iterationaction.h"
#include "../actions/plotaction.h"
#include "../actions/setflaggingaction.h"
#include "../actions/setimageaction.h"
#include "../actions/slidingwindowfitaction.h"
#include "../actions/statisticalflagaction.h"
#include "../actions/strategyaction.h"
#include "../actions/sumthresholdaction.h"
#include "../actions/timeselectionaction.h"
#include "../actions/writeflagsaction.h"

namespace rfiStrategy {

	const unsigned
		DefaultStrategy::FLAG_NONE             = aoflagger::StrategyFlags::NONE,
		DefaultStrategy::FLAG_LOW_FREQUENCY    = aoflagger::StrategyFlags::LOW_FREQUENCY,
		DefaultStrategy::FLAG_HIGH_FREQUENCY   = aoflagger::StrategyFlags::HIGH_FREQUENCY,
		DefaultStrategy::FLAG_LARGE_BANDWIDTH  = aoflagger::StrategyFlags::LARGE_BANDWIDTH,
		DefaultStrategy::FLAG_SMALL_BANDWIDTH  = aoflagger::StrategyFlags::SMALL_BANDWIDTH,
		DefaultStrategy::FLAG_TRANSIENTS       = aoflagger::StrategyFlags::TRANSIENTS,
		DefaultStrategy::FLAG_ROBUST           = aoflagger::StrategyFlags::ROBUST,
		DefaultStrategy::FLAG_FAST             = aoflagger::StrategyFlags::FAST,
		DefaultStrategy::FLAG_OFF_AXIS_SOURCES = aoflagger::StrategyFlags::OFF_AXIS_SOURCES,
		DefaultStrategy::FLAG_UNSENSITIVE      = aoflagger::StrategyFlags::UNSENSITIVE,
		DefaultStrategy::FLAG_SENSITIVE        = aoflagger::StrategyFlags::SENSITIVE,
		DefaultStrategy::FLAG_GUI_FRIENDLY     = aoflagger::StrategyFlags::GUI_FRIENDLY,
		DefaultStrategy::FLAG_CLEAR_FLAGS      = aoflagger::StrategyFlags::CLEAR_FLAGS,
		DefaultStrategy::FLAG_AUTO_CORRELATION = aoflagger::StrategyFlags::AUTO_CORRELATION;
			
	Strategy *DefaultStrategy::CreateStrategy(enum DefaultStrategyId strategyId, unsigned flags, double frequency, double timeRes, double frequencyRes)
	{
		bool calPassband =
			// Default MWA observations have strong frequency dependence
			(strategyId==MWA_STRATEGY && ((flags&FLAG_SMALL_BANDWIDTH) == 0)) ||
			// Other cases with large bandwidth
			((flags&FLAG_LARGE_BANDWIDTH) != 0);
		
		Strategy *strategy = new Strategy();
		LoadDefaultSingleStrategy(*strategy, false, false, calPassband);
		return strategy;
	}
	
	void DefaultStrategy::LoadDefaultSingleStrategy(ActionBlock &block, bool pedantic, bool pulsar, bool calPassband)
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
		if(calPassband)
		{
			current->Add(new CalibratePassbandAction());
		}
		
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

	void DefaultStrategy::LoadDefaultFullStrategy(ActionBlock &destination, bool pedantic, bool pulsar, bool calPassband)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		destination.Add(feBaseBlock);
		
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
		destination.Add(baselineSelection);
	}
	
}
