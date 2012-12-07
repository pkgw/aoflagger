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
#ifndef AOFLAGGER_DEFAULTSTRATEGYSPEEDTEST_H
#define AOFLAGGER_DEFAULTSTRATEGYSPEEDTEST_H

#include "../testingtools/asserter.h"
#include "../testingtools/unittest.h"

#include "../../strategy/algorithms/mitigationtester.h"
#include "../../strategy/algorithms/siroperator.h"
#include "../../strategy/algorithms/thresholdmitigater.h"

#include "../../strategy/actions/baselineselectionaction.h"
#include "../../strategy/actions/changeresolutionaction.h"
#include "../../strategy/actions/combineflagresultsaction.h"
#include "../../strategy/actions/foreachcomplexcomponentaction.h"
#include "../../strategy/actions/foreachpolarisationaction.h"
#include "../../strategy/actions/frequencyselectionaction.h"
#include "../../strategy/actions/highpassfilteraction.h"
#include "../../strategy/actions/iterationaction.h"
#include "../../strategy/actions/plotaction.h"
#include "../../strategy/actions/setflaggingaction.h"
#include "../../strategy/actions/setimageaction.h"
#include "../../strategy/actions/slidingwindowfitaction.h"
#include "../../strategy/actions/statisticalflagaction.h"
#include "../../strategy/actions/strategyaction.h"
#include "../../strategy/actions/sumthresholdaction.h"
#include "../../strategy/actions/timeselectionaction.h"

#include "../../strategy/control/artifactset.h"
#include "../../strategy/control/defaultstrategy.h"

#include "../../msio/timefrequencydata.h"

#include "../../util/aologger.h"

class DefaultStrategySpeedTest : public UnitTest {
	public:
		DefaultStrategySpeedTest() : UnitTest("Default strategy speed test")
		{
			if(false)
			{
				AddTest(TimeLoopUntilAmplitude(), "Timing loop until amplitude");
				AddTest(TimeLoop(), "Timing loop");
				AddTest(TimeSumThreshold(), "Timing SumThreshold method");
				AddTest(TimeRankOperator(), "Timing scale-invariant rank operator");
				//AddTest(TimeSumThresholdN(), "Timing varying SumThreshold method");
			}
			AddTest(TimeSlidingWindowFit(), "Timing sliding window fit");
			AddTest(TimeHighPassFilter(), "Timing high-pass filter");
			AddTest(TimeStrategy(), "Timing strategy");
			AddTest(TimeSSEHighPassFilterStrategy(), "Timing SSE high-pass filter strategy");
		}
		
		DefaultStrategySpeedTest(const std::string &) : UnitTest("Default strategy speed test")
		{
			AddTest(TimeSumThresholdN(), "Timing varying SumThreshold method");
		}
		
	private:
		struct TimeStrategy : public Asserter
		{
			void operator()();
		};
		struct TimeSlidingWindowFit : public Asserter
		{
			void operator()();
		};
		struct TimeHighPassFilter : public Asserter
		{
			void operator()();
		};
		struct TimeLoop : public Asserter
		{
			void operator()();
		};
		struct TimeLoopUntilAmplitude : public Asserter
		{
			void operator()();
		};
		struct TimeSumThreshold : public Asserter
		{
			void operator()();
		};
		struct TimeSumThresholdN : public Asserter
		{
			void operator()();
		};
		struct TimeRankOperator : public Asserter
		{
			void operator()();
		};
		struct TimeSSEHighPassFilterStrategy : public Asserter
		{
			void operator()();
		};
		
		static void prepareStrategy(rfiStrategy::ArtifactSet &artifacts);
};

