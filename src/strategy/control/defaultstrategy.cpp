#include "defaultstrategy.h"

#include "../actions/adapter.h"
#include "../actions/baselineselectionaction.h"
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

	void DefaultStrategy::LoadDefaultSingleStrategy(ActionBlock &block, bool pedantic, bool pulsar)
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

	void DefaultStrategy::LoadDefaultFullStrategy(ActionBlock &destination, bool pedantic, bool pulsar)
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
