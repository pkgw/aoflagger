/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include "baselineselector.h"

#include "../../util/aologger.h"
#include "../../util/plot.h"

#include "../../quality/defaultstatistics.h"

#include "thresholdtools.h"

namespace rfiStrategy
{

void BaselineSelector::Add(Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData)
{
	BaselineSelector::SingleBaselineInfo baseline;
	baseline.length = metaData->Baseline().Distance();
	if(baseline.length > 0)
	{
		baseline.antenna1 = metaData->Antenna1().id;
		baseline.antenna2 = metaData->Antenna2().id;
		baseline.antenna1Name = metaData->Antenna1().name;
		baseline.antenna2Name = metaData->Antenna2().name;
		baseline.band = metaData->Band().windowIndex;
		baseline.sequenceId = metaData->SequenceId();

		baseline.rfiCount = mask->GetCount<true>();
		baseline.totalCount = mask->Width() * mask->Height();

		_baselines.push_back(baseline);
	}
}

void BaselineSelector::Add(DefaultStatistics &baselineStat, AntennaInfo &antenna1, AntennaInfo &antenna2)
{
	if(antenna1.id != antenna2.id)
	{
		BaselineSelector::SingleBaselineInfo baseline;
		baseline.length = Baseline(antenna1, antenna2).Distance();
		baseline.antenna1 = antenna1.id;
		baseline.antenna2 = antenna2.id;
		baseline.antenna1Name = antenna1.name;
		baseline.antenna2Name = antenna2.name;
		baseline.band = 0;
		baseline.sequenceId = 0;
		
		const DefaultStatistics singleStat = baselineStat.ToSinglePolarization();
		
		baseline.rfiCount = singleStat.rfiCount[0];
		baseline.totalCount = singleStat.count[0] + singleStat.rfiCount[0];
		
		_baselines.push_back(baseline);
	}
}

void BaselineSelector::Search(std::vector<BaselineSelector::SingleBaselineInfo> &markedBaselines)
{
	// Perform a first quick threshold to remove baselines which deviate a lot (e.g. 100% flagged
	// baselines). Sometimes, there are a lot of them, causing instability if this would not be
	// done.
	for(int i=_baselines.size()-1;i>=0;--i)
	{
		double currentValue = (double) _baselines[i].rfiCount / (double) _baselines[i].totalCount;
		if(currentValue>_absThreshold || (_baselines[i].rfiCount==0 && _baselines[i].totalCount>=2500))
		{
			if(_useLog)
				AOLogger::Info << "Baseline " << _baselines[i].antenna1Name << " x " << _baselines[i].antenna2Name << " looks bad: "
			<< round(currentValue * 10000.0)/100.0 << "% rfi (zero or above " << (_absThreshold*100.0) << "% abs threshold)\n";
				
			_baselines[i].marked = true;
			markedBaselines.push_back(_baselines[i]);
			_baselines.erase(_baselines.begin()+i);
		}
	}

	bool foundMoreBaselines;
	do {
		std::sort(_baselines.begin(), _baselines.end());

		Plot *plot = 0;
		if(_makePlot)
		{
			plot = new Plot("baselineSelection.pdf");
			plot->SetXAxisText("Baseline length (meters)");
			plot->SetYAxisText("Percentage RFI");
		}

		size_t unmarkedBaselineCount = _baselines.size();
		double *values = new double[unmarkedBaselineCount];

		// Calculate the smoothed values
		if(_makePlot)
			plot->StartLine("Smoothed values");

		size_t valueIndex = 0;
		for(BaselineVector::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
		{
			double smoothedVal = smoothedValue(*i);
			if(_makePlot)
				plot->PushDataPoint(i->length, 100.0*smoothedVal);
			values[valueIndex] = smoothedVal - (double) i->rfiCount / (double) i->totalCount;
			++valueIndex;
		}

		// Calculate the std dev
		double mean, stddev;
		std::vector<double> valuesCopy;
		for(size_t i=0;i<unmarkedBaselineCount;++i)
			valuesCopy.push_back(values[i]);
		ThresholdTools::TrimmedMeanAndStdDev(valuesCopy, mean, stddev);

		if(_makePlot && _useLog)
			AOLogger::Debug << "Estimated std dev for thresholding, in percentage of RFI: " << round(10000.0*stddev)/100.0 << "%\n";

		// unselect already marked baselines
		for(int i=markedBaselines.size()-1;i>=0;--i)
		{
			BaselineSelector::SingleBaselineInfo baseline =
				markedBaselines[i];
			double currentValue = (double) baseline.rfiCount / (double) baseline.totalCount;
			double baselineValue =
				smoothedValue(baseline.length) - currentValue;
			if(baselineValue >= mean - _threshold*stddev && baselineValue <= mean + _threshold*stddev && currentValue<_absThreshold && (baseline.rfiCount!=0 || baseline.totalCount<2500))
			{
				markedBaselines.erase(markedBaselines.begin()+i);
				_baselines.push_back(baseline);
				if(_useLog)
					AOLogger::Info << "Baseline " << baseline.antenna1Name << " x " << baseline.antenna2Name << " is now within baseline curve\n";
			}
		}
		
		// (re)select baselines to be thrown away
		foundMoreBaselines = false;
		if(_makePlot)
			plot->StartScatter("Threshold");
		double maxPlotY = 0.0;
		for(int i=unmarkedBaselineCount-1;i>=0;--i)
		{
			double currentValue = (double) _baselines[i].rfiCount / (double) _baselines[i].totalCount;
			if(_makePlot)
			{
				double plotY = 100.0*(values[i] + currentValue + mean + _threshold*stddev);
				plot->PushDataPoint(_baselines[i].length, plotY);
				plot->PushDataPoint(_baselines[i].length, 100.0*(values[i] + currentValue + mean - _threshold*stddev));
				if(plotY > maxPlotY) maxPlotY=plotY;
			}
			if(values[i] < mean - _threshold*stddev || values[i] > mean + _threshold*stddev || currentValue>_absThreshold || (_baselines[i].rfiCount==0 && _baselines[i].totalCount>=2500))
			{
				if(_useLog)
					AOLogger::Info << "Baseline " << _baselines[i].antenna1Name << " x " << _baselines[i].antenna2Name << " looks bad: "
				<< round(currentValue * 10000.0)/100.0 << "% rfi, "
				<< round(10.0*fabs((values[i] - mean) / stddev))/10.0 << "*sigma away from est baseline curve\n";
					
				if(!_baselines[i].marked)
				{
					foundMoreBaselines = true;
					_baselines[i].marked = true;
				}
				markedBaselines.push_back(_baselines[i]);
				_baselines.erase(_baselines.begin()+i);
			}
		}
		if(_makePlot)
		{
			plot->SetYRange(0.0, maxPlotY*1.5);
			plot->StartScatter("Accepted baselines");
			for(BaselineVector::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
			{
				plot->PushDataPoint(i->length, 100.0 * (double) i->rfiCount / (double) i->totalCount);
			}
			plot->StartScatter("Rejected baselines");
			for(BaselineVector::const_iterator i=markedBaselines.begin();i!=markedBaselines.end();++i)
			{
				plot->PushDataPoint(i->length, 100.0 * (double) i->rfiCount / (double) i->totalCount);
			}
			plot->Close();
			delete plot;
		}

		delete[] values;
	} while(foundMoreBaselines);
}

void BaselineSelector::ImplyStations(const std::vector<BaselineSelector::SingleBaselineInfo> &markedBaselines, double maxRatio, std::set<unsigned> &badStations) const
{
	std::map<unsigned, unsigned> stations;
	for(std::vector<BaselineSelector::SingleBaselineInfo>::const_iterator i=markedBaselines.begin();i!=markedBaselines.end();++i)
	{
		stations[i->antenna1]++;
		stations[i->antenna2]++;
	}
	
	for(std::map<unsigned, unsigned>::const_iterator i=stations.begin();i!=stations.end();++i)
	{
		double ratio = (double) i->second / (double) stations.size();
		if(ratio > maxRatio)
		{
			badStations.insert(i->first);
		}
	}
}

double BaselineSelector::smoothedValue(double length) const
{
	double logLength = log(length);

	double sum = 0.0;
	double weight = 0.0;

	for(BaselineSelector::BaselineVector::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
	{
		double otherLogLength = log(i->length);
		double otherValue = (double) i->rfiCount / (double) i->totalCount;
		double x = otherLogLength-logLength;
		double curWeight = exp(-x*x/(2.0*_smoothingSigma*_smoothingSigma));
		sum += curWeight * otherValue;
		weight += curWeight;
	}

	return sum / weight;
}

}
