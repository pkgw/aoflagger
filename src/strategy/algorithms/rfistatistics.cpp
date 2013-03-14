/***************************************************************************
 *   Copyright (C) 2008-2010 by A.R. Offringa   *
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
#include "rfistatistics.h"

#include <deque>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

#include "../../msio/date.h"
#include "../../msio/timefrequencydata.h"

#include "../../util/aologger.h"
#include "../../util/plot.h"

#include "morphology.h"
#include <sys/stat.h>

void RFIStatistics::Add(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
{
	Mask2DCPtr mask = data.GetSingleMask();
	SegmentedImagePtr segmentedMask = SegmentedImage::CreatePtr(mask->Width(), mask->Height());
	Image2DCPtr image = data.GetSingleImage();
	
	SegmentedImagePtr classifiedMask;
	if(_performClassification)
	{
		Morphology morphology;
		morphology.SegmentByLengthRatio(mask, segmentedMask);
		classifiedMask = SegmentedImage::CreateCopy(segmentedMask);
		morphology.Classify(classifiedMask);
	} else {
		classifiedMask = segmentedMask;
	}
	
	addEverything(data, metaData, image, mask, segmentedMask, classifiedMask);
}

void RFIStatistics::addEverything(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask, SegmentedImagePtr segmentedMask, SegmentedImagePtr classifiedMask)
{
	addSingleBaseline(data, metaData, image, mask, segmentedMask, classifiedMask, _writeImmediately);
	
	boost::mutex::scoped_lock taLock(_baselineMapMutex);
	addBaselines(data, metaData, image, mask, segmentedMask, classifiedMask);
	if(_writeImmediately)
	{
		saveBaselines(_filePrefix + "counts-baselines.txt");
	}
}

void RFIStatistics::addSingleBaseline(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask, SegmentedImagePtr segmentedMask, SegmentedImagePtr classifiedMask, bool save)
{
	boost::mutex::scoped_lock taLock(_taMapMutex);
	addBaselineTimeInfo(metaData, image, mask);
	if(save) saveBaselineTimeInfo(_filePrefix + "counts-baseltime.txt");
	taLock.unlock();
	
	boost::mutex::scoped_lock afLock(_afMapMutex);
	addBaselineFrequencyInfo(metaData, image, mask);
	saveBaselineFrequencyInfo(_filePrefix + "counts-baselfreq.txt");
	afLock.unlock();
	
	bool isCross = (!metaData->HasAntenna1()) || (!metaData->HasAntenna2()) || metaData->Antenna1().id == metaData->Antenna2().id;
	if(isCross)
	{
		boost::mutex::scoped_lock genLock(_genericMutex);
		addFeatures(_autoAmplitudes, image, mask, metaData, segmentedMask);
		segmentedMask.reset();
		addAmplitudes(_autoAmplitudes, image, mask, metaData, classifiedMask);
		if(data.Polarisation() == DipolePolarisation && _polarizationAmplitudeStatistics)
		{
			addStokes(_autoAmplitudes, data, metaData);
			addPolarisations(_autoAmplitudes, data, metaData);
		}
		if(save) {
			saveAmplitudes(_autoAmplitudes, _filePrefix + "counts-amplitudes-auto.txt");
		}
		genLock.unlock();
		
		boost::mutex::scoped_lock freqLock(_frequencyMapMutex);
		addChannels(_autoChannels, image, mask, metaData, classifiedMask);
		if(save) saveChannels(_autoChannels, _filePrefix + "counts-channels-auto.txt");
		freqLock.unlock();
		
		boost::mutex::scoped_lock timeLock(_timeMapMutex);
		addTimesteps(_autoTimesteps, image, mask, metaData, classifiedMask);
		if(save) {
			saveTimesteps(_autoTimesteps, _filePrefix + "counts-timesteps-auto.txt");
			saveTimeIntegrated(_autoTimesteps, _filePrefix + "counts-timeint-auto.txt");
		}
		timeLock.unlock();
		
		boost::mutex::scoped_lock tfLock(_tfMapMutex);
		addTimeFrequencyInfo(_autoTimeFrequencyInfo, metaData, image, mask);
		if(save) saveTimeFrequencyInfo(_autoTimeFrequencyInfo, _filePrefix + "counts-timefreq-auto.txt");
		tfLock.unlock();
	} else {
		boost::mutex::scoped_lock genLock(_genericMutex);
		addFeatures(_crossAmplitudes, image, mask, metaData, segmentedMask);
		segmentedMask.reset();
		addAmplitudes(_crossAmplitudes, image, mask, metaData, classifiedMask);
		if(data.Polarisation() == DipolePolarisation && _polarizationAmplitudeStatistics)
		{
			addStokes(_crossAmplitudes, data, metaData);
			addPolarisations(_crossAmplitudes, data, metaData);
		}
		if(save) saveAmplitudes(_crossAmplitudes, _filePrefix + "counts-amplitudes-cross.txt");
		genLock.unlock();
		
		boost::mutex::scoped_lock freqLock(_frequencyMapMutex);
		addChannels(_crossChannels, image, mask, metaData, classifiedMask);
		if(save) saveChannels(_crossChannels, _filePrefix + "counts-channels-cross.txt");
		freqLock.unlock();
		
		boost::mutex::scoped_lock timeLock(_timeMapMutex);
		addTimesteps(_crossTimesteps, image, mask, metaData, classifiedMask);
		if(save) {
			saveTimesteps(_crossTimesteps, _filePrefix + "counts-timesteps-cross.txt");
			saveTimeIntegrated(_crossTimesteps, _filePrefix + "counts-timeint-cross.txt");
		}
		timeLock.unlock();
		
		boost::mutex::scoped_lock tfLock(_tfMapMutex);
		addTimeFrequencyInfo(_crossTimeFrequencyInfo, metaData, image, mask);
		if(save) saveTimeFrequencyInfo(_crossTimeFrequencyInfo, _filePrefix + "counts-timefreq-cross.txt");
		tfLock.unlock();
	}
}

void RFIStatistics::Add(const ChannelInfo &channel, bool autocorrelation)
{
	std::map<double, ChannelInfo> *channels;
	if(autocorrelation)
		channels = &_autoChannels;
	else
		channels = &_crossChannels;
	
	std::map<double, ChannelInfo>::iterator element = channels->find(channel.frequencyHz);
	if(element == channels->end())
	{
		channels->insert(std::pair<double, ChannelInfo>(channel.frequencyHz, channel));
	} else {
		ChannelInfo &c = element->second;
		c.totalCount += channel.totalCount;
		c.totalAmplitude += channel.totalAmplitude;
		c.rfiCount += channel.rfiCount;
		c.rfiAmplitude += channel.rfiAmplitude;
		c.broadbandRfiCount += channel.broadbandRfiCount;
		c.lineRfiCount += channel.lineRfiCount;
		c.broadbandRfiAmplitude += channel.broadbandRfiAmplitude;
		c.lineRfiAmplitude += channel.lineRfiAmplitude;
		c.falsePositiveCount += channel.falsePositiveCount;
		c.falseNegativeCount += channel.falseNegativeCount;
		c.truePositiveCount += channel.truePositiveCount;
		c.trueNegativeCount += channel.trueNegativeCount;
		c.falsePositiveAmplitude += channel.falsePositiveAmplitude;
		c.falseNegativeAmplitude += channel.falseNegativeAmplitude;
	}
}

void RFIStatistics::Add(const TimestepInfo &timestep, bool autocorrelation)
{
	std::map<double, TimestepInfo> *timesteps;
	if(autocorrelation)
		timesteps = &_autoTimesteps;
	else
		timesteps = &_crossTimesteps;
	
	std::map<double, TimestepInfo>::iterator element = timesteps->find(timestep.time);
	if(element == timesteps->end())
	{
		timesteps->insert(std::pair<double, TimestepInfo>(timestep.time, timestep));
	} else {
		TimestepInfo &t = element->second;
		t.totalCount += timestep.totalCount;
		t.totalAmplitude += timestep.totalAmplitude;
		t.rfiCount += timestep.rfiCount;
		t.rfiAmplitude += timestep.rfiAmplitude;
		t.broadbandRfiCount += timestep.broadbandRfiCount;
		t.lineRfiCount += timestep.lineRfiCount;
		t.broadbandRfiAmplitude += timestep.broadbandRfiAmplitude;
		t.lineRfiAmplitude += timestep.lineRfiAmplitude;
	}
}

void RFIStatistics::Add(const AmplitudeBin &amplitudeBin, bool autocorrelation)
{
	std::map<double, AmplitudeBin> *amplitudes;
	if(autocorrelation)
		amplitudes = &_autoAmplitudes;
	else
		amplitudes = &_crossAmplitudes;
	
	std::map<double, AmplitudeBin>::iterator element = amplitudes->find(amplitudeBin.centralAmplitude);
	if(element == amplitudes->end())
	{
		amplitudes->insert(std::pair<double, AmplitudeBin>(amplitudeBin.centralAmplitude, amplitudeBin));
	} else {
		AmplitudeBin &a = element->second;
		a.count += amplitudeBin.count;
		a.rfiCount += amplitudeBin.rfiCount;
		a.broadbandRfiCount += amplitudeBin.broadbandRfiCount;
		a.lineRfiCount += amplitudeBin.lineRfiCount;
		a.featureAvgCount += amplitudeBin.featureAvgCount;
		a.featureMaxCount += amplitudeBin.featureMaxCount;
		a.featureIntCount += amplitudeBin.featureIntCount;
		a.xxCount += amplitudeBin.xxCount;
		a.xyCount += amplitudeBin.xyCount;
		a.yxCount += amplitudeBin.yxCount;
		a.yyCount += amplitudeBin.yyCount;
		a.xxRfiCount += amplitudeBin.xxRfiCount;
		a.xyRfiCount += amplitudeBin.xyRfiCount;
		a.yxRfiCount += amplitudeBin.yxRfiCount;
		a.yyRfiCount += amplitudeBin.yyRfiCount;
		a.falsePositiveCount += amplitudeBin.falsePositiveCount;
		a.falseNegativeCount += amplitudeBin.falseNegativeCount;
		a.truePositiveCount += amplitudeBin.truePositiveCount;
		a.trueNegativeCount += amplitudeBin.trueNegativeCount;
	}
}

void RFIStatistics::Add(const BaselineInfo &baseline)
{
	BaselineMatrix::iterator rowElement = _baselines.find(baseline.antenna1);
	if(rowElement == _baselines.end())
	{
		_baselines.insert(BaselineMatrix::value_type(baseline.antenna1, std::map<int, BaselineInfo>()));
		rowElement = _baselines.find(baseline.antenna1);
	}
	
	std::map<int, BaselineInfo> &row = rowElement->second;
	std::map<int, BaselineInfo>::iterator element = row.find(baseline.antenna2);
	if(element == row.end())
	{
		row.insert(std::pair<int, BaselineInfo>(baseline.antenna2, baseline));
	} else {
		BaselineInfo &b = element->second;
		b.count += baseline.count;
		b.totalAmplitude += baseline.totalAmplitude;
		b.rfiCount += baseline.rfiCount;
		b.rfiAmplitude += baseline.rfiAmplitude;
		b.broadbandRfiCount += baseline.broadbandRfiCount;
		b.lineRfiCount += baseline.lineRfiCount;
		b.broadbandRfiAmplitude += baseline.broadbandRfiAmplitude;
		b.lineRfiAmplitude += baseline.lineRfiAmplitude;
	}
}

void RFIStatistics::Add(const BaselineFrequencyInfo &entry)
{
	IndexTriple index;
	index.antenna1Index = entry.antenna1Index;
	index.antenna2Index = entry.antenna2Index;
	index.thirdIndex = entry.centralFrequency;
	BaselineFrequencyInfoMap::iterator element = _baselineFrequencyInfo.find(index);
	if(element == _baselineFrequencyInfo.end())
	{
		_baselineFrequencyInfo.insert(std::pair<IndexTriple, BaselineFrequencyInfo>(index, entry));
	} else {
		BaselineFrequencyInfo &info = element->second;
		info.totalCount += entry.totalCount;
		info.rfiCount += entry.rfiCount;
	}
}

void RFIStatistics::Add(const BaselineTimeInfo &entry)
{
	IndexTriple index;
	index.antenna1Index = entry.antenna1Index;
	index.antenna2Index = entry.antenna2Index;
	index.thirdIndex = entry.time;
	BaselineTimeInfoMap::iterator element = _baselineTimeInfo.find(index);
	if(element == _baselineTimeInfo.end())
	{
		_baselineTimeInfo.insert(std::pair<IndexTriple, BaselineTimeInfo>(index, entry));
	} else {
		BaselineTimeInfo &info = element->second;
		info.totalCount += entry.totalCount;
		info.rfiCount += entry.rfiCount;
	}
}

void RFIStatistics::Add(const TimeFrequencyInfo &entry, bool autocorrelation)
{
	TimeFrequencyInfoMap *timeFrequencyInfo;
	if(autocorrelation)
		timeFrequencyInfo = &_autoTimeFrequencyInfo;
	else
		timeFrequencyInfo = &_crossTimeFrequencyInfo;
	
	std::pair<double, double> index;
	index.first = entry.time;
	index.second = entry.centralFrequency;
	TimeFrequencyInfoMap::iterator element = timeFrequencyInfo->find(index);
	if(element == timeFrequencyInfo->end())
	{
		timeFrequencyInfo->insert(std::pair<std::pair<double, double>, TimeFrequencyInfo>(index, entry));
	} else {
		TimeFrequencyInfo &info = element->second;
		info.totalCount += entry.totalCount;
		info.rfiCount += entry.rfiCount;
		info.totalAmplitude += entry.totalAmplitude;
		info.rfiAmplitude += entry.rfiAmplitude;
	}
}

void RFIStatistics::addChannels(std::map<double, class ChannelInfo> &channels, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	for(size_t y=startChannel(image->Height());y<image->Height();++y)
	{
		long unsigned count = 0;
		long double totalAmplitude = 0.0;
		long unsigned rfiCount = 0;
		long double rfiAmplitude = 0.0;
		long unsigned broadbandRfiCount = 0;
		long unsigned lineRfiCount = 0;
		long double broadbandRfiAmplitude = 0.0;
		long double lineRfiAmplitude = 0.0;
		
		for(size_t x=0;x<image->Width();++x)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				totalAmplitude += image->Value(x, y);
				++count;
				if(mask->Value(x, y))
				{
					++rfiCount;
					rfiAmplitude += image->Value(x, y);
					if(segmentedImage->Value(x, y) == Morphology::BROADBAND_SEGMENT)
					{
						++broadbandRfiCount;
						broadbandRfiAmplitude += image->Value(x, y);
					} else if(segmentedImage->Value(x, y) == Morphology::LINE_SEGMENT)
					{
						++lineRfiCount;
						lineRfiAmplitude += image->Value(x, y);
					}
				}
			}
		}
		if(channels.count(metaData->Band().channels[y].frequencyHz) == 0)
		{
			ChannelInfo channel(metaData->Band().channels[y].frequencyHz);
			channel.totalCount = count;
			channel.totalAmplitude = totalAmplitude;
			channel.rfiCount = rfiCount;
			channel.rfiAmplitude = rfiAmplitude;
			channel.broadbandRfiCount = broadbandRfiCount;
			channel.lineRfiCount = lineRfiCount;
			channel.broadbandRfiAmplitude = broadbandRfiAmplitude;
			channel.lineRfiAmplitude = lineRfiAmplitude;
			channels.insert(std::pair<double, ChannelInfo>(channel.frequencyHz, channel));
		} else {
			ChannelInfo &channel = channels.find(metaData->Band().channels[y].frequencyHz)->second;
			channel.totalCount += count;
			channel.totalAmplitude += totalAmplitude;
			channel.rfiCount += rfiCount;
			channel.rfiAmplitude += rfiAmplitude;
			channel.broadbandRfiCount += broadbandRfiCount;
			channel.lineRfiCount += lineRfiCount;
			channel.broadbandRfiAmplitude += broadbandRfiAmplitude;
			channel.lineRfiAmplitude += lineRfiAmplitude;
		}
	}
}

void RFIStatistics::addTimesteps(std::map<double, class TimestepInfo> &timesteps, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	for(size_t x=0;x<image->Width();++x)
	{
		long unsigned totalCount = 0;
		long double totalAmplitude = 0.0;
		long unsigned rfiCount = 0;
		long double rfiAmplitude = 0.0;
		long unsigned broadbandRfiCount = 0;
		long unsigned lineRfiCount = 0;
		long double broadbandRfiAmplitude = 0.0;
		long double lineRfiAmplitude = 0.0;
		
		for(size_t y=startChannel(image->Height());y<image->Height();++y)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				++totalCount;
				totalAmplitude += image->Value(x, y);

				if(mask->Value(x, y))
				{
					++rfiCount;
					rfiAmplitude += image->Value(x, y);
					if(segmentedImage->Value(x, y) == Morphology::BROADBAND_SEGMENT)
					{
						++broadbandRfiCount;
						broadbandRfiAmplitude += image->Value(x, y);
					} else if(segmentedImage->Value(x, y) == Morphology::LINE_SEGMENT)
					{
						++lineRfiCount;
						lineRfiAmplitude += image->Value(x, y);
					}
				}
			}
		}
		if(timesteps.count(metaData->ObservationTimes()[x]) == 0)
		{
			TimestepInfo timestep(metaData->ObservationTimes()[x]);
			timestep.totalCount = totalCount;
			timestep.totalAmplitude = totalAmplitude;
			timestep.rfiCount = rfiCount;
			timestep.rfiAmplitude = rfiAmplitude;
			timestep.broadbandRfiCount = broadbandRfiCount;
			timestep.lineRfiCount = lineRfiCount;
			timestep.broadbandRfiAmplitude = broadbandRfiAmplitude;
			timestep.lineRfiAmplitude = lineRfiAmplitude;
			timesteps.insert(std::pair<double, TimestepInfo>(timestep.time, timestep));
		} else {
			TimestepInfo &timestep = timesteps.find(metaData->ObservationTimes()[x])->second;
			timestep.totalCount += totalCount;
			timestep.totalAmplitude += totalAmplitude;
			timestep.rfiCount += rfiCount;
			timestep.rfiAmplitude += rfiAmplitude;
			timestep.broadbandRfiCount += broadbandRfiCount;
			timestep.lineRfiCount += lineRfiCount;
			timestep.broadbandRfiAmplitude += broadbandRfiAmplitude;
			timestep.lineRfiAmplitude += lineRfiAmplitude;
		}
	}
}

void RFIStatistics::addAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr, SegmentedImageCPtr segmentedImage)
{
	for(size_t y=startChannel(image->Height());y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			double amp = image->Value(x, y);
			if(std::isfinite(amp))
			{
				double centralAmp = getCentralAmplitude(amp);
				std::map<double, class AmplitudeBin>::iterator element =
					amplitudes.find(centralAmp);
				
				AmplitudeBin bin;
				if(element == amplitudes.end())
				{
					bin.centralAmplitude = centralAmp;
				} else {
					bin = element->second;
				}
				++bin.count;
				if(mask->Value(x, y))
				{
					++bin.rfiCount;
					if(segmentedImage->Value(x, y) == Morphology::BROADBAND_SEGMENT)
					{
						++bin.broadbandRfiCount;
					} else if(segmentedImage->Value(x, y) == Morphology::LINE_SEGMENT)
					{
						++bin.lineRfiCount;
					}
				}
				if(element == amplitudes.end())
					amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmp, bin));
				else
					element->second = bin;
			}
		}
	}
}

void RFIStatistics::addStokes(std::map<double, class AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr)
{
	for(unsigned i=0;i<3;++i)
	{
		TimeFrequencyData *stokes;
		switch(i)
		{
			case 0: stokes = data.CreateTFData(StokesQPolarisation); break;
			case 1: stokes = data.CreateTFData(StokesUPolarisation); break;
			case 2: stokes = data.CreateTFData(StokesVPolarisation); break;
			default: stokes = 0; break;
		}
		Image2DCPtr image = stokes->GetSingleImage();
		delete stokes;
		for(size_t y=startChannel(image->Height());y<image->Height();++y)
		{
			for(size_t x=0;x<image->Width();++x)
			{
				double amp = image->Value(x, y);
				if(std::isfinite(amp))
				{
					double centralAmp = getCentralAmplitude(amp);
					std::map<double, class AmplitudeBin>::iterator element =
						amplitudes.find(centralAmp);
					
					AmplitudeBin bin;
					if(element == amplitudes.end())
					{
						bin.centralAmplitude = centralAmp;
					} else {
						bin = element->second;
					}
					switch(i)
					{
						case 0: ++bin.stokesQCount; break;
						case 1: ++bin.stokesUCount; break;
						case 2: ++bin.stokesVCount; break;
					}
					if(element == amplitudes.end())
						amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmp, bin));
					else
						element->second = bin;
				}
			}
		}
	}
}

void RFIStatistics::addPolarisations(std::map<double, class AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr)
{
	for(size_t polIndex=0;polIndex<data.PolarisationCount();++polIndex)
	{
		TimeFrequencyData *polData = data.CreateTFDataFromPolarisationIndex(polIndex);
		Image2DCPtr image = polData->GetSingleImage();
		Mask2DCPtr mask = polData->GetSingleMask();
		delete polData;
		
		for(size_t y=startChannel(image->Height());y<image->Height();++y)
		{
			for(size_t x=0;x<image->Width();++x)
			{
				double amp = image->Value(x, y);
				if(std::isfinite(amp))
				{
					double centralAmp = getCentralAmplitude(amp);
					std::map<double, class AmplitudeBin>::iterator element =
						amplitudes.find(centralAmp);
					
					AmplitudeBin bin;
					if(element == amplitudes.end())
					{
						bin.centralAmplitude = centralAmp;
					} else {
						bin = element->second;
					}
					switch(polIndex)
					{
						case 0: ++bin.xxCount; break;
						case 1: ++bin.xyCount; break;
						case 2: ++bin.yxCount; break;
						case 3: ++bin.yyCount; break;
					}
					if(mask->Value(x, y))
					{
						switch(polIndex)
						{
							case 0: ++bin.xxRfiCount; break;
							case 1: ++bin.xyRfiCount; break;
							case 2: ++bin.yxRfiCount; break;
							case 3: ++bin.yyRfiCount; break;
						}
					}
					if(element == amplitudes.end())
						amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmp, bin));
					else
						element->second = bin;
				}
			}
		}
	}
}

void RFIStatistics::addBaselines(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask, SegmentedImagePtr segmentedMask, SegmentedImagePtr classifiedMask)
{
	long unsigned count = 0;
	long unsigned rfiCount = 0;
	long double totalAmplitude = 0.0;
	long double rfiAmplitude = 0.0;
	long unsigned broadbandRfiCount = 0;
	long unsigned lineRfiCount = 0;
	long double broadbandRfiAmplitude = 0.0;
	long double lineRfiAmplitude = 0.0;

	for(size_t y=startChannel(image->Height());y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				++count;
				totalAmplitude += image->Value(x, y);
				if(mask->Value(x, y))
				{
					++rfiCount;
					rfiAmplitude += image->Value(x, y);
					if(classifiedMask->Value(x, y) == Morphology::BROADBAND_SEGMENT)
					{
						++broadbandRfiCount;
						broadbandRfiAmplitude += image->Value(x, y);
					} else if(classifiedMask->Value(x, y) == Morphology::LINE_SEGMENT)
					{
						++lineRfiCount;
						lineRfiAmplitude += image->Value(x, y);
					}
				}
			}
		}
	}
	int a1, a2;
	if(metaData->HasAntenna1())
		a1 = metaData->Antenna1().id;
	else
		a1 = 0;
	if(metaData->HasAntenna2())
		a2 = metaData->Antenna2().id;
	else
		a2 = 0;
	if(_baselines.count(a1) == 0)
		_baselines.insert(BaselineMatrix::value_type(a1, std::map<int, BaselineInfo>() ));
	BaselineMatrix::mapped_type &row = _baselines.find(a1)->second;
	
	Baseline baselineMSData;
	if(metaData->HasAntenna1() && metaData->HasAntenna2())
		baselineMSData = Baseline(metaData->Antenna1(), metaData->Antenna2());

	if(row.count(a2) == 0)
	{
		BaselineInfo baseline;
		baseline.antenna1 = a1;
		baseline.antenna2 = a2;
		if(metaData->HasAntenna1())
			baseline.antenna1Name = metaData->Antenna1().name;
		if(metaData->HasAntenna2())
			baseline.antenna2Name = metaData->Antenna2().name;
		baseline.baselineLength = baselineMSData.Distance();
		baseline.baselineAngle = baselineMSData.Angle();

		baseline.count = count;
		baseline.totalAmplitude = totalAmplitude;
		baseline.rfiCount = rfiCount;
		baseline.rfiAmplitude = rfiAmplitude;
		baseline.broadbandRfiCount = broadbandRfiCount;
		baseline.lineRfiCount = lineRfiCount;
		baseline.broadbandRfiAmplitude = broadbandRfiAmplitude;
		baseline.lineRfiAmplitude = lineRfiAmplitude;
		if(_separateBaselineStatistics)
		{
			baseline.baselineStatistics = new RFIStatistics();
			baseline.baselineStatistics->addSingleBaseline(data, metaData, image, mask, segmentedMask, classifiedMask, false);
		}
		row.insert(std::pair<int, BaselineInfo>(a2, baseline));
	} else {
		BaselineInfo &baseline = row.find(a2)->second;
		baseline.count += count;
		baseline.totalAmplitude += totalAmplitude;
		baseline.rfiCount += rfiCount;
		baseline.rfiAmplitude += rfiAmplitude;
		baseline.broadbandRfiCount += broadbandRfiCount;
		baseline.lineRfiCount += lineRfiCount;
		baseline.broadbandRfiAmplitude += broadbandRfiAmplitude;
		baseline.lineRfiAmplitude += lineRfiAmplitude;
		if(_separateBaselineStatistics)
		{
			baseline.baselineStatistics->addSingleBaseline(data, metaData, image, mask, segmentedMask, classifiedMask, false);
		}
	}
}

void RFIStatistics::addFeatures(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr, SegmentedImageCPtr segmentedImage)
{
	FeatureMap features;
	
	for(size_t y=startChannel(image->Height());y<image->Height();++y) {
		for(size_t x=0;x<image->Width();++x) {
			if(mask->Value(x, y) && std::isfinite(image->Value(x, y)))
			{
				FeatureMap::iterator i = features.find(segmentedImage->Value(x, y));
				if(i == features.end()) {
					FeatureInfo newFeature;
					newFeature.amplitudeSum = image->Value(x, y);
					newFeature.amplitudeMax = image->Value(x, y);
					newFeature.sampleCount = 1;
					features.insert(FeatureMap::value_type(segmentedImage->Value(x, y), newFeature));
				} else {
					FeatureInfo &feature = i->second;
					num_t sampleValue = image->Value(x, y);
					feature.amplitudeSum += sampleValue;
					if(sampleValue > feature.amplitudeMax)
						feature.amplitudeMax = sampleValue;
					feature.sampleCount++;
				}
			}
		}
	}
	for(FeatureMap::const_iterator i=features.begin();i!=features.end();++i)
	{
		const FeatureInfo &feature = i->second;
		double intBin = getCentralAmplitude(feature.amplitudeSum);
		double avgBin = getCentralAmplitude(feature.amplitudeSum / feature.sampleCount);
		double maxBin = getCentralAmplitude(feature.amplitudeMax);
		if(amplitudes.count(intBin) == 0)
		{
			AmplitudeBin bin;
			bin.centralAmplitude = intBin;
			bin.featureIntCount=1;
			amplitudes.insert(std::pair<double, AmplitudeBin>(intBin, bin));
		} else {
			amplitudes[intBin].featureIntCount++;
		}
		if(amplitudes.count(avgBin) == 0)
		{
			AmplitudeBin bin;
			bin.centralAmplitude = avgBin;
			bin.featureAvgCount=1;
			amplitudes.insert(std::pair<double, AmplitudeBin>(avgBin, bin));
		} else {
			amplitudes[avgBin].featureAvgCount++;
		}
		if(amplitudes.count(maxBin) == 0)
		{
			AmplitudeBin bin;
			bin.centralAmplitude = maxBin;
			bin.featureMaxCount=1;
			amplitudes.insert(std::pair<double, AmplitudeBin>(maxBin, bin));
		} else {
			amplitudes[maxBin].featureMaxCount++;
		}
	}
}

void RFIStatistics::addChannelComparison(std::map<double, ChannelInfo> &channels, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Mask2DCPtr groundTruthFlagging)
{
	Image2DCPtr image = data.GetSingleImage();
	Mask2DCPtr mask = data.GetSingleMask();

	for(size_t y=startChannel(image->Height());y<image->Height();++y)
	{
		long unsigned falsePositiveCount = 0;
		long unsigned falseNegativeCount = 0;
		long unsigned truePositiveCount = 0;
		long unsigned trueNegativeCount = 0;
		long double falsePositiveAmplitude = 0;
		long double falseNegativeAmplitude = 0;
		
		for(size_t x=0;x<image->Width();++x)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				if(mask->Value(x,y) && groundTruthFlagging->Value(x,y))
					++truePositiveCount;
				else if(!mask->Value(x,y) && !groundTruthFlagging->Value(x,y))
					++trueNegativeCount;
				else if(mask->Value(x,y) && !groundTruthFlagging->Value(x,y))
				{
					++falsePositiveCount;
					falsePositiveAmplitude += image->Value(x, y);
				}
				else // !mask->Value(x,y) && groundTruthFlagging->Value(x,y)
				{
					++falseNegativeCount;
					falseNegativeAmplitude += image->Value(x, y);
				}
			}
		}
		ChannelInfo &channel = channels.find(metaData->Band().channels[y].frequencyHz)->second;
		channel.falsePositiveCount += falsePositiveCount;
		channel.falseNegativeCount += falseNegativeCount;
		channel.truePositiveCount += truePositiveCount;
		channel.trueNegativeCount += trueNegativeCount;
		channel.falsePositiveAmplitude += falsePositiveAmplitude;
		channel.falseNegativeAmplitude += falseNegativeAmplitude;
	}
}

void RFIStatistics::addAmplitudeComparison(std::map<double, AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr, Mask2DCPtr groundTruthFlagging)
{
	Image2DCPtr image = data.GetSingleImage();
	Mask2DCPtr mask = data.GetSingleMask();

	for(size_t y=startChannel(image->Height());y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			double amp = image->Value(x, y);
			if(std::isfinite(amp))
			{
				double centralAmp = getCentralAmplitude(amp);
				std::map<double, class AmplitudeBin>::iterator element =
					amplitudes.find(centralAmp);
				
				AmplitudeBin bin;
				if(element == amplitudes.end())
				{
					bin.centralAmplitude = centralAmp;
				} else {
					bin = element->second;
				}
				if(mask->Value(x,y) && groundTruthFlagging->Value(x,y))
					++bin.truePositiveCount;
				else if(!mask->Value(x,y) && !groundTruthFlagging->Value(x,y))
					++bin.trueNegativeCount;
				else if(mask->Value(x,y) && !groundTruthFlagging->Value(x,y))
					++bin.falsePositiveCount;
				else // !mask->Value(x,y) && groundTruthFlagging->Value(x,y)
					++bin.falseNegativeCount;
				if(element == amplitudes.end())
					amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmp, bin));
				else
					amplitudes.find(centralAmp)->second = bin;
			}
		}
	}
}

void RFIStatistics::addBaselineFrequencyInfo(TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask)
{
	IndexTriple index;
	if(metaData->HasAntenna1())
		index.antenna1Index = metaData->Antenna1().id;
	else
		index.antenna1Index = 0;
	if(metaData->HasAntenna2())
		index.antenna2Index = metaData->Antenna2().id;
	else
		index.antenna2Index = 0;
	index.thirdIndex = (metaData->Band().channels.begin()->frequencyHz + metaData->Band().channels.rbegin()->frequencyHz) / 2.0;
	std::map<IndexTriple, BaselineFrequencyInfo>::iterator element = _baselineFrequencyInfo.find(index);
	if(element == _baselineFrequencyInfo.end())
	{
		BaselineFrequencyInfo newInfo;
		newInfo.centralFrequency = index.thirdIndex;
		newInfo.antenna1Index = index.antenna1Index;
		newInfo.antenna2Index = index.antenna2Index;
		element = _baselineFrequencyInfo.insert(std::pair<IndexTriple, BaselineFrequencyInfo>(index, newInfo)).first;
	}
	for(size_t y=startChannel(image->Height());y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				++element->second.totalCount;
				if(mask->Value(x, y))
					++element->second.rfiCount;
			}
		}
	}
}

void RFIStatistics::addBaselineTimeInfo(TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask)
{
	IndexTriple index;
	if(metaData->HasAntenna1())
		index.antenna1Index = metaData->Antenna1().id;
	else
		index.antenna1Index = 0;
	if(metaData->HasAntenna2())
		index.antenna2Index = metaData->Antenna2().id;
	else
		index.antenna2Index = 0;
	//double timeStart = metaData->ObservationTimes()[0];
	//double duration = metaData->ObservationTimes()[image->Width()-1] - timeStart;
	for(size_t x=0;x<image->Width();++x)
	{
		double time = metaData->ObservationTimes()[x];
		//double timePos = timeStart + round((double) x * 1000.0 / (double) image->Width()) / 1000.0 * duration;
		index.thirdIndex = time;
		std::map<IndexTriple, BaselineTimeInfo>::iterator element = _baselineTimeInfo.find(index);
		if(element == _baselineTimeInfo.end())
		{
			BaselineTimeInfo newInfo;
			newInfo.time = index.thirdIndex;
			newInfo.antenna1Index = index.antenna1Index;
			newInfo.antenna2Index = index.antenna2Index;
			element = _baselineTimeInfo.insert(std::pair<IndexTriple, BaselineTimeInfo>(index, newInfo)).first;
		}
		for(size_t y=startChannel(image->Height());y<image->Height();++y)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				++element->second.totalCount;
				if(mask->Value(x, y))
					++element->second.rfiCount;
			}
		}
	}
}

void RFIStatistics::addTimeFrequencyInfo(TimeFrequencyInfoMap &map, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask)
{
	double centralFrequency = (metaData->Band().channels.begin()->frequencyHz + metaData->Band().channels.rbegin()->frequencyHz) / 2.0;
	
	for(size_t x=0;x<image->Width();++x)
	{
		double time = metaData->ObservationTimes()[x];
		std::pair<double, double> index(time, centralFrequency);
		
		TimeFrequencyInfoMap::iterator element = map.find(index);
		if(element == map.end())
		{
			TimeFrequencyInfo newInfo;
			newInfo.time = index.first;
			newInfo.centralFrequency = index.second;
			element = map.insert(std::pair<std::pair<double, double>, TimeFrequencyInfo>(index, newInfo)).first;
		}
		for(size_t y=startChannel(image->Height());y<image->Height();++y)
		{
			const num_t amplitude = image->Value(x, y);
			if(std::isfinite(amplitude))
			{
				++element->second.totalCount;
				element->second.totalAmplitude += amplitude;
				if(mask->Value(x, y))
				{
					++element->second.rfiCount;
					element->second.rfiAmplitude += amplitude;
				}
			}
		}
	}
}

void RFIStatistics::saveChannels(const std::map<double, ChannelInfo> &channels, const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file << "frequency\ttotalCount\ttotalAmplitude\trfiCount\trfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\tFalse P\tFalse N\tTrue P\tTrue N\tFalse P amp\tFalse N amp\n" << std::setprecision(14);
	for(std::map<double, class ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
	{
		const ChannelInfo &c = i->second;
		file
			<< c.frequencyHz << "\t"
			<< c.totalCount << "\t"
			<< c.totalAmplitude << "\t"
			<< c.rfiCount << "\t"
			<< c.rfiAmplitude << "\t"
			<< c.broadbandRfiCount << "\t"
			<< c.lineRfiCount << "\t"
			<< c.broadbandRfiAmplitude << "\t"
			<< c.lineRfiAmplitude << "\t"
			<< c.falsePositiveCount << "\t"
			<< c.falseNegativeCount << "\t"
			<< c.truePositiveCount << "\t"
			<< c.trueNegativeCount << "\t"
			<< c.falsePositiveAmplitude << "\t"
			<< c.falseNegativeAmplitude
			<< "\n";
	}
	file.close();
}

void RFIStatistics::saveTimesteps(const std::map<double, class TimestepInfo> &timesteps, const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file << "timestep\ttotalCount\ttotalAmplitude\trfiCount\trfiAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\n" << std::setprecision(14);
	for(std::map<double, class TimestepInfo>::const_iterator i=timesteps.begin();i!=timesteps.end();++i)
	{
		const TimestepInfo &c = i->second;
		file
			<< c.time << "\t"
			<< c.totalCount << "\t"
			<< c.totalAmplitude << "\t"
			<< c.rfiCount << "\t"
			<< c.rfiAmplitude << "\t"
			<< c.broadbandRfiCount << "\t"
			<< c.lineRfiCount << "\t"
			<< c.broadbandRfiAmplitude << "\t"
			<< c.lineRfiAmplitude << "\n";
	}
	file.close();
}

void RFIStatistics::saveSubbands(const std::map<double, class ChannelInfo> &channels, const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file <<
		"subband\ts-frequency\te-frequency\ttotalCount\ttotalAmplitude\trfiCount\t"
		"rfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\t"
		"False P\tFalse N\tTrue P\tTrue N\tFalse P amp\tFalse N amp\t"
		"totalCountLQ\ttotalCountUQ\ttotalAmplitudeLQ\ttotalAmplitudeUQ\trfiCountLQ\trfiCountUQ\t"
		"rfiSummedAmplitudeLQ\trfiSummedAmplitudeUQ\tbroadbandRfiCountLQ\tbroadbandRfiCountUQ\t"
		"lineRfiCountLQ\tlineRfiCountUQ\tbroadbandRfiAmplitudeLQ\tbroadbandRfiAmplitudeUQ\t"
		"lineRfiAmplitudeLQ\tlineRfiAmplitudeUQ\n"
	<< std::setprecision(14);
	size_t index = 0;
	std::multiset<double>
		bandTotals,
		bandAmps,
		bandRFIs,
		bandRFIAmps,
		bandBRFIs,
		bandLRFIs,
		bandBRFIAmps,
		bandLRFIAmps,
		bandFP,
		bandFN,
		bandTP,
		bandTN,
		bandFPAmps,
		bandFNAmps;
	unsigned countPerSubband = _channelCountPerSubband;
	if(_ignoreFirstChannel) --countPerSubband;
	for(std::map<double, class ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
	{
		const ChannelInfo &c = i->second;
		bandTotals.insert(c.totalCount);
		bandAmps.insert(c.totalAmplitude);
		bandRFIs.insert(c.rfiCount);
		bandRFIAmps.insert(c.rfiAmplitude);
		bandBRFIs.insert(c.broadbandRfiCount);
		bandLRFIs.insert(c.lineRfiCount);
		bandBRFIAmps.insert(c.broadbandRfiAmplitude);
		bandLRFIAmps.insert(c.lineRfiAmplitude);
		bandFP.insert(c.falsePositiveCount);
		bandFN.insert(c.falseNegativeCount);
		bandTP.insert(c.truePositiveCount);
		bandTN.insert(c.trueNegativeCount);
		bandFPAmps.insert(c.falsePositiveAmplitude);
		bandFNAmps.insert(c.falseNegativeAmplitude);
		if(index%countPerSubband == 0)
			file << index/countPerSubband << '\t' << c.frequencyHz << '\t';
		if(index%countPerSubband == (countPerSubband-1))
		{
			file
			<< c.frequencyHz << "\t"
			<< avg(bandTotals) << "\t" // 4
			<< avg(bandAmps) << "\t"
			<< avg(bandRFIs) << "\t"
			<< avg(bandRFIAmps) << "\t"
			<< avg(bandBRFIs) << "\t"
			<< avg(bandLRFIs) << "\t"
			<< avg(bandBRFIAmps) << "\t"
			<< avg(bandLRFIAmps) << "\t"
			<< avg(bandFP) << "\t"
			<< avg(bandFN) << "\t"
			<< avg(bandTP) << "\t"
			<< avg(bandTN) << "\t"
			<< avg(bandFPAmps) << "\t"
			<< avg(bandFNAmps) << "\t" // 17
			<< lowerQuartile(bandTotals) << "\t" << median(bandTotals) << "\t" << upperQuartile(bandTotals) << "\t"
			<< lowerQuartile(bandAmps) << "\t" << median(bandAmps) << "\t" << upperQuartile(bandAmps) << "\t"
			<< lowerQuartile(bandRFIs) << "\t" << median(bandRFIs) << "\t" << upperQuartile(bandRFIs) << "\t"
			<< lowerQuartile(bandRFIAmps) << "\t" << median(bandRFIAmps) << "\t" << upperQuartile(bandRFIAmps) << "\t"
			<< lowerQuartile(bandBRFIs) << "\t" << median(bandBRFIs) << "\t" << upperQuartile(bandBRFIs) << "\t"
			<< lowerQuartile(bandLRFIs) << "\t" << median(bandLRFIs) << "\t" << upperQuartile(bandLRFIs) << "\t"
			<< lowerQuartile(bandBRFIAmps) << "\t" << median(bandBRFIAmps) << "\t" << upperQuartile(bandBRFIAmps) << "\t"
			<< lowerQuartile(bandLRFIAmps) << "\t" << median(bandLRFIAmps) << "\t" << upperQuartile(bandLRFIAmps) << "\t"
			<< lowerQuartile(bandFP) << "\t" << median(bandFP) << "\t" << upperQuartile(bandFP) << "\t"
			<< lowerQuartile(bandFN) << "\t" << median(bandFN) << "\t" << upperQuartile(bandFN) << "\t"
			<< lowerQuartile(bandTP) << "\t" << median(bandTP) << "\t" << upperQuartile(bandTP) << "\t"
			<< lowerQuartile(bandTN) << "\t" << median(bandTN) << "\t" << upperQuartile(bandTN) << "\t"
			<< lowerQuartile(bandFPAmps) << "\t" << median(bandFPAmps) << "\t" << upperQuartile(bandFPAmps) << "\t"
			<< lowerQuartile(bandFNAmps) << "\t" << median(bandFNAmps) << "\t" << upperQuartile(bandFNAmps) << "\n";
			bandTotals.clear();
			bandAmps.clear();
			bandRFIs.clear();
			bandRFIAmps.clear();
			bandBRFIs.clear();
			bandLRFIs.clear();
			bandBRFIAmps.clear();
			bandLRFIAmps.clear();
			bandFP.clear();
			bandFN.clear();
			bandTP.clear();
			bandTN.clear();
			bandFPAmps.clear();
			bandFNAmps.clear();
		}
		++index;
	}
	file.close();
	if(index%countPerSubband != 0)
		AOLogger::Warn << "Warning: " << (index%countPerSubband) << " rows were not part of a sub-band (channels were not dividable by " << countPerSubband << ")\n";
}

void RFIStatistics::saveTimeIntegrated(const std::map<double, class TimestepInfo> &timesteps, const std::string &filename)
{
	size_t steps = 200;
	if(timesteps.size() < steps) steps = timesteps.size();
	std::ofstream file(filename.c_str());
	file <<
		"timestep\ttime\ttotalCount\ttotalAmplitude\trfiCount\t"
		"rfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\t"
		"totalCountLQ\ttotalCountUQ\ttotalAmplitudeLQ\ttotalAmplitudeUQ\trfiCountLQ\trfiCountUQ\t"
		"rfiSummedAmplitudeLQ\trfiSummedAmplitudeUQ\tbroadbandRfiCountLQ\tbroadbandRfiCountUQ\t"
		"lineRfiCountLQ\tlineRfiCountUQ\tbroadbandRfiAmplitudeLQ\tbroadbandRfiAmplitudeUQ\t"
		"lineRfiAmplitudeLQ\tlineRfiAmplitudeUQ\n"
	<< std::setprecision(14);
	size_t index = 0, integratedSteps = 0;
	std::multiset<double>
		totals,
		amps,
		rfis,
		rfiAmps,
		brfis,
		lrfis,
		brfiAmps,
		lrfiAmps;
	for(std::map<double, class TimestepInfo>::const_iterator i=timesteps.begin();i!=timesteps.end();++i)
	{
		const TimestepInfo &t = i->second;
		if(totals.size() == 0)
			file << t.time << "\t-\t";
		totals.insert(t.totalCount);
		amps.insert(t.totalAmplitude);
		rfis.insert(t.rfiCount);
		rfiAmps.insert(t.rfiAmplitude);
		brfis.insert(t.broadbandRfiCount);
		lrfis.insert(t.lineRfiCount);
		brfiAmps.insert(t.broadbandRfiAmplitude);
		lrfiAmps.insert(t.lineRfiAmplitude);
		++index;
		if(index * steps / timesteps.size() > integratedSteps)
		{
			file
			<< avg(totals) << "\t"
			<< avg(amps) << "\t"
			<< avg(rfis) << "\t"
			<< avg(rfiAmps) << "\t"
			<< avg(brfis) << "\t"
			<< avg(lrfis) << "\t"
			<< avg(brfiAmps) << "\t"
			<< avg(lrfiAmps) << "\t"
			<< lowerQuartile(totals) << "\t" << median(totals) << '\t' << upperQuartile(totals) << "\t"
			<< lowerQuartile(amps) << "\t" << median(amps) << '\t' << upperQuartile(amps) << "\t"
			<< lowerQuartile(rfis) << "\t" << median(rfis) << '\t' << upperQuartile(rfis) << "\t"
			<< lowerQuartile(rfiAmps) << "\t" << median(rfiAmps) << '\t' << upperQuartile(rfiAmps) << "\t"
			<< lowerQuartile(brfis) << "\t" << median(brfis) << '\t' << upperQuartile(brfis) << "\t"
			<< lowerQuartile(lrfis) << "\t" << median(lrfis) << '\t' << upperQuartile(lrfis) << "\t"
			<< lowerQuartile(brfiAmps) << "\t" << median(brfiAmps) << '\t' << upperQuartile(brfiAmps) << "\t"
			<< lowerQuartile(lrfiAmps) << "\t" << median(lrfiAmps) << '\t' << upperQuartile(lrfiAmps) << "\n";
			totals.clear();
			amps.clear();
			rfis.clear();
			rfiAmps.clear();
			brfis.clear();
			lrfis.clear();
			brfiAmps.clear();
			lrfiAmps.clear();
			++integratedSteps;
		}
	}
	file.close();
}

void RFIStatistics::saveAmplitudes(const std::map<double, class AmplitudeBin> &amplitudes, const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file << "centr-amplitude\tlog-centr-amplitude\tcount\trfiCount\tbroadbandRfiCount\tlineRfiCount\tfeatureAvgCount\tfeatureIntCount\tfeatureMaxCount\txx\txy\tyx\tyy\txxRfi\txyRfi\tyxRfi\tyyRfi\tstokesQ\tstokesU\tstokesV\tFalseP\tFalseN\tTrueP\tTrueN\n" << std::setprecision(14);
	for(std::map<double, class AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
	{
		const AmplitudeBin &a = i->second;
		double logAmp = a.centralAmplitude > 0.0 ? log10(a.centralAmplitude) : 0.0;
		file
			<< a.centralAmplitude << '\t'
			<< logAmp << '\t'
			<< a.count << '\t'
			<< a.rfiCount << '\t'
			<< a.broadbandRfiCount << '\t'
			<< a.lineRfiCount << '\t'
			<< a.featureAvgCount<< '\t'
			<< a.featureIntCount << '\t'
			<< a.featureMaxCount << '\t'
			<< a.xxCount << '\t'
			<< a.xyCount << '\t'
			<< a.yxCount << '\t'
			<< a.yyCount << '\t'
			<< a.xxRfiCount << '\t'
			<< a.xyRfiCount << '\t'
			<< a.yxRfiCount << '\t'
			<< a.yyRfiCount << '\t'
			<< a.stokesQCount << '\t'
			<< a.stokesUCount << '\t'
			<< a.stokesVCount << '\t'
			<< a.falsePositiveCount << "\t"
			<< a.falseNegativeCount << "\t"
			<< a.truePositiveCount << "\t"
			<< a.trueNegativeCount
			<< "\n";
	}
	file.close();
}

void RFIStatistics::saveBaselines(const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file << "a1\ta2\ta1name\ta2name\tbaselineLength\tbaselineAngle\tcount\ttotalAmplitude\trfiCount\tbroadbandRfiCount\tlineRfiCount\trfiAmplitude\tbroadbandRfiAmplitude\tlineRfiAmplitude\n" << std::setprecision(14);
	for(BaselineMatrix::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
	{
		const std::map<int, BaselineInfo> &row = i->second;
		for(std::map<int, BaselineInfo>::const_iterator j=row.begin();j!=row.end();++j)
		{
			const BaselineInfo &b = j->second;
			file
				<< b.antenna1 << "\t"
				<< b.antenna2 << "\t"
				<< b.antenna1Name << "\t"
				<< b.antenna2Name << "\t"
				<< b.baselineLength << "\t"
				<< b.baselineAngle << "\t"
				<< b.count << "\t"
				<< b.totalAmplitude << "\t"
				<< b.rfiCount << "\t"
				<< b.broadbandRfiCount << "\t"
				<< b.lineRfiCount << "\t"
				<< b.rfiAmplitude << "\t"
				<< b.broadbandRfiAmplitude << "\t"
				<< b.lineRfiAmplitude << "\n";
			if(b.baselineStatistics != 0)
			{
				std::stringstream s;
				s << _filePrefix << "baseline-" << b.antenna1 << 'x' << b.antenna2 << '-';
				b.baselineStatistics->saveWithoutBaselines(s.str());
			}
		}
	}
	file.close();
}

void RFIStatistics::saveBaselineTimeInfo(const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file << "antenna1\tantenna2\ttime\ttotalCount\trfiCount\n" << std::setprecision(14);
	for(BaselineTimeInfoMap::const_iterator i=_baselineTimeInfo.begin();i!=_baselineTimeInfo.end();++i)
	{
		const BaselineTimeInfo &info = i->second;
		file
			<< info.antenna1Index << "\t"
			<< info.antenna2Index << "\t"
			<< info.time << "\t"
			<< info.totalCount << "\t"
			<< info.rfiCount << "\n";
	}
	file.close();
}

void RFIStatistics::saveStationTimeInfoRow(std::ostream &stream, bool &firstRow, std::vector<unsigned long> &totals, std::vector<unsigned long> &rfis, double time)
{
	if(firstRow)
	{
		stream << "time";
		for(unsigned i=0;i<totals.size();++i)
		{
			stream << "\ttotal" << i << "\trfi" << i;
		}
		stream << '\n' << std::setprecision(14);
		firstRow = false;
	}

	stream << time;
	for(unsigned i=0;i<totals.size();++i)
	{
		stream << '\t' << totals[i] << '\t' << rfis[i];
		totals[i] = 0;
		rfis[i] = 0;
	}
	stream << '\n';
}

void RFIStatistics::saveStationTimeInfo(const std::string &filename, const std::string &plotname)
{
	std::ofstream file(filename.c_str());
	std::ofstream stationTimePlot(plotname.c_str());
	bool firstRow = true;
	unsigned stationCount = 0;
	if(_baselineTimeInfo.size() > 0)
	{
		// Calculate number of time steps
		double time = _baselineTimeInfo.begin()->second.time;
		unsigned long timeStepCount = 1;
		for(BaselineTimeInfoMap::const_iterator i=_baselineTimeInfo.begin();i!=_baselineTimeInfo.end();++i)
		{
			if(time != i->second.time)
			{
				time = i->second.time;
				++timeStepCount;
			}
		}
		
		time = _baselineTimeInfo.begin()->second.time;
		unsigned long timeStepIndex = 0 , partIndex = 0;
		std::vector<unsigned long> totals, rfis;
		for(BaselineTimeInfoMap::const_iterator i=_baselineTimeInfo.begin();i!=_baselineTimeInfo.end();++i)
		{
			const BaselineTimeInfo &info = i->second;
			unsigned requiredSize = info.antenna1Index+1;
			if(info.antenna2Index >= requiredSize)
				requiredSize = info.antenna2Index+1;
			
			if(totals.size() <= requiredSize)
			{
				totals.resize(requiredSize);
				rfis.resize(requiredSize);
			}
			
			if(info.time != time)
			{
				if(firstRow)
					stationCount = totals.size();
				if(timeStepIndex >= (partIndex+1) * timeStepCount / 200)
				{
					saveStationTimeInfoRow(file, firstRow, totals, rfis, time);
					++partIndex;
				}
				++timeStepIndex;
				time = info.time;
			}
			
			totals[info.antenna1Index] += info.totalCount;
			rfis[info.antenna1Index] += info.rfiCount;
			totals[info.antenna2Index] += info.totalCount;
			rfis[info.antenna2Index] += info.rfiCount;
		}
		saveStationTimeInfoRow(file, firstRow, totals, rfis, time);

		stationTimePlot << std::setprecision(14) <<
			"set term postscript enhanced color font \"Helvetica,16\"\n"
			"set title \"RFI station statistics by time\"\n"
			"set xlabel \"Time (hrs)\"\n"
			"set ylabel \"RFI (percentage)\"\n"
			"set output \"StationsTime-Rfi.ps\"\n"
			"set key inside top\n"
			"set xrange [" << 0 << ":" << ((time-_baselineTimeInfo.begin()->second.time)/(60.0*60.0)) << "]\n"
			"plot \\\n";
		std::stringstream timeAxisStr;
		timeAxisStr << std::setprecision(14) << "((column(1)-" << _baselineTimeInfo.begin()->second.time << ")/(60.0*60.0))";
		const std::string timeAxis = timeAxisStr.str();
		for(unsigned x=0;x<stationCount;++x)
		{
			if(x != 0)
				stationTimePlot << ", \\\n";
			stationTimePlot
			<< "\"counts-stationstime.txt\" using "
			<< timeAxis
			<< ":(100*column(" << (x*2+3)
			<< ")/column(" << (x*2+2)
			<< ")) title \"" << getStationName(x) << "\" with lines lw 2";
		}
		stationTimePlot << '\n';
	}
		
	file.close();
	stationTimePlot.close();
}

void RFIStatistics::saveBaselineFrequencyInfo(const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file << "antenna1\tantenna2\tcentralFrequency\ttotalCount\trfiCount\n" << std::setprecision(14);
	for(BaselineFrequencyInfoMap::const_iterator i=_baselineFrequencyInfo.begin();i!=_baselineFrequencyInfo.end();++i)
	{
		const BaselineFrequencyInfo &info = i->second;
		file
			<< info.antenna1Index << "\t"
			<< info.antenna2Index << "\t"
			<< info.centralFrequency << "\t"
			<< info.totalCount << "\t"
			<< info.rfiCount << "\n";
	}
	file.close();
}

void RFIStatistics::saveTimeFrequencyInfo(TimeFrequencyInfoMap &map, const std::string &filename)
{
	std::ofstream file(filename.c_str());
	file << "time\tfrequency\ttotalCount\trfiCount\ttotalAmplitude\trfiAmplitude\n" << std::setprecision(14);
	for(TimeFrequencyInfoMap::const_iterator i=map.begin();i!=map.end();++i)
	{
		const TimeFrequencyInfo &info = i->second;
		file
			<< info.time << '\t'
			<< info.centralFrequency << '\t'
			<< info.totalCount << '\t'
			<< info.rfiCount << '\t'
			<< info.totalAmplitude << '\t'
			<< info.rfiAmplitude << '\n';
	}
	file.close();
}

void RFIStatistics::saveBaselinesOrdered(const std::string &filename)
{
	std::vector<BaselineInfo> orderedBaselines;
	for(BaselineMatrix::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
	{
		const std::map<int, BaselineInfo> &row = i->second;
		for(std::map<int, BaselineInfo>::const_iterator j=row.begin();j!=row.end();++j)
		{
			const BaselineInfo &b = j->second;
			orderedBaselines.push_back(b);
		}
	}
	std::sort(orderedBaselines.begin(), orderedBaselines.end());

	std::ofstream file(filename.c_str());
	file << "a1\ta2\ta1name\ta2name\tbaselineLength\tbaselineAngle\tcount\ttotalAmplitude\trfiCount\tbroadbandRfiCount\tlineRfiCount\trfiAmplitude\tbroadbandRfiAmplitude\tlineRfiAmplitude\n" << std::setprecision(14);
	for(std::vector<BaselineInfo>::const_iterator i=orderedBaselines.begin();i!=orderedBaselines.end();++i)
	{
		const BaselineInfo &b = *i;
		file
			<< b.antenna1 << "\t"
			<< b.antenna2 << "\t"
			<< b.antenna1Name << "\t"
			<< b.antenna2Name << "\t"
			<< b.baselineLength << "\t"
			<< b.baselineAngle << "\t"
			<< b.count << "\t"
			<< b.totalAmplitude << "\t"
			<< b.rfiCount << "\t"
			<< b.broadbandRfiCount << "\t"
			<< b.lineRfiCount << "\t"
			<< b.rfiAmplitude << "\t"
			<< b.broadbandRfiAmplitude << "\t"
			<< b.lineRfiAmplitude << "\n";
	}
	file.close();
}

void RFIStatistics::saveStations(const std::string &filename)
{
	unsigned stationCount = _baselines.size();
	for(BaselineMatrix::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
	{
		const std::map<int, BaselineInfo> &row = i->second;
		if(row.size() > stationCount) stationCount = row.size();
	}
	double *values = new double[stationCount];
	for(unsigned i=0;i<stationCount;++i)
		values[i] = 0.0;
	
	
}

long double RFIStatistics::FitScore(Image2DCPtr image, Image2DCPtr fit, Mask2DCPtr mask)
{
	long double summedError = 0.0L;
	unsigned count = 0;

	for(unsigned y=0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width();++x) {
			if(!mask->Value(x, y) && std::isfinite(image->Value(x, y)))
			{
				long double error = image->Value(x, y) - fit->Value(x, y);
				summedError += error * error;
				++count;
			} else {
			}
		}
	}
	long double procentData = (long double) count / (image->Width() * image->Height());
	long double averageError = summedError / (image->Width() * image->Height());
	//long double avgError = summedError / (image->Width()*image->Height());
	//return 1.0L/(summedError + avgError * 2.0L * (long double) count);
	return procentData/averageError;
}

num_t RFIStatistics::DataQuality(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned startX, unsigned endX)
{
	unsigned count = 0;
	double sum = 0;
	for(unsigned y=0;y<image->Height();++y)
	{
		for(unsigned x=startX;x<endX;++x)
		{
			if(!mask->Value(x, y) && std::isfinite(image->Value(x, y)) && std::isfinite(model->Value(x,y)))
			{
				num_t noise = fabsn(image->Value(x, y) - model->Value(x, y));
				num_t signal = fabsn(model->Value(x, y));
				if(signal != 0.0)
				{
					if(noise <= 1e-50) noise = 1e-50;
					num_t snr = logn(signal / noise);
					sum += snr;

					++count;
				}
			}
		}
	}
	if(count == 0)
		return 0;
	else
		return sum / (sqrtn(count) * sqrtn((endX-startX) * image->Height()));
}

num_t RFIStatistics::FrequencySNR(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned channel)
{
	num_t sum = 0.0;
	size_t count = 0;
	for(unsigned x=0;x<image->Width();++x)
	{
		if(!mask->Value(x, channel))
		{
			num_t noise = fabsn(image->Value(x, channel) - model->Value(x, channel));
			num_t signal = fabsn(model->Value(x, channel));
			if(std::isfinite(signal) && std::isfinite(noise))
			{
				if(noise <= 1e-50) noise = 1e-50;
				num_t snr = logn(signal / noise);
				sum += snr;
	
				++count;
			}
		}
	}
	return expn(sum / count);
}

void RFIStatistics::createStationData(std::vector<StationInfo> &stations) const
{
	for(BaselineMatrix::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
	{
		size_t index1 = i->first;
		if(index1 >= stations.size())
			stations.resize(index1+1);
		const std::map<int, BaselineInfo> &row = i->second;
		for(std::map<int, BaselineInfo>::const_iterator j=row.begin();j!=row.end();++j)
		{
			size_t index2 = j->first;
			if(index2 >= stations.size())
				stations.resize(index2+1);

			const BaselineInfo &b = j->second;
			StationInfo &a1 = stations[index1], &a2 = stations[index2];
			a1.index = j->second.antenna1;
			a1.name = j->second.antenna1Name;
			a1.totalRfi += (double) b.rfiCount / (double) b.count;
			a1.count++;
			a2.index = j->second.antenna2;
			a2.name = j->second.antenna2Name;
			a2.totalRfi += (double) b.rfiCount / (double) b.count;
			a2.count++;
		}
	}
	for(std::vector<StationInfo>::iterator i=stations.begin();i!=stations.end();++i)
	{
		if(i->count == 0)
		{
			--i;
			stations.erase(i+1);
		}
	}
	std::sort(stations.begin(), stations.end());
}

void RFIStatistics::saveMetaData(const std::string &filename) const
{
	if(_crossTimesteps.empty() || _crossChannels.empty())
		return;
	
	const class TimestepInfo
		&firstStep = _crossTimesteps.begin()->second,
		&lastStep = _crossTimesteps.rbegin()->second;
	const class ChannelInfo
		&startChannel = _crossChannels.begin()->second,
		&endChannel = _crossChannels.rbegin()->second;

	double startTime = firstStep.time;
	double endTime = lastStep.time;
	double timeResolution = round(100.0 * (lastStep.time - firstStep.time) / (double) _crossTimesteps.size()) * 0.01;

	double
		freqResolution = round((endChannel.frequencyHz - startChannel.frequencyHz) / ((double) _crossChannels.size() * 1000.0 * 0.01))*0.01,
		startFrequency = round(startChannel.frequencyHz/100000.0)/10.0,
		endFrequency = round(endChannel.frequencyHz/100000.0)/10.0;
	double lengthInUnits;
	std::string lengthUnits;
	if((endTime - startTime) / 60.0 > 120.0)
	{
		lengthInUnits = round(10.0*(endTime - startTime) / (60.0*60.0))/10.0;
		lengthUnits = "hrs";
	} else if((endTime - startTime) > 120.0)
	{
		lengthInUnits = round(10.0*(endTime - startTime) / (60.0)) / 10.0;
		lengthUnits = "min";
	} else {
		lengthInUnits = round(10.0*(endTime - startTime) );
		lengthUnits = "s";
	}
	double baselineCount = (double) _baselines.size() * (double) (_baselines.size()+1) / 2.0;
	double totalSize = (double) _crossTimesteps.size() * (double) _crossChannels.size() * 4.0 * 8.0 * baselineCount;
	double sizeInUnits;
	std::string sizeUnits;
	if(totalSize > (1024.0*1024.0*1024.0*1024.0*1024.0/10.0))
	{
		sizeInUnits = round(totalSize*10.0/(1024.0*1024.0*1024.0*1024.0*1024.0))/10.0;
		sizeUnits = "PB";
	} else if(totalSize > (1024.0*1024.0*1024.0*1024.0/10.0))
	{
		sizeInUnits = round(totalSize*10.0/(1024.0*1024.0*1024.0*1024.0))/10.0;
		sizeUnits = "TB";
	} else
	{
		sizeInUnits = round(totalSize*10.0/(1024.0*1024.0*1024.0))/10.0;
		sizeUnits = "GB";
	}
	std::vector<BaselineInfo> orderedBaselines;
	for(BaselineMatrix::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
	{
		const std::map<int, BaselineInfo> &row = i->second;
		for(std::map<int, BaselineInfo>::const_iterator j=row.begin();j!=row.end();++j)
		{
			const BaselineInfo &b = j->second;
			orderedBaselines.push_back(b);
		}
	}
	std::sort(orderedBaselines.begin(), orderedBaselines.end());
	double maxBaselineLength = round(10.0*orderedBaselines.rbegin()->baselineLength / 1000.0) / 10.0;
	
	size_t countPerSubband = _channelCountPerSubband;
	if(_ignoreFirstChannel) --countPerSubband;

	std::ofstream file(filename.c_str());
	file
		<< "Observation date: " << Date::AipsMJDToDateString(startTime) << "\\\\\n"
		<< "Start time: " << Date::AipsMJDToRoundTimeString(startTime) << "\\\\\n"
		<< "Observation length: " << lengthInUnits << ' ' << lengthUnits << "\\\\\n"
		<< "Time resolution: " << timeResolution << " s \\\\\n"
		<< "Total percentage of RFI: " << round(10000.0*RFIFractionInCrossTimeSteps())/100.0 << " \\%\\\\\n"
		<< "Number of channels/sub-band: " << _channelCountPerSubband << "\\\\\n"
		<< "Number of sub-bands: " << (_crossChannels.size()/countPerSubband) << "\\\\\n"
		<< "Number of stations: " << _baselines.size() << "\\\\\n"
		<< "Frequency range: " << startFrequency << "-" << endFrequency << " MHz\\\\\n"
		<< "Frequency resolution: " << freqResolution << " kHz \\\\\n"
		<< "Total size: " << sizeInUnits << ' ' << sizeUnits << "\\\\\n"
		<< "Max baseline length: " << maxBaselineLength << " km\\\\\n";
		
	std::vector<StationInfo> stations;
	createStationData(stations);
	if(stations.size() > 3)
	{
		file << "Best 3 stations: ";
		std::vector<StationInfo>::const_iterator stationI = stations.begin();
		file << toTex(stationI->name) << ' ' << round(stationI->totalRfi*1000.0/(double) stationI->count)/10.0 << "\\%,\\\\\n\\indent ";
		++stationI;
		file << toTex(stationI->name) << ' ' << round(stationI->totalRfi*1000.0/(double) stationI->count)/10.0 << "\\%, ";
		++stationI;
		file << toTex(stationI->name) << ' ' << round(stationI->totalRfi*1000.0/(double) stationI->count)/10.0 << "\\%";

		file << "\\\\\nWorst 3 stations: ";
		std::vector<StationInfo>::const_reverse_iterator stationRI = stations.rbegin();
		file << toTex(stationRI->name) << ' ' << round(stationRI->totalRfi*1000.0/(double) stationRI->count)/10.0 << "\\%,\\\\\n\\indent ";
		++stationRI;
		file << toTex(stationRI->name) << ' ' << round(stationRI->totalRfi*1000.0/(double) stationRI->count)/10.0 << "\\%, ";
		++stationRI;
		file << toTex(stationRI->name) << ' ' << round(stationRI->totalRfi*1000.0/(double) stationRI->count)/10.0 << "\\%\n";
	}

	file.close();
}

void RFIStatistics::savePlots(const std::string &basename) const
{
	
	class TimestepInfo firstStep, lastStep;
	if(!_crossTimesteps.empty())
	{
		firstStep = _crossTimesteps.begin()->second;
		lastStep = _crossTimesteps.rbegin()->second;
	}
	class ChannelInfo startChannel, endChannel;
	if(!_crossChannels.empty())
	{
		startChannel = _crossChannels.begin()->second;
		endChannel = _crossChannels.rbegin()->second;
	}

	std::ofstream baselPlot((basename + "Baseline.plt").c_str());
	baselPlot << std::setprecision(14) <<
		"set term postscript enhanced color font \"Helvetica,16\"\n"
		"set title \"RFI statistics by baseline length\"\n"
		"set xlabel \"Baseline length (m)\"\n"
		"set ylabel \"RFI (percentage)\"\n"
		"set lmargin 2.5\n"
		"set rmargin 0.5\n"
		"set tmargin 0.3\n"
		"set bmargin 0.1\n"
		"set log x\n"
		"set output \"Baselines-Rfi.ps\"\n"
		"set key inside top\n"
		"plot \\\n"
		"\"counts-obaselines.txt\" using 5:(100*column(9)/column(7)) title \"Total\" with points lw 0.5 lc rgbcolor \"#FF0000\", \\\n"
		"\"counts-obaselines.txt\" using 5:(100*column(10)/column(7)) title \"Broadband\" with points lw 0.5 lc rgbcolor \"#008000\", \\\n"
		"\"counts-obaselines.txt\" using 5:(100*column(11)/column(7)) title \"Spectral line\" with points lw 0.5 lc rgbcolor \"#0000FF\"\n";
	baselPlot.close();

	std::ofstream distPlot((basename + "Distribution.plt").c_str());
	distPlot << std::setprecision(14) <<
		"set term postscript enhanced color font \"Helvetica,16\"\n"
		"set title \"Data distribution\"\n"
		"set xlabel \"Visibility amplitude\"\n"
		"set ylabel \"RFI (percentage)\"\n"
		"set lmargin 2.5\n"
		"set rmargin 0.5\n"
		"set tmargin 0.3\n"
		"set bmargin 0.1\n"
		"set log x\n"
		"set log y\n"
		"set output \"Distribution.ps\"\n"
		"set key inside top\n"
		"plot \\\n"
		"\"counts-amplitudes-cross.txt\" using 1:(column(3)/column(1)) title \"All\" with points lw 0.5 pt 7 ps 0.4 lc rgbcolor \"#000000\", \\\n"
		"\"counts-amplitudes-cross.txt\" using 1:(column(4)/column(1)) title \"RFI\" with points lw 0.5 pt 7 ps 0.4 lc rgbcolor \"#FF0000\", \\\n"
		"\"counts-amplitudes-cross.txt\" using 1:((column(3)-column(4))/column(1)) title \"Non-RFI\" with points lw 0.5 pt 7 ps 0.4 lc rgbcolor \"#808080\", \\\n"
		"\"counts-amplitudes-cross.txt\" using 1:(column(5)/column(1)) title \"Broadband\" with points lw 0.5 pt 7 ps 0.4 lc rgbcolor \"#008000\", \\\n"
		"\"counts-amplitudes-cross.txt\" using 1:(column(6)/column(1)) title \"Spectral line\" with points lw 0.5 pt 7 ps 0.4 lc rgbcolor \"#0000FF\"\n";
	distPlot.close();

	std::ofstream freqPlot((basename + "Frequency.plt").c_str());
	freqPlot << std::setprecision(14) <<
		"set term postscript enhanced color font \"Helvetica,16\"\n"
		"set title \"RFI statistics by frequency\"\n"
		"set xlabel \"Frequency (MHz)\"\n"
		"set ylabel \"RFI (percentage)\"\n"
		"set lmargin 2.5\n"
		"set rmargin 0.5\n"
		"set tmargin 0.3\n"
		"set bmargin 0.1\n"
		"set output \"Frequency-Rfi.ps\"\n"
		"set key inside top\n"
		"set xrange [" << startChannel.frequencyHz/1000000.0 << ':' << endChannel.frequencyHz/1000000.0 << "]\n"
		"plot \\\n"
		"\"counts-subbands-cross.txt\" using ((column(2)+column(3))/2000000):(100*column(24)/column(4)):(100*column(26)/column(4)) title \"Total (quartiles)\" with filledcu lc rgbcolor \"#FF8080\", \\\n"
		"\"counts-subbands-cross.txt\" using ((column(2)+column(3))/2000000):(100*column(25)/column(4)) title \"Total (median)\" with lines lw 2 lt 2 lc rgbcolor \"#800000\", \\\n"
		"\"counts-subbands-cross.txt\" using ((column(2)+column(3))/2000000):(100*column(6)/column(4)) title \"Total (average)\" with lines lw 2 lt 1 lc rgbcolor \"#FF0000\", \\\n"
		"\"counts-subbands-cross.txt\" using ((column(2)+column(3))/2000000):(100*column(8)/column(4)) title \"Broadband\" with lines lw 2 lt 3 lc rgbcolor \"#008000\", \\\n"
		"\"counts-subbands-cross.txt\" using ((column(2)+column(3))/2000000):(100*column(9)/column(4)) title \"Spectral line\" with lines lw 2 lt 4 lc rgbcolor \"#0000FF\"\\\n";
	freqPlot.close();

	std::ostringstream timeStr;
	timeStr << std::setprecision(14);
	std::string timeAxisCaption;
	double rangeEnd;
	if(lastStep.time-firstStep.time > 60*120)
	{
		timeStr << "((column(1)-" << firstStep.time << ")/(60.0*60.0))";
		timeAxisCaption = "Time (hrs)";
		rangeEnd = (lastStep.time - firstStep.time) / (60.0*60.0);
	} else if(lastStep.time-firstStep.time > 120)
	{
		timeStr << "((column(1)-" << firstStep.time << ")/60.0)";
		timeAxisCaption = "Time (min)";
		rangeEnd = (lastStep.time - firstStep.time) / 60.0;
	} else {
		timeStr << "(column(1)-" << firstStep.time << ")";
		timeAxisCaption = "Time (s)";
		rangeEnd = lastStep.time - firstStep.time;
	}
	const std::string timeAxis = timeStr.str();
	std::ofstream timePlot((basename + "Time.plt").c_str());
	timePlot << std::setprecision(14) <<
		"set term postscript enhanced color font \"Helvetica,16\"\n"
		"set title \"RFI statistics by time\"\n"
		"set xlabel \"" << timeAxisCaption << "\"\n"
		"set ylabel \"RFI (percentage)\"\n"
		"set lmargin 2.5\n"
		"set rmargin 0.5\n"
		"set tmargin 0.3\n"
		"set bmargin 0.1\n"
		"set output \"Time-Rfi.ps\"\n"
		"set key inside top\n"
		"set xrange [0:" << rangeEnd << "]\n"
		"plot \\\n";
	if(_crossTimesteps.size() > 400)
	{
		timePlot << 
			"\"counts-timeint-cross.txt\" using " << timeAxis << ":(100*column(17)/column(3)):(100*column(19)/column(3)) title \"Total (quartiles)\" with filledcu lc rgbcolor \"#FF8080\", \\\n"
			"\"counts-timeint-cross.txt\" using " << timeAxis << ":(100*column(18)/column(3)) title \"Total (median)\" with lines lw 2 lt 2 lc rgbcolor \"#800000\", \\\n";
	}
	timePlot << 
		"\"counts-timeint-cross.txt\" using " << timeAxis << ":(100*column(5)\
/column(3)) title \"Total (average)\" with lines lw 2 lt 1 lc rgbcolor \"#FF0000\", \\\n"
		"\"counts-timeint-cross.txt\" using " << timeAxis << ":(100*column(7)\
/column(3)) title \"Broadband\" with lines lw 2 lt 3 lc rgbcolor \"#008000\", \\\n"
		"\"counts-timeint-cross.txt\" using " << timeAxis << ":(100*column(8)\
/column(3)) title \"Spectral line\" with lines lw 2 lt 4 lc rgbcolor \"#0000FF\"\n";

	timePlot.close();
}