inline void DefaultStrategySpeedTest::prepareStrategy(rfiStrategy::ArtifactSet &artifacts)
{
	const unsigned
		width = 10000,
		height = 256;
	Mask2DPtr rfi = Mask2D::CreateUnsetMaskPtr(width, height);
	Image2DPtr
		xxReal = MitigationTester::CreateTestSet(26, rfi, width, height),
		xxImag = MitigationTester::CreateTestSet(26, rfi, width, height),
		xyReal = MitigationTester::CreateTestSet(26, rfi, width, height),
		xyImag = MitigationTester::CreateTestSet(26, rfi, width, height),
		yxReal = MitigationTester::CreateTestSet(26, rfi, width, height),
		yxImag = MitigationTester::CreateTestSet(26, rfi, width, height),
		yyReal = MitigationTester::CreateTestSet(26, rfi, width, height),
		yyImag = MitigationTester::CreateTestSet(26, rfi, width, height);
	TimeFrequencyData data(
		xxReal, xxImag, xyReal, xyImag,
		yxReal, yxImag, yyReal, yyImag);
	artifacts.SetOriginalData(data);
	artifacts.SetContaminatedData(data);
	data.SetImagesToZero();
	artifacts.SetRevisedData(data);
}

inline void DefaultStrategySpeedTest::TimeStrategy::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);
	rfiStrategy::Strategy *strategy = rfiStrategy::DefaultStrategy::CreateStrategy(
		rfiStrategy::DefaultStrategy::GENERIC_TELESCOPE, rfiStrategy::DefaultStrategy::FLAG_NONE
	);
	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy->Perform(artifacts, progressListener);
	AOLogger::Info << "Default strategy took: " << watch.ToString() << '\n';
	delete strategy;
}

inline void DefaultStrategySpeedTest::TimeSlidingWindowFit::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);
	rfiStrategy::ActionBlock *current;

	rfiStrategy::Strategy strategy;
	
	rfiStrategy::ForEachPolarisationBlock *fepBlock = new rfiStrategy::ForEachPolarisationBlock();
	strategy.Add(fepBlock);
	current = fepBlock;

	rfiStrategy::ForEachComplexComponentAction *focAction = new rfiStrategy::ForEachComplexComponentAction();
	focAction->SetOnAmplitude(true);
	focAction->SetOnImaginary(false);
	focAction->SetOnReal(false);
	focAction->SetOnPhase(false);
	focAction->SetRestoreFromAmplitude(false);
	current->Add(focAction);
	current = focAction;

	rfiStrategy::IterationBlock *iteration = new rfiStrategy::IterationBlock();
	iteration->SetIterationCount(2);
	iteration->SetSensitivityStart(4.0);
	current->Add(iteration);
	current = iteration;
	
	rfiStrategy::ChangeResolutionAction *changeResAction2 = new rfiStrategy::ChangeResolutionAction();
	changeResAction2->SetTimeDecreaseFactor(3);
	changeResAction2->SetFrequencyDecreaseFactor(3);

	rfiStrategy::SlidingWindowFitAction *swfAction2 = new rfiStrategy::SlidingWindowFitAction();
	swfAction2->Parameters().timeDirectionKernelSize = 2.5;
	swfAction2->Parameters().timeDirectionWindowSize = 10;
	swfAction2->Parameters().frequencyDirectionKernelSize = 5.0;
	swfAction2->Parameters().frequencyDirectionWindowSize = 15;
	changeResAction2->Add(swfAction2);

	current->Add(changeResAction2);
	
	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy.Perform(artifacts, progressListener);
	AOLogger::Info << "Sliding window fit took (loop + fit): " << watch.ToString() << '\n';
}

inline void DefaultStrategySpeedTest::TimeHighPassFilter::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);
	rfiStrategy::ActionBlock *current;

	rfiStrategy::Strategy strategy;
	
	rfiStrategy::ForEachPolarisationBlock *fepBlock = new rfiStrategy::ForEachPolarisationBlock();
	strategy.Add(fepBlock);
	current = fepBlock;

	rfiStrategy::ForEachComplexComponentAction *focAction = new rfiStrategy::ForEachComplexComponentAction();
	focAction->SetOnAmplitude(true);
	focAction->SetOnImaginary(false);
	focAction->SetOnReal(false);
	focAction->SetOnPhase(false);
	focAction->SetRestoreFromAmplitude(false);
	current->Add(focAction);
	current = focAction;

	rfiStrategy::IterationBlock *iteration = new rfiStrategy::IterationBlock();
	iteration->SetIterationCount(2);
	iteration->SetSensitivityStart(4.0);
	current->Add(iteration);
	current = iteration;
	
	rfiStrategy::ChangeResolutionAction *changeResAction2 = new rfiStrategy::ChangeResolutionAction();
	changeResAction2->SetTimeDecreaseFactor(3);
	changeResAction2->SetFrequencyDecreaseFactor(3);

	rfiStrategy::HighPassFilterAction *hpAction = new rfiStrategy::HighPassFilterAction();
	hpAction->SetHKernelSigmaSq(2.5);
	hpAction->SetWindowWidth(10);
	hpAction->SetVKernelSigmaSq(5.0);
	hpAction->SetWindowHeight(15);
	hpAction->SetMode(rfiStrategy::HighPassFilterAction::StoreRevised);
	changeResAction2->Add(hpAction);

	current->Add(changeResAction2);
	
	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy.Perform(artifacts, progressListener);
	AOLogger::Info << "High-pass filter took (loop + fit): " << watch.ToString() << '\n';
}

