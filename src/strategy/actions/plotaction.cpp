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
#include <boost/thread.hpp>

#include "plotaction.h"

#include "../plots/antennaflagcountplot.h"
#include "../plots/frequencyflagcountplot.h"
#include "../plots/frequencypowerplot.h"
#include "../plots/iterationsplot.h"
#include "../plots/timeflagcountplot.h"

#include "../algorithms/polarizationstatistics.h"
#include "../algorithms/thresholdtools.h"

#include "../control/artifactset.h"

namespace rfiStrategy {
	
	void PlotAction::Perform(ArtifactSet &artifacts, ProgressListener &)
	{
		boost::mutex::scoped_lock lock(_plotMutex);
		switch(_plotKind)
		{
			case AntennaFlagCountPlot:
				plotAntennaFlagCounts(artifacts);
				break;
			case FrequencyFlagCountPlot:
				plotFrequencyFlagCounts(artifacts);
				break;
			case FrequencyPowerPlot:
				plotFrequencyPower(artifacts);
				break;
			case TimeFlagCountPlot:
				plotTimeFlagCounts(artifacts);
				break;
			case BaselineSpectrumPlot:
				plotSpectrumPerBaseline(artifacts);
				break;
			case PolarizationStatisticsPlot:
				plotPolarizationFlagCounts(artifacts);
				break;
			case BaselineRMSPlot:
				plotBaselineRMS(artifacts);
				break;
			case IterationsPlot:
				plotIterations(artifacts);
				break;
		}
	}


#ifdef HAVE_GTKMM

	void PlotAction::plotAntennaFlagCounts(ArtifactSet &artifacts)
	{
		if(artifacts.AntennaFlagCountPlot() == 0)
			throw BadUsageException("No antenna flag count plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.AntennaFlagCountPlot()->Add(data, meta);
	}

	void PlotAction::plotFrequencyFlagCounts(ArtifactSet &artifacts)
	{
		if(artifacts.FrequencyFlagCountPlot() == 0)
			throw BadUsageException("No frequency flag count plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.FrequencyFlagCountPlot()->Add(data, meta);
	}

	void PlotAction::plotFrequencyPower(ArtifactSet &artifacts)
	{
		if(artifacts.FrequencyPowerPlot() == 0)
			throw BadUsageException("No frequency power plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.FrequencyPowerPlot()->Add(data, meta);
	}

	void PlotAction::plotTimeFlagCounts(ArtifactSet &artifacts)
	{
		if(artifacts.TimeFlagCountPlot() == 0)
			throw BadUsageException("No time flag count plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.TimeFlagCountPlot()->Add(data, meta);
	}

	void PlotAction::plotSpectrumPerBaseline(ArtifactSet &artifacts)
	{
		if(artifacts.FrequencyPowerPlot() == 0)
			throw BadUsageException("No frequency power plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.FrequencyPowerPlot()->SetLogYAxis(_logYAxis);
		artifacts.FrequencyPowerPlot()->StartNewLine(meta->Antenna1().name + " x " + meta->Antenna2().name);
		artifacts.FrequencyPowerPlot()->Add(data, meta);
	}

	void PlotAction::plotPolarizationFlagCounts(ArtifactSet &artifacts)
	{
		if(artifacts.PolarizationStatistics() == 0)
			throw BadUsageException("No polarization statistics in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		artifacts.PolarizationStatistics()->Add(data);
	}

	void PlotAction::plotBaselineRMS(ArtifactSet &artifacts)
	{
		if(artifacts.PolarizationStatistics() == 0)
			throw BadUsageException("No polarization statistics in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();
		double rms = 0.0;
		for(unsigned i=0;i<data.PolarisationCount();++i)
		{
			TimeFrequencyData *polarisation = data.CreateTFDataFromPolarisationIndex(i);
			Mask2DCPtr mask = polarisation->GetSingleMask();
			for(unsigned j=0;j<polarisation->ImageCount();++j)
			{
				Image2DCPtr image = polarisation->GetImage(j);
				rms += ThresholdTools::RMS(image, mask);
			}
			delete polarisation;
		}
		rms /= data.PolarisationCount();
		;
		AOLogger::Info << "RMS of " << metaData->Antenna1().name << " x " << metaData->Antenna2().name << ": "
			<< rms << '\n';
	}
	
	void PlotAction::plotIterations(class ArtifactSet &artifacts)
	{
		class IterationsPlot *plot = artifacts.IterationsPlot();
		if(plot != 0)
		{
			plot->Add(artifacts.ContaminatedData(), artifacts.MetaData());
		}
	}

#else
	void PlotAction::plotAntennaFlagCounts(ArtifactSet &)
	{}

	void PlotAction::plotFrequencyFlagCounts(ArtifactSet &)
	{}

	void PlotAction::plotFrequencyPower(ArtifactSet &)
	{}

	void PlotAction::plotTimeFlagCounts(ArtifactSet &)
	{}

	void PlotAction::plotSpectrumPerBaseline(ArtifactSet &)
	{}

	void PlotAction::plotPolarizationFlagCounts(ArtifactSet &)
	{}

	void PlotAction::plotBaselineRMS(ArtifactSet &)
	{}
	
	void PlotAction::plotIterations(class ArtifactSet &)
	{}
#endif
}
