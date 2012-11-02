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

#ifndef TIMEFREQUENCYSTATIMAGESET_H
#define TIMEFREQUENCYSTATIMAGESET_H

#include <string>
#include <fstream>
#include <set>
#include <map>

#include "../../msio/types.h"

#include "singleimageset.h"
#include "../../util/aologger.h"

namespace rfiStrategy {

	class TimeFrequencyStatImageSet : public SingleImageSet {
		public:
			enum Mode { RFIPercentages, TotalAmplitude, RFIAmplitude, NonRFIAmplitude };
			
			TimeFrequencyStatImageSet(const std::string &path) : SingleImageSet(), _path(path), _mode(RFIPercentages)
			{
			}

			virtual ImageSet *Copy()
			{
				TimeFrequencyStatImageSet *newSet = new TimeFrequencyStatImageSet(_path);
				newSet->_mode = _mode;
				return newSet;
			}

			virtual void Initialize()
			{
			}

			virtual std::string Name()
			{
				switch(_mode)
				{
					default:
					case RFIPercentages:
						return "TF Statistics: RFI percentages";
					case TotalAmplitude:
						return "TF Statistics: Total amplitudes";
					case RFIAmplitude:
						return "TF Statistics: RFI amplitudes";
					case NonRFIAmplitude:
						return "TF Statistics: Non-RFI amplitudes";
				}
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
				// Scan file for width (time steps) and height (frequency bands)
				std::ifstream f(_path.c_str());
				std::string headers;
				std::getline(f, headers);
				std::set<double> times, frequencies;
				while(!f.eof())
				{
					double time, centralFrequency;
					unsigned long totalCount, rfiCount;
					double totalAmplitude, rfiAmplitude;
					f >> time;
					if(f.eof()) break;
					f
					>> centralFrequency
					>> totalCount
					>> rfiCount
					>> totalAmplitude
					>> rfiAmplitude;
					times.insert(time);
					frequencies.insert(centralFrequency);
				}
				unsigned width = times.size(), height = frequencies.size();
				AOLogger::Debug << "Image size: " << width << 'x' << height << '\n';
				Image2DPtr image = Image2D::CreateZeroImagePtr(width, height);
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<true>(width, height);
				
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
				
				// Rescan file, now actually store data
				f.close();
				f.open(_path.c_str());
				std::getline(f, headers);
				while(!f.eof())
				{
					double time, centralFrequency;
					unsigned long totalCount, rfiCount;
					double totalAmplitude, rfiAmplitude;
					f >> time;
					if(f.eof()) break;
					f
					>> centralFrequency
					>> totalCount
					>> rfiCount
					>> totalAmplitude
					>> rfiAmplitude;
					
					//AOLogger::Debug << "Time: " << time << " freq: " << centralFrequency << '\n';
					unsigned
						x = timeIndices.find(time)->second,
						y = frequencyIndices.find(centralFrequency)->second;
					//AOLogger::Debug << '(' << x << ',' << y << ")\n";
					
					if(totalCount > 0)
					{
						switch(_mode)
						{
							case RFIPercentages:
								image->SetValue(x, y, 100.0L * (long double) rfiCount / (long double) totalCount);
								mask->SetValue(x, y, false);
								break;
							case TotalAmplitude:
								image->SetValue(x, y, (long double) totalAmplitude / (long double) totalCount);
								mask->SetValue(x, y, false);
								break;
							case RFIAmplitude:
								if(rfiCount > 0) {
									image->SetValue(x, y, (long double) rfiAmplitude / (long double) rfiCount);
									mask->SetValue(x, y, false);
								}
								break;
							case NonRFIAmplitude:
								if(totalCount - rfiCount > 0) {
									image->SetValue(x, y, (long double) (totalAmplitude-rfiAmplitude) / (long double) (totalCount - rfiCount));
									mask->SetValue(x, y, false);
								}
								break;
						}
					}
				}
				
				// Fill metadata
				TimeFrequencyMetaDataPtr metaData = TimeFrequencyMetaDataPtr(new TimeFrequencyMetaData());
				metaData->SetObservationTimes(observationTimes);
				metaData->SetBand(bandInfo);
				metaData->SetDataDescription(dataDescription());
				metaData->SetDataUnits(units());
				
				// Return it structured.
				TimeFrequencyData data(TimeFrequencyData::AmplitudePart, StokesIPolarisation, image);
				data.SetGlobalMask(mask);
				BaselineData *baselineData = new BaselineData(data, metaData);
				return baselineData;
			}
		private:
			virtual std::string units()
			{
				switch(_mode)
				{
					default:
					case RFIPercentages:
						return "%";
					case TotalAmplitude:
					case RFIAmplitude:
					case NonRFIAmplitude:
						return "";
				}
			}

			virtual std::string dataDescription()
			{
				switch(_mode)
				{
					default:
					case RFIPercentages:
						return "RFI fraction";
					case TotalAmplitude:
					case RFIAmplitude:
					case NonRFIAmplitude:
						return "Amplitude";
				}
			}

			std::string _path;
			enum Mode _mode;
	};

}

#endif