inline void DefaultStrategySpeedTest::TimeLoop::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);
	rfiStrategy::ActionBlock *current;

	rfiStrategy::Strategy strategy;
	
	rfiStrategy::ForEachPolarisationBlock *fepBlock = new rfiStrategy::ForEachPolarisationBlock();
	strategy.Add(fepBlock);
	current = fepBlock;

	rfiStrategy::ForEachComplexComponentAction *focAction = new rfiStrategy::ForEachComplexComponentAction();
	focAction->SetOnAmplitude(true);
	focAction->SetOnImaginary(false);
	focAction->SetOnReal(false);
	focAction->SetOnPhase(false);
	focAction->SetRestoreFromAmplitude(false);
	current->Add(focAction);
	current = focAction;

	rfiStrategy::IterationBlock *iteration = new rfiStrategy::IterationBlock();
	iteration->SetIterationCount(2);
	iteration->SetSensitivityStart(4.0);
	current->Add(iteration);
	current = iteration;
	
	rfiStrategy::ChangeResolutionAction *changeResAction2 = new rfiStrategy::ChangeResolutionAction();
	changeResAction2->SetTimeDecreaseFactor(3);
	changeResAction2->SetFrequencyDecreaseFactor(3);
	current->Add(changeResAction2);
	
	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy.Perform(artifacts, progressListener);
	AOLogger::Info << "Loop took: " << watch.ToString() << '\n';
}

inline void DefaultStrategySpeedTest::TimeLoopUntilAmplitude::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);
	rfiStrategy::ActionBlock *current;

	rfiStrategy::Strategy strategy;
	
	rfiStrategy::ForEachPolarisationBlock *fepBlock = new rfiStrategy::ForEachPolarisationBlock();
	strategy.Add(fepBlock);
	current = fepBlock;

	rfiStrategy::ForEachComplexComponentAction *focAction = new rfiStrategy::ForEachComplexComponentAction();
	focAction->SetOnAmplitude(true);
	focAction->SetOnImaginary(false);
	focAction->SetOnReal(false);
	focAction->SetOnPhase(false);
	focAction->SetRestoreFromAmplitude(false);
	current->Add(focAction);

	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy.Perform(artifacts, progressListener);
	AOLogger::Info << "Loop took: " << watch.ToString() << '\n';
}

inline void DefaultStrategySpeedTest::TimeSumThreshold::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);
	rfiStrategy::ActionBlock *current;

	rfiStrategy::Strategy strategy;
	
	rfiStrategy::ForEachPolarisationBlock *fepBlock = new rfiStrategy::ForEachPolarisationBlock();
	strategy.Add(fepBlock);
	current = fepBlock;

	rfiStrategy::ForEachComplexComponentAction *focAction = new rfiStrategy::ForEachComplexComponentAction();
	focAction->SetOnAmplitude(true);
	focAction->SetOnImaginary(false);
	focAction->SetOnReal(false);
	focAction->SetOnPhase(false);
	focAction->SetRestoreFromAmplitude(false);
	current->Add(focAction);
	current = focAction;

	rfiStrategy::IterationBlock *iteration = new rfiStrategy::IterationBlock();
	iteration->SetIterationCount(2);
	iteration->SetSensitivityStart(4.0);
	current->Add(iteration);
	current = iteration;
	
	rfiStrategy::SumThresholdAction *t2 = new rfiStrategy::SumThresholdAction();
	t2->SetBaseSensitivity(1.0);
	current->Add(t2);
		
	rfiStrategy::ChangeResolutionAction *changeResAction2 = new rfiStrategy::ChangeResolutionAction();
	changeResAction2->SetTimeDecreaseFactor(3);
	changeResAction2->SetFrequencyDecreaseFactor(3);
	current->Add(changeResAction2);
	
	current = focAction;
	rfiStrategy::SumThresholdAction *t3 = new rfiStrategy::SumThresholdAction();
	current->Add(t3);
		
	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy.Perform(artifacts, progressListener);
	AOLogger::Info << "Sum threshold took (loop + threshold): " << watch.ToString() << '\n';
}

