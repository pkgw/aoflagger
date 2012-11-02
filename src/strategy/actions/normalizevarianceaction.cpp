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
#include "normalizevarianceaction.h"

#include <map>

#include "../../util/progresslistener.h"

#include "../../quality/defaultstatistics.h"
#include "../../quality/statisticsderivator.h"
#include "../../quality/qualitytablesformatter.h"
#include "../../quality/statisticalvalue.h"
#include "../../quality/statisticscollection.h"

#include "../algorithms/medianwindow.h"
#include "../imagesets/msimageset.h"

namespace rfiStrategy {

NormalizeVarianceAction::~NormalizeVarianceAction()
{
}

void NormalizeVarianceAction::initializeStdDevs(ArtifactSet &artifacts)
{
	// The thread that calls this function first will initialize the
	// std dev. When a new measurement set is read, Initialize or Finalize
	// will be called, causing a clean().
	
	boost::mutex::scoped_lock lock(_mutex);
	if(!_isInitialized)
	{
		if(!artifacts.HasImageSet())
			throw std::runtime_error("Normalize variance called without image set");
		ImageSet *imageSet = artifacts.ImageSet();
		MSImageSet *msImageSet = dynamic_cast<MSImageSet*>(imageSet);
		if(msImageSet == 0)
			throw std::runtime_error("Normalize variance actions needs measurement set");
		std::string filename = msImageSet->Reader()->Set().Location();
		QualityTablesFormatter qtables(filename);
		StatisticsCollection statCollection(msImageSet->Reader()->Set().GetPolarizationCount());
		statCollection.LoadTimeStatisticsOnly(qtables);
		statCollection.IntegrateTimeToOneChannel();
		_isInitialized = true;
		
		// Calculate all stddevs
		const std::map<double, DefaultStatistics> &statMap = statCollection.TimeStatistics();
		_stddevs.clear();
		std::map<double, double>::iterator pos = _stddevs.begin();
		for(std::map<double, DefaultStatistics>::const_iterator i = statMap.begin();
				i != statMap.end(); ++i)
		{
			double stddev = StatisticsDerivator::GetStatisticAmplitude(
				QualityTablesFormatter::DStandardDeviationStatistic,
				i->second.ToSinglePolarization(), 0);
			pos = _stddevs.insert(pos, std::pair<double, double>(i->first, stddev));
		}
	}
}

void NormalizeVarianceAction::clean()
{
	boost::mutex::scoped_lock lock(_mutex);
	_isInitialized = false;
	_stddevs.clear(); // frees a bit of memory.
}

void NormalizeVarianceAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
{
	initializeStdDevs(artifacts);
	
	TimeFrequencyData &original = artifacts.OriginalData();
	const std::vector<double> &observationTimes = artifacts.MetaData()->ObservationTimes();
	size_t
		width = original.ImageWidth();
	
	std::vector<Image2DPtr> data;
	for(unsigned img=0;img<original.ImageCount();++img)
		data.push_back(Image2D::CreateCopy(original.GetImage(img)));
		
	// Add the first half of the window
	const double halfWindowTime = _medianFilterSizeInS * 0.5;
	MedianWindow<double> window;
	std::map<double, double>::const_iterator windowRightSideIterator = _stddevs.begin();
	const double startTime = windowRightSideIterator->first;
	do {
		if(std::isfinite(windowRightSideIterator->second))
			window.Add(windowRightSideIterator->second);
		++windowRightSideIterator;
	} while(windowRightSideIterator->first - startTime < halfWindowTime);
	
	// Add the second half, and start correcting the data
	size_t dataTimeIndex = 0;
	while(windowRightSideIterator != _stddevs.end() &&
		windowRightSideIterator->first - startTime < _medianFilterSizeInS)
	{
		correctDataUpTo(data, dataTimeIndex, windowRightSideIterator->first, observationTimes, window.Median());
		if(std::isfinite(windowRightSideIterator->second))
			window.Add(windowRightSideIterator->second);
		++windowRightSideIterator;
	}
	
	// Slide window until right side hits end
	std::map<double, double>::const_iterator windowLeftSideIterator = _stddevs.begin();
	const double endTime = _stddevs.rbegin()->first;
	while(windowRightSideIterator != _stddevs.end() && windowRightSideIterator->first < endTime)
	{
		correctDataUpTo(data, dataTimeIndex, windowRightSideIterator->first, observationTimes, window.Median());
		
		if(std::isfinite(windowRightSideIterator->second))
			window.Add(windowRightSideIterator->second);
		if(std::isfinite(windowLeftSideIterator->second))
			window.Remove(windowLeftSideIterator->second);
		
		++windowRightSideIterator;
		++windowLeftSideIterator;
	}
	
	// Slide until window center hits end
	while(windowLeftSideIterator != _stddevs.end() && windowLeftSideIterator->first + halfWindowTime < endTime)
	{
		correctDataUpTo(data, dataTimeIndex, windowLeftSideIterator->first + _medianFilterSizeInS, observationTimes, window.Median());
		if(std::isfinite(windowLeftSideIterator->second))
			window.Remove(windowLeftSideIterator->second);
		++windowLeftSideIterator;
	}
	
	while(dataTimeIndex < width)
	{
		correctData(data, dataTimeIndex, window.Median());
		++dataTimeIndex;
	}
	
	// Replace images
	for(unsigned img=0;img<original.ImageCount();++img)
		original.SetImage(img, data[img]);
}

void NormalizeVarianceAction::correctDataUpTo(std::vector<Image2DPtr> &data, size_t &dataTimeIndex, double rightSideTime, const std::vector<double> &observationTimes, double stddev)
{
	size_t width = (*data.begin())->Width();
	double halfWindowWidth = _medianFilterSizeInS*0.5;
	while(dataTimeIndex < width &&
		observationTimes[dataTimeIndex] + halfWindowWidth < rightSideTime)
	{
		correctData(data, dataTimeIndex, stddev);
		++dataTimeIndex;
	}
}

void NormalizeVarianceAction::correctData(std::vector<Image2DPtr> &data, size_t timeStep, double stddev)
{
	num_t oneOverStddev = 1.0 / stddev;
	
	for(std::vector<Image2DPtr>::iterator i=data.begin();i!=data.end();++i)
	{
		Image2DPtr image = *i;
		
		for(unsigned y=0;y<image->Height();++y)
			image->SetValue(timeStep, y, image->Value(timeStep, y) * oneOverStddev);
	}
}

} // end of namespace
