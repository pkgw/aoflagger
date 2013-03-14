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

#ifndef NOISESTATIMAGESET_H
#define NOISESTATIMAGESET_H

#include <string>
#include <fstream>
#include <set>
#include <map>
#include <cmath>
#include <vector>

#include "../../msio/types.h"

#include "../algorithms/noisestatistics.h"
#include "../algorithms/noisestatisticscollector.h"

#include "singleimageset.h"

#include "../../util/aologger.h"

namespace rfiStrategy {

	class NoiseStatImageSet : public SingleImageSet {
		public:
			enum Mode { Mean, StdDev, Variance, VarianceOfVariance };
			
			NoiseStatImageSet(const std::string &path) : SingleImageSet(), _path(path), _mode(StdDev)
			{
			}

			virtual ImageSet *Copy()
			{
				return new NoiseStatImageSet(_path);
			}

			virtual void Initialize()
			{
			}

			virtual std::string Name()
			{
				return "Noise statistics";
			}
			
			virtual std::string File()
			{
				return _path;
			}
			
			void SetMode(enum Mode mode)
			{
				_mode = mode;
			}
			
			enum Mode Mode() const
			{
				return _mode;
			}
			
			virtual BaselineData *Read()
			{
				NoiseStatisticsCollector collector;
				collector.ReadTF(_path);
				const NoiseStatisticsCollector::StatTFMap &map = collector.TBMap();
				
				// Scan map for width (time steps) and height (frequency channels)
				std::set<double> times, frequencies;
				for(NoiseStatisticsCollector::StatTFMap::const_iterator i=map.begin();
				i!=map.end();++i)
				{
					times.insert(i->first.first);
					frequencies.insert(i->first.second);
				}
				
				std::map<double, unsigned> timeIndices, frequencyIndices;
				std::vector<double> observationTimes;
				BandInfo bandInfo;
				
				unsigned index = 0;
				for(std::set<double>::const_iterator i=times.begin();i!=times.end();++i)
				{
					timeIndices.insert(std::pair<double, unsigned>(*i, index));
					observationTimes.push_back(*i);
					++index;
				}
				
				unsigned width = timeIndices.size(), height = frequencies.size();
				AOLogger::Debug << "Image size: " << width << 'x' << height << '\n';
				Image2DPtr
					imageReal = Image2D::CreateZeroImagePtr(width, height),
					imageImag = Image2D::CreateZeroImagePtr(width, height);
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<true>(width, height);
				index = 0;
				
				for(std::set<double>::const_iterator i=frequencies.begin();i!=frequencies.end();++i)
				{
					frequencyIndices.insert(std::pair<double, unsigned>(*i, index));
					ChannelInfo channel;
					channel.frequencyHz = *i;
					channel.frequencyIndex = index;
					bandInfo.channels.push_back(channel);
					++index;
				}
				bandInfo.windowIndex = 0;
				
				// Rescan map and fill image
				for(NoiseStatisticsCollector::StatTFMap::const_iterator i=map.begin();
				i!=map.end();++i)
				{
					const double
						time = i->first.first,
						centralFrequency = i->first.second;
					const CNoiseStatistics
						&stats = i->second;
					std::map<double, unsigned>::iterator timeElement = timeIndices.upper_bound(time);
					if(timeElement != timeIndices.begin())
						--timeElement;
					const unsigned
						x = timeElement->second,
						y = frequencyIndices.find(centralFrequency)->second;
					
					if(stats.real.Count() > 25 && stats.imaginary.Count() > 25)
					{
						mask->SetValue(x, y, false);
					}
					switch(_mode)
					{
						case Mean:
							imageReal->SetValue(x, y, stats.real.Mean());
							imageImag->SetValue(x, y, stats.imaginary.Mean());
							break;
						case StdDev:
							imageReal->SetValue(x, y, stats.real.StdDevEstimator());
							imageImag->SetValue(x, y, stats.imaginary.StdDevEstimator());
							break;
						case Variance:
							imageReal->SetValue(x, y, stats.real.VarianceEstimator());
							imageImag->SetValue(x, y, stats.imaginary.VarianceEstimator());
							break;
						case VarianceOfVariance:
							imageReal->SetValue(x, y, stats.real.VarianceOfVarianceEstimator());
							imageImag->SetValue(x, y, stats.imaginary.VarianceOfVarianceEstimator());
							break;
					}
				}
				
				// Fill metadata
				TimeFrequencyMetaDataPtr metaData = TimeFrequencyMetaDataPtr(new TimeFrequencyMetaData());
				metaData->SetObservationTimes(observationTimes);
				metaData->SetBand(bandInfo);
				switch(_mode)
				{
					case Mean:
						metaData->SetValueDescription("Mean visibility difference");
						metaData->SetValueUnits("Jy");
						break;
					case StdDev:
						metaData->SetValueDescription("Stddev of visibility difference");
						metaData->SetValueUnits("Jy");
						break;
					case Variance:
						metaData->SetValueDescription("Variance of visibility difference");
						metaData->SetValueUnits("Jy^2");
						break;
					case VarianceOfVariance:
						metaData->SetValueDescription("Variance of visibility difference");
						metaData->SetValueUnits("Jy^4");
						break;
				}
				
				// Return it structured.
				TimeFrequencyData data(TimeFrequencyData::ComplexRepresentation, StokesIPolarisation, imageReal, imageImag);
				data.SetGlobalMask(mask);
				BaselineData *baselineData = new BaselineData(data, metaData);
				return baselineData;
			}
			
