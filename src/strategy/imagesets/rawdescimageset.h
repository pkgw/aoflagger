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
#ifndef RAWDESCIMAGESET_H
#define RAWDESCIMAGESET_H

#include <deque>
#include <set>
#include <vector>

#include "../../msio/image2d.h"
#include "../../msio/rawdescfile.h"

#include "imageset.h"

#include "../../msio/rawreader.h"

namespace rfiStrategy {
	
	class RawDescImageSetIndex : public ImageSetIndex {
		public:
			RawDescImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _isValid(true), _timeBlockIndex(0)
			{
			}
			
			inline virtual void Previous();
			inline virtual void Next();
			inline virtual void LargeStepPrevious();
			inline virtual void LargeStepNext();
			
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Raw file";
				return s.str();
			}
			virtual bool IsValid() const
			{
				return _isValid;
			}
			virtual RawDescImageSetIndex *Copy() const
			{
				RawDescImageSetIndex *index = new RawDescImageSetIndex(imageSet());
				index->_isValid = _isValid;
				index->_timeBlockIndex = _timeBlockIndex;
				return index;
			}
		private:
			friend class RawDescImageSet;

			inline class RawDescImageSet &RawDescSet() const;
			
			bool _isValid;
			size_t _timeBlockIndex;
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class RawDescImageSet : public ImageSet
	{
		public:
			RawDescImageSet(const std::string &file) : _rawDescFile(file), _totalTimesteps(0), _imageWidth(0)
			{
				AOLogger::Debug
					<< "Opening rawdescfile, beams=" << _rawDescFile.BeamCount()
					<< "(" << _rawDescFile.SelectedBeam() << ")"
					<< ", subbands=" << _rawDescFile.SubbandCount()
					<< ", channels=" << _rawDescFile.ChannelsPerSubbandCount()
					<< ", timesteps=" << _rawDescFile.TimestepsPerBlockCount() << '\n';
				_imageWidth = (size_t) round(_rawDescFile.DisplayedTimeDuration() / _rawDescFile.TimeResolution());
				_readers = new RawReader*[_rawDescFile.GetCount()];
				
				for(size_t i=0;i!=_rawDescFile.GetCount();++i)
				{
					_readers[i] = new RawReader(_rawDescFile.GetSet(i));
					_readers[i]->SetSubbandCount(_rawDescFile.SubbandCount());
					_readers[i]->SetChannelCount(_rawDescFile.ChannelsPerSubbandCount());
					_readers[i]->SetBeamCount(_rawDescFile.BeamCount());
					_readers[i]->SetTimestepsPerBlockCount(_rawDescFile.TimestepsPerBlockCount());
					_readers[i]->SetBlockHeaderSize(_rawDescFile.BlockHeaderSize());
					_readers[i]->SetBlockFooterSize(_rawDescFile.BlockFooterSize());
					if(i == 0)
						_totalTimesteps = _readers[i]->TimestepCount();
					else if(_readers[i]->TimestepCount() < _totalTimesteps)
						_totalTimesteps = _readers[i]->TimestepCount();
				}
			}
			
			~RawDescImageSet()
			{
				for(size_t i=0;i!=_rawDescFile.GetCount();++i)
				{
					delete _readers[i];
				}
				delete[] _readers;
			}
			
			virtual void Initialize()
			{
			}

			virtual RawDescImageSet *Copy()
			{
				return new RawDescImageSet(_rawDescFile.Filename());
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new RawDescImageSetIndex(*this);
			}
			virtual std::string Name()
			{
				return "Raw file";
			}
			virtual std::string File()
			{
				return _rawDescFile.Filename();
			}
			virtual void AddReadRequest(const ImageSetIndex &index)
			{
				const RawDescImageSetIndex &rawIndex = static_cast<const RawDescImageSetIndex&>(index);
				size_t readStart = _imageWidth * rawIndex._timeBlockIndex;
				const size_t samplesPerTimestep = _rawDescFile.BeamCount() * _rawDescFile.ChannelsPerSubbandCount() * _rawDescFile.SubbandCount();
				const size_t totalChannels =  _rawDescFile.GetCount() * _rawDescFile.ChannelsPerSubbandCount() * _rawDescFile.SubbandCount();
				
				Image2DPtr image = Image2D::CreateUnsetImagePtr(_imageWidth, totalChannels);
				float *data = new float[_imageWidth * samplesPerTimestep];
				for(size_t setIndex=0;setIndex<_rawDescFile.GetCount();++setIndex)
				{
					_readers[setIndex]->Read(readStart, readStart + _imageWidth, data);
					for(size_t x=0;x<_imageWidth;++x)
					{
						size_t pos = x * _rawDescFile.BeamCount() * _rawDescFile.ChannelsPerSubbandCount() * _rawDescFile.SubbandCount()
							+ _rawDescFile.SelectedBeam() * _rawDescFile.ChannelsPerSubbandCount() * _rawDescFile.SubbandCount();
						size_t y = setIndex * _rawDescFile.ChannelsPerSubbandCount() * _rawDescFile.SubbandCount();
						for(size_t subbandIndex=0;subbandIndex < _rawDescFile.SubbandCount();++subbandIndex)
						{
							for(size_t channelIndex=0;channelIndex < _rawDescFile.ChannelsPerSubbandCount();++channelIndex)
							{
								image->SetValue(x, y, (num_t) data[pos]);
								++y;
								++pos;
							}
						}
					}
				}
				delete[] data;
				TimeFrequencyData tfData(TimeFrequencyData::AmplitudePart, SinglePolarisation, image);
				TimeFrequencyMetaDataPtr metaData(new TimeFrequencyMetaData());
				
				std::vector<double> observationTimes;
				for(unsigned t=0;t<_imageWidth;++t)
				{
					observationTimes.push_back((t + readStart) * _rawDescFile.TimeResolution());
				}
				metaData->SetObservationTimes(observationTimes);
				
				BandInfo bandInfo;
				for(unsigned i=0;i<totalChannels;++i)
				{
					ChannelInfo channel;
					channel.frequencyHz = _rawDescFile.FrequencyStart() + _rawDescFile.FrequencyResolution() * i;
					channel.frequencyIndex = i;
					bandInfo.channels.push_back(channel);
				}
				metaData->SetBand(bandInfo);
				
				BaselineData *baseline = new BaselineData(tfData, metaData, index);
				_baselineBuffer.push_back(baseline);
			}
			virtual void PerformReadRequests()
			{
			}
			virtual BaselineData *GetNextRequested()
			{
				BaselineData *baseline = _baselineBuffer.front();
				_baselineBuffer.pop_front();
				return baseline;
			}
			size_t TotalTimesteps() const { return _totalTimesteps; }
			size_t ImageWidth() const { return _imageWidth; }
		private:
			RawDescFile _rawDescFile;
			RawReader **_readers;
			std::deque<BaselineData*> _baselineBuffer;
			size_t _totalTimesteps, _imageWidth;
	};

	void RawDescImageSetIndex::Previous()
	{
		if(_timeBlockIndex > 0)
		{
			--_timeBlockIndex;
		} else {
			_timeBlockIndex = (RawDescSet().TotalTimesteps()-1) / RawDescSet().ImageWidth();
		}
	}
	
	void RawDescImageSetIndex::Next()
	{
		++_timeBlockIndex;
		if(_timeBlockIndex > (RawDescSet().TotalTimesteps()-1) / RawDescSet().ImageWidth())
		{
			_timeBlockIndex = 0;
			_isValid = false;
		}
	}
	
	void RawDescImageSetIndex::LargeStepPrevious()
	{
	}
	
	void RawDescImageSetIndex::LargeStepNext()
	{
		_isValid = false;
	}
	
	RawDescImageSet &RawDescImageSetIndex::RawDescSet() const
	{
		return static_cast<RawDescImageSet&>(imageSet());
	}
}

#endif // RAWDESCIMAGESET_H