inline void DefaultStrategySpeedTest::TimeSumThresholdN::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);
	prepareStrategy(artifacts);

	ThresholdConfig config;
	config.InitializeLengthsDefault(9);
	num_t stddev = artifacts.OriginalData().GetSingleImage()->GetStdDev();
	num_t mode = artifacts.OriginalData().GetSingleImage()->GetMode();
	AOLogger::Info << "Stddev: " << stddev << '\n';
	AOLogger::Info << "Mode: " << mode << '\n';
	config.InitializeThresholdsFromFirstThreshold(6.0 * stddev, ThresholdConfig::Rayleigh);
	for(unsigned i=0;i<9;++i)
	{
		const unsigned length = config.GetHorizontalLength(i);
		const double threshold = config.GetHorizontalThreshold(i);
		Image2DCPtr input = artifacts.OriginalData().GetSingleImage();
		
		Mask2DPtr maskA = Mask2D::CreateCopy(artifacts.OriginalData().GetSingleMask());
		Stopwatch watchA(true);
		ThresholdMitigater::HorizontalSumThresholdLargeReference(input, maskA, length, threshold);
		AOLogger::Info << "Horizontal, length " << length << ": " << watchA.ToString() << '\n';
		
		Mask2DPtr maskC = Mask2D::CreateCopy(artifacts.OriginalData().GetSingleMask());
		Stopwatch watchC(true);
		ThresholdMitigater::HorizontalSumThresholdLargeSSE(input, maskC, length, threshold);
		AOLogger::Info << "Horizontal SSE, length " << length << ": " << watchC.ToString() << '\n';
		
		Mask2DPtr maskB = Mask2D::CreateCopy(artifacts.OriginalData().GetSingleMask());
		Stopwatch watchB(true);
		ThresholdMitigater::VerticalSumThresholdLargeReference(input, maskB, length, threshold);
		AOLogger::Info << "Vertical, length " << length << ": " << watchB.ToString() << '\n';
		
		Mask2DPtr maskD = Mask2D::CreateCopy(artifacts.OriginalData().GetSingleMask());
		Stopwatch watchD(true);
		ThresholdMitigater::VerticalSumThresholdLargeSSE(input, maskD, length, threshold);
		AOLogger::Info << "SSE Vertical, length " << length << ": " << watchD.ToString() << '\n';
	}
}

inline void DefaultStrategySpeedTest::TimeRankOperator::operator()()
{
	rfiStrategy::ArtifactSet artifacts(0);

	rfiStrategy::Strategy *strategy = rfiStrategy::DefaultStrategy::CreateStrategy(
		rfiStrategy::DefaultStrategy::GENERIC_TELESCOPE, rfiStrategy::DefaultStrategy::FLAG_NONE
	);
	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy->Perform(artifacts, progressListener);
	watch.Pause();
	delete strategy;
	
	Mask2DPtr input = Mask2D::CreateCopy(artifacts.ContaminatedData().GetSingleMask());
	
	Stopwatch operatorTimer(true);
	SIROperator::OperateHorizontally(input, 0.2);
	SIROperator::OperateVertically(input, 0.2);
	operatorTimer.Pause();
	
	long double operatorTime = operatorTimer.Seconds();
	long double totalTime = watch.Seconds();
	
	AOLogger::Info
		<< "Rank operator took " << operatorTimer.ToShortString() << " (" << operatorTime << ")"
		<< " of " << watch.ToShortString() << " (" << totalTime << ")"
		<< ", " << ( operatorTime * 100.0 / totalTime) << "%\n";
}