			static void MergeInTime(TimeFrequencyData &data, TimeFrequencyMetaDataPtr metaData)
			{
				Mask2DPtr mask = Mask2D::CreateCopy(data.GetSingleMask());
				std::vector<Image2DPtr> images(data.ImageCount());
				for(unsigned i=0;i<data.ImageCount();++i)
					images[i] = Image2D::CreateCopy(data.GetImage(i));
				bool hasObsTimes = metaData != 0;
				
				std::vector<double> observationTimes;
				if(hasObsTimes) observationTimes = metaData->ObservationTimes();
				
				unsigned x=0;
				std::vector<unsigned> removedColumns;
				while(x<mask->Width()-1)
				{
					unsigned checkSize = 1;
					bool rowIsMergable = true;
					
					while(x + checkSize < mask->Width() && rowIsMergable)
					{
						++checkSize;
						// check if the range [ x ; x+checkSize > can be merged
						for(unsigned y=0;y<mask->Height();++y)
						{
							unsigned valuesSet = 0;
							for(unsigned checkX = x; checkX != x + checkSize; ++checkX)
							{
								if(!mask->Value(checkX, y)) ++valuesSet;
							}
							if(valuesSet > 1) {
								rowIsMergable = false;
								break;
							}
						}
					}
					
					if(!rowIsMergable) checkSize--;
					if(checkSize > 1)
					{
						// merge all timesteps in the interval [ x ; x+checkSize >
						AOLogger::Debug << "Merging timesteps " << x << " - " << (x+checkSize-1) << "\n";
						for(unsigned y=0;y<mask->Height();++y)
						{
							double timeAvg = 0.0;
							unsigned timeCount = 0;
							for(unsigned mergeX=x;mergeX!=x+checkSize;++mergeX)
							{
								if(!mask->Value(mergeX, y))
								{
									mask->SetValue(mergeX, y, true);
									mask->SetValue(x, y, false);
									for(unsigned i=0;i<data.ImageCount();++i)
									{
										const num_t val = images[i]->Value(mergeX, y);
										images[i]->SetValue(x, y, val);
									}
									if(hasObsTimes)
									{
										timeAvg += observationTimes[mergeX];
										timeCount++;
									}
								}
							}
							if(hasObsTimes && timeCount > 0)
								observationTimes[x] = timeAvg / (double) timeCount;
						}
						for(unsigned removeX=x+1;removeX!=x+checkSize;++removeX)
							removedColumns.push_back(removeX);
					}
					x += checkSize;
				}
				
				AOLogger::Debug << "Removed " << removedColumns.size() << " time steps.\n";
				
				// Remove the timesteps
				unsigned newWidth = data.ImageWidth() - removedColumns.size();
				std::vector<Image2DPtr> resizedImages(data.ImageCount());
				for(unsigned i=0;i<data.ImageCount();++i)
				{
					resizedImages[i] = Image2D::CreateUnsetImagePtr(newWidth, data.ImageHeight());
				}
				Mask2DPtr resizedMask = Mask2D::CreateUnsetMaskPtr(newWidth, data.ImageHeight());
				
				std::vector<unsigned>::const_iterator nextRemovedCol = removedColumns.begin();
				
				if(nextRemovedCol != removedColumns.begin())
				{
					unsigned xOfResized=0;
					for(unsigned xOfOld=0;xOfOld<mask->Width();++xOfOld)
					{
						if(xOfOld == *nextRemovedCol)
						{
							++nextRemovedCol;
							if(hasObsTimes)
								observationTimes.erase(observationTimes.begin()+xOfResized);
						} else {
							for(unsigned y=0;y<mask->Height();++y)
							{
								resizedMask->SetValue(xOfResized, y, mask->Value(xOfOld, y));
								for(unsigned i=0;i<data.ImageCount();++i)
									resizedImages[i]->SetValue(xOfResized, y, images[i]->Value(xOfOld, y));
							}
							++xOfResized;
						}
					}
					
					for(unsigned i=0;i<data.ImageCount();++i)
						data.SetImage(i, resizedImages[i]);
					data.SetGlobalMask(resizedMask);
					
					if(hasObsTimes)
						metaData->SetObservationTimes(observationTimes);
				}
			}

		private:
			std::string _path;
			enum Mode _mode;
	};
	
}

#endif
