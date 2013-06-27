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

#include "../imagesets/imageset.h"
#include "../imagesets/fitsimageset.h"
#include "../imagesets/msimageset.h"

#include "../../msio/measurementset.h"

#include <boost/algorithm/string/case_conv.hpp>

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
	
	std::string DefaultStrategy::TelescopeName(DefaultStrategy::TelescopeId telescopeId)
	{
		switch(telescopeId)
		{
			default:
			case GENERIC_TELESCOPE: return "Generic";
			case ARECIBO_TELESCOPE: return "Arecibo";
			case JVLA_TELESCOPE: return "JVLA";
			case LOFAR_TELESCOPE: return "LOFAR";
			case MWA_TELESCOPE: return "MWA";
			case PARKES_TELESCOPE: return "Parkes";
			case WSRT_TELESCOPE: return "WSRT";
		}
	}
		
	DefaultStrategy::TelescopeId DefaultStrategy::TelescopeIdFromName(const std::string &name)
	{
		const std::string nameUpper = boost::algorithm::to_upper_copy(name);
		if(nameUpper == "ARECIBO" || nameUpper == "ARECIBO 305M")
			return ARECIBO_TELESCOPE;
		else if(nameUpper == "EVLA" || nameUpper == "JVLA")
			return JVLA_TELESCOPE;
		else if(nameUpper == "LOFAR")
			return LOFAR_TELESCOPE;
		else if(nameUpper == "MWA")
			return MWA_TELESCOPE;
		else if(nameUpper == "PKS" || nameUpper == "ATPKSMB")
			return PARKES_TELESCOPE;
		else if(nameUpper == "WSRT")
			return WSRT_TELESCOPE;
		else
			return GENERIC_TELESCOPE;
	}
	
	Strategy *DefaultStrategy::CreateStrategy(enum TelescopeId telescopeId, unsigned flags, double frequency, double timeRes, double frequencyRes)
	{
		Strategy *strategy = new Strategy();
		LoadStrategy(*strategy, telescopeId, flags, frequency, timeRes, frequencyRes);
		return strategy;
	}
	
	void DefaultStrategy::LoadStrategy(ActionBlock &strategy, enum TelescopeId telescopeId, unsigned flags, double frequency, double timeRes, double frequencyRes)
	{
		bool calPassband =
			// Default MWA observations have strong frequency dependency
			(telescopeId==MWA_TELESCOPE && ((flags&FLAG_SMALL_BANDWIDTH) == 0)) ||
			// JVLA observation I saw (around 1100 MHz) have steep band edges
			(telescopeId==JVLA_TELESCOPE && ((flags&FLAG_SMALL_BANDWIDTH) == 0)) ||
			// Other cases with large bandwidth
			((flags&FLAG_LARGE_BANDWIDTH) != 0);
		bool keepTransients = (flags&FLAG_TRANSIENTS) != 0;
		// Don't remove edges because of channel selection
		bool channelSelection = (telescopeId != JVLA_TELESCOPE);
		bool changeResVertically = true;
		// WSRT has automatic gain control, which strongly affect autocorrelations
		if(((flags&FLAG_AUTO_CORRELATION) != 0) && telescopeId == WSRT_TELESCOPE)
		{
			changeResVertically = false;
			keepTransients = true;
		}
		// JVLA observations I saw (around 1100 MHz) have steep band edges, so smooth very little
		if(telescopeId == JVLA_TELESCOPE)
		{
			changeResVertically = false;
		}
		bool clearFlags =
			((flags&FLAG_CLEAR_FLAGS) != 0) ||
			((flags&FLAG_GUI_FRIENDLY) != 0);
		bool resetContaminated =
			((flags&FLAG_GUI_FRIENDLY) != 0);
		int iterationCount = ((flags&FLAG_ROBUST)==0) ? 2 : 4;
		double sumThresholdSensitivity = 1.0;
		if(telescopeId == PARKES_TELESCOPE || telescopeId == WSRT_TELESCOPE)
			sumThresholdSensitivity = 1.4;
		else if(telescopeId == ARECIBO_TELESCOPE)
			sumThresholdSensitivity = 1.2;
		if((flags&FLAG_AUTO_CORRELATION) != 0)
			sumThresholdSensitivity *= 1.4;
		if((flags&FLAG_SENSITIVE) != 0)
			sumThresholdSensitivity /= 1.2;
		if((flags&FLAG_UNSENSITIVE) != 0)
			sumThresholdSensitivity *= 1.2;
		bool onStokesIQ = ((flags&FLAG_FAST) != 0);
		bool assembleStatistics = ((flags&FLAG_GUI_FRIENDLY)!=0) || telescopeId!=MWA_TELESCOPE;
		
		double verticalSmoothing = 5.0;
		if(telescopeId == JVLA_TELESCOPE)
			verticalSmoothing = 1.0;
		
		bool hasBaselines = telescopeId!=PARKES_TELESCOPE && telescopeId!=ARECIBO_TELESCOPE;
		
		LoadSingleStrategy(strategy, iterationCount, keepTransients, changeResVertically, calPassband, channelSelection, clearFlags, resetContaminated, sumThresholdSensitivity, onStokesIQ, assembleStatistics, verticalSmoothing, hasBaselines);
	}
	
	void DefaultStrategy::LoadSingleStrategy(ActionBlock &block, int iterationCount, bool keepTransients, bool changeResVertically, bool calPassband, bool channelSelection, bool clearFlags, bool resetContaminated, double sumThresholdSensitivity, bool onStokesIQ, bool assembleStatistics, double verticalSmoothing, bool hasBaselines)
	{
		ActionBlock *current;

		if(resetContaminated)
			block.Add(new SetImageAction());
		
		block.Add(new SetFlaggingAction());

		ForEachPolarisationBlock *fepBlock = new ForEachPolarisationBlock();
		if(onStokesIQ)
		{
			fepBlock->SetOnXX(false);
			fepBlock->SetOnXY(false);
			fepBlock->SetOnYX(false);
			fepBlock->SetOnYY(false);
			fepBlock->SetOnStokesI(true);
			fepBlock->SetOnStokesQ(true);
		}
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
		iteration->SetIterationCount(iterationCount);
		iteration->SetSensitivityStart(2.0 * pow(2.0, iterationCount/2.0));
		current->Add(iteration);
		current = iteration;
		
		SumThresholdAction *t1 = new SumThresholdAction();
		t1->SetBaseSensitivity(sumThresholdSensitivity);
		if(keepTransients)
			t1->SetFrequencyDirectionFlagging(false);
		current->Add(t1);

		CombineFlagResults *cfr1 = new CombineFlagResults();
		current->Add(cfr1);

		if(channelSelection)
			cfr1->Add(new FrequencySelectionAction());
		if(!keepTransients)
			cfr1->Add(new TimeSelectionAction());
	
		current->Add(new SetImageAction());
		
		if(!keepTransients || changeResVertically)
		{
			ChangeResolutionAction *changeResAction = new ChangeResolutionAction();
			if(keepTransients)
				changeResAction->SetTimeDecreaseFactor(1);
			else
				changeResAction->SetTimeDecreaseFactor(3);
			if(changeResVertically)
				changeResAction->SetFrequencyDecreaseFactor(3);
			else
				changeResAction->SetFrequencyDecreaseFactor(1);
			current->Add(changeResAction);
			current = changeResAction;
		}

		HighPassFilterAction *hpAction = new HighPassFilterAction();
		if(keepTransients)
		{
			hpAction->SetWindowWidth(1);
		} else {
			hpAction->SetHKernelSigmaSq(2.5);
			hpAction->SetWindowWidth(21);
		}
		hpAction->SetVKernelSigmaSq(verticalSmoothing);
		hpAction->SetWindowHeight(31);
		if(!keepTransients || changeResVertically)
			hpAction->SetMode(HighPassFilterAction::StoreRevised);
		else
			hpAction->SetMode(HighPassFilterAction::StoreContaminated);
		current->Add(hpAction);

		current = focAction;
		if(calPassband)
			current->Add(new CalibratePassbandAction());
		
		SumThresholdAction *t2 = new SumThresholdAction();
		t2->SetBaseSensitivity(sumThresholdSensitivity);
		if(keepTransients)
			t2->SetFrequencyDirectionFlagging(false);
		current->Add(t2);
		
		if(assembleStatistics)
		{
			PlotAction *plotPolarizationStatistics = new PlotAction();
			plotPolarizationStatistics->SetPlotKind(PlotAction::PolarizationStatisticsPlot);
			block.Add(plotPolarizationStatistics);
		}
		
		SetFlaggingAction *setFlagsInAllPolarizations = new SetFlaggingAction();
		setFlagsInAllPolarizations->SetNewFlagging(SetFlaggingAction::PolarisationsEqual);
		
		block.Add(setFlagsInAllPolarizations);
		block.Add(new StatisticalFlagAction());

		bool pedantic = false;
		if(pedantic)
		{
			CombineFlagResults *cfr2 = new CombineFlagResults();
			block.Add(cfr2);
			cfr2->Add(new FrequencySelectionAction());
			if(!keepTransients)
				cfr2->Add(new TimeSelectionAction());
		} else {
			if(!keepTransients)
				block.Add(new TimeSelectionAction());
		}

		if(assembleStatistics && hasBaselines)
		{
			BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
			baselineSelection->SetPreparationStep(true);
			block.Add(baselineSelection);
		}

		if(!clearFlags)
		{
			SetFlaggingAction *orWithOriginals = new SetFlaggingAction();
			orWithOriginals->SetNewFlagging(SetFlaggingAction::OrOriginal);
			block.Add(orWithOriginals);
		}
	}

	void DefaultStrategy::LoadFullStrategy(ActionBlock &destination, enum TelescopeId telescopeId, unsigned flags, double frequency, double timeRes, double frequencyRes)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		destination.Add(feBaseBlock);
		
		LoadStrategy(*feBaseBlock, telescopeId, flags, frequency, timeRes, frequencyRes);

		feBaseBlock->Add(new WriteFlagsAction());

		if(telescopeId != ARECIBO_TELESCOPE && telescopeId != PARKES_TELESCOPE)
		{
			PlotAction *antennaPlotAction = new PlotAction();
			antennaPlotAction->SetPlotKind(PlotAction::AntennaFlagCountPlot);
			feBaseBlock->Add(antennaPlotAction);
		}

		PlotAction *frequencyPlotAction = new PlotAction();
		frequencyPlotAction->SetPlotKind(PlotAction::FrequencyFlagCountPlot);
		feBaseBlock->Add(frequencyPlotAction);

		if(telescopeId != ARECIBO_TELESCOPE && telescopeId != PARKES_TELESCOPE)
		{
			BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
			baselineSelection->SetPreparationStep(false);
			destination.Add(baselineSelection);
		}
	}
	
	void DefaultStrategy::warnIfUnknownTelescope(DefaultStrategy::TelescopeId& telescopeId, const string& telescopeName)
	{
		if(telescopeId == GENERIC_TELESCOPE)
		{
			AOLogger::Warn << 
				"**\n"
				"** Measurement set specified the following telescope name: '" << telescopeName << "'\n"
				"** No good strategy is known for this telescope!\n"
				"** A generic strategy will be used which might not be optimal.\n"
				"**\n";
		}
	}
	
	void DefaultStrategy::DetermineSettings(MeasurementSet& measurementSet, DefaultStrategy::TelescopeId& telescopeId, unsigned int& flags, double& frequency, double& timeRes, double& frequencyRes)
	{
		AOLogger::Debug << "Determining best known strategy for measurement set...\n";
		
		std::string telescopeName = measurementSet.TelescopeName();
		telescopeId = TelescopeIdFromName(telescopeName);
		warnIfUnknownTelescope(telescopeId, telescopeName);
		
		flags = 0;
		size_t bandCount = measurementSet.BandCount();
		double frequencySum = 0.0, freqResSum = 0.0;
		size_t resSumCount = 0;
		for(size_t bandIndex=0; bandIndex!=bandCount; ++bandIndex)
		{
			const BandInfo &band = measurementSet.GetBandInfo(bandIndex);
			frequencySum += band.CenterFrequencyHz();
			if(band.channels.size() > 1)
			{
				const double
					startFrequency = band.channels.begin()->frequencyHz,
					endFrequency = band.channels.rbegin()->frequencyHz;
				freqResSum += fabs((endFrequency - startFrequency) / (band.channels.size() - 1));
				++resSumCount;
			}
		}
		if(bandCount != 0)
			frequency = frequencySum / (double) bandCount;
		else
			frequency = 0.0;
		
		if(resSumCount != 0)
			frequencyRes = freqResSum / (double) resSumCount;
		else
			frequencyRes = 0.0;
		
		const std::set<double> &obsTimes = measurementSet.GetObservationTimesSet();
		if(obsTimes.size() > 1)
		{
			double
				startTime = *obsTimes.begin(),
				endTime = *obsTimes.rbegin();
			timeRes = (endTime - startTime) / (double) (obsTimes.size() - 1);
		}
		else
			timeRes = 0.0;
		
		AOLogger::Info <<
			"The strategy will be optimized for the following settings:\n"
			"Telescope=" << TelescopeName(telescopeId) << ", flags=NONE, frequency="
			<< Frequency::ToString(frequency) << ",\n"
			"time resolution=" << timeRes << " s, frequency resolution=" << Frequency::ToString(frequencyRes) << '\n';
	}
	
	void DefaultStrategy::DetermineSettings(ImageSet& measurementSet, DefaultStrategy::TelescopeId& telescopeId, unsigned int& flags, double& frequency, double& timeRes, double& frequencyRes)
	{
		MSImageSet *msImageSet = dynamic_cast<MSImageSet*>(&measurementSet);
		if(msImageSet != 0)
		{
			DetermineSettings(
				msImageSet->Reader()->Set(),
				telescopeId,
				flags,
				frequency,
				timeRes,
				frequencyRes
			);
		} else {
			FitsImageSet *fitsImageSet = dynamic_cast<FitsImageSet*>(&measurementSet);
			if(fitsImageSet != 0)
			{
				std::string telescopeName = fitsImageSet->ReadTelescopeName();
				telescopeId = TelescopeIdFromName(telescopeName);
				warnIfUnknownTelescope(telescopeId, telescopeName);
				if(telescopeId != GENERIC_TELESCOPE)
					AOLogger::Info <<
						"The strategy will be optimized for telescope " << TelescopeName(telescopeId) << ". Telescope-specific\n"
						"settings will be left to their defaults, which might not be optimal for all cases.\n";
			} else {
				telescopeId = GENERIC_TELESCOPE;
				AOLogger::Warn <<
					"** Could not determine telescope name from set, because it has not\n"
					"** been implemented for this file format. A generic strategy will be used!\n";
			}
			flags = 0;
			frequency = 0.0;
			timeRes = 0.0;
			frequencyRes = 0.0;
		}
	}
	
}