inline void DefaultStrategySpeedTest::TimeSSEHighPassFilterStrategy::operator()()
{
	rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
	rfiStrategy::ActionBlock &block = *strategy;
	rfiStrategy::ActionBlock *current;

	block.Add(new rfiStrategy::SetFlaggingAction());

	rfiStrategy::ForEachPolarisationBlock *fepBlock = new rfiStrategy::ForEachPolarisationBlock();
	block.Add(fepBlock);
	current = fepBlock;

	rfiStrategy::ForEachComplexComponentAction *focAction = new rfiStrategy::ForEachComplexComponentAction();
	focAction->SetOnAmplitude(true);
	focAction->SetOnImaginary(false);
	focAction->SetOnReal(false);
	focAction->SetOnPhase(false);
	focAction->SetRestoreFromAmplitude(false);
	current->Add(focAction);
	current = focAction;

	rfiStrategy::IterationBlock *iteration = new rfiStrategy::IterationBlock();
	iteration->SetIterationCount(2);
	iteration->SetSensitivityStart(4.0);
	current->Add(iteration);
	current = iteration;
	
	rfiStrategy::SumThresholdAction *t2 = new rfiStrategy::SumThresholdAction();
	t2->SetBaseSensitivity(1.0);
	current->Add(t2);

	rfiStrategy::CombineFlagResults *cfr2 = new rfiStrategy::CombineFlagResults();
	current->Add(cfr2);

	cfr2->Add(new rfiStrategy::FrequencySelectionAction());
	cfr2->Add(new rfiStrategy::TimeSelectionAction());

	current->Add(new rfiStrategy::SetImageAction());
	rfiStrategy::ChangeResolutionAction
		*changeResAction2 = new rfiStrategy::ChangeResolutionAction();
	changeResAction2->SetTimeDecreaseFactor(3);
	changeResAction2->SetFrequencyDecreaseFactor(3);

	rfiStrategy::HighPassFilterAction *hpAction = new rfiStrategy::HighPassFilterAction();
	hpAction->SetHKernelSigmaSq(2.5);
	hpAction->SetWindowWidth(10*2+1);
	hpAction->SetVKernelSigmaSq(5.0);
	hpAction->SetWindowHeight(15*2+1);
	hpAction->SetMode(rfiStrategy::HighPassFilterAction::StoreRevised);
	changeResAction2->Add(hpAction);

	current->Add(changeResAction2);

	current = focAction;
	rfiStrategy::SumThresholdAction *t3 = new rfiStrategy::SumThresholdAction();
	current->Add(t3);
	
	rfiStrategy::PlotAction *plotPolarizationStatistics = new rfiStrategy::PlotAction();
	plotPolarizationStatistics->SetPlotKind(rfiStrategy::PlotAction::PolarizationStatisticsPlot);
	block.Add(plotPolarizationStatistics);
	
	rfiStrategy::SetFlaggingAction
		*setFlagsInAllPolarizations = new rfiStrategy::SetFlaggingAction();
	setFlagsInAllPolarizations->SetNewFlagging(rfiStrategy::SetFlaggingAction::PolarisationsEqual);
	
	block.Add(setFlagsInAllPolarizations);
	block.Add(new rfiStrategy::StatisticalFlagAction());
	block.Add(new rfiStrategy::TimeSelectionAction());

	rfiStrategy::BaselineSelectionAction
		*baselineSelection = new rfiStrategy::BaselineSelectionAction();
	baselineSelection->SetPreparationStep(true);
	block.Add(baselineSelection);

	rfiStrategy::SetFlaggingAction *orWithOriginals = new rfiStrategy::SetFlaggingAction();
	orWithOriginals->SetNewFlagging(rfiStrategy::SetFlaggingAction::OrOriginal);
	block.Add(orWithOriginals);

	rfiStrategy::ArtifactSet artifacts(0);
	prepareStrategy(artifacts);
	DummyProgressListener progressListener;
	Stopwatch watch(true);
	strategy->Perform(artifacts, progressListener);
	AOLogger::Info << "Default strategy took: " << watch.ToString() << '\n';
	delete strategy;
}

#endif
