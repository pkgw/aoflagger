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
#ifndef RSPIMAGESET_H
#define RSPIMAGESET_H

#include <deque>
#include <set>
#include <vector>

#include "../../baseexception.h"

#include "imageset.h"

#include "../../msio/rspreader.h"

namespace rfiStrategy {
	
	class RSPImageSetIndex : public ImageSetIndex {
		public:
			RSPImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _beamlet(0), _timeBlock(0), _isValid(true) { }
			
			inline virtual void Previous();
			inline virtual void Next();
			inline virtual void LargeStepPrevious();
			inline virtual void LargeStepNext();
			
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Raw file, time block " << _timeBlock <<", beamlet " << _beamlet;
				return s.str();
			}
			virtual bool IsValid() const
			{
				return _isValid;
			}
			virtual RSPImageSetIndex *Copy() const
			{
				RSPImageSetIndex *index = new RSPImageSetIndex(imageSet());
				index->_beamlet = _beamlet;
				index->_timeBlock = _timeBlock;
				index->_isValid = _isValid;
				return index;
			}
		private:
			friend class RSPImageSet;

			inline class RSPImageSet &RSPSet() const;
			
			unsigned long _beamlet, _timeBlock;
			bool _isValid;
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class RSPImageSet : public ImageSet
	{
		public:
			enum Mode { AllBeamletsMode, SingleBeamletMode, BeamletChannelMode, StatisticsMode };
			
			RSPImageSet(const std::string &file) : _reader(file), _mode(BeamletChannelMode), _timeBlockSize(2048), _beamletsInSet(5)
			{
			}
			~RSPImageSet()
			{
			}
			virtual void Initialize()
			{
				_timestepCount = _reader.TimeStepCount(_beamletsInSet);
				if(_mode == BeamletChannelMode) _timestepCount /= (unsigned long) 256;
			}

			virtual RSPImageSet *Copy()
			{
				return 0;
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new RSPImageSetIndex(*this);
			}
			virtual std::string Name()
			{
				return "Raw RSP file";
			}
			virtual std::string File()
			{
				return _reader.File();
			}
			virtual TimeFrequencyData *LoadData(const ImageSetIndex &)
			{
				return 0;
			}
			virtual size_t GetPart(const ImageSetIndex &)
			{
				return 0;
			}
			virtual size_t GetAntenna1(const ImageSetIndex &)
			{
				return 0;
			}
			virtual size_t GetAntenna2(const ImageSetIndex &)
			{
				return 0;
			}
			virtual void AddReadRequest(const ImageSetIndex &index)
			{
				const RSPImageSetIndex &rspIndex = static_cast<const RSPImageSetIndex&>(index);
				std::pair<TimeFrequencyData,TimeFrequencyMetaDataPtr> data;
				switch(_mode)
				{
					case AllBeamletsMode:
						data = _reader.ReadAllBeamlets(rspIndex._timeBlock * TimeBlockSize(), (rspIndex._timeBlock+1ul) * TimeBlockSize(), _beamletsInSet);
						break;
					case SingleBeamletMode:
						data = _reader.ReadSingleBeamlet(rspIndex._timeBlock * TimeBlockSize(), (rspIndex._timeBlock+1ul) * TimeBlockSize(), _beamletsInSet, rspIndex._beamlet);
						break;
					case BeamletChannelMode:
						data = _reader.ReadChannelBeamlet(rspIndex._timeBlock * TimeBlockSize(), (rspIndex._timeBlock+1ul) * TimeBlockSize(), _beamletsInSet, rspIndex._beamlet);
						break;
					case StatisticsMode:
						_reader.ReadForStatistics(_beamletsInSet);
						data = _reader.ReadChannelBeamlet(rspIndex._timeBlock * TimeBlockSize(), (rspIndex._timeBlock+1ul) * TimeBlockSize(), _beamletsInSet, rspIndex._beamlet);
						break;
				}
				BaselineData *baseline = new BaselineData(data.first, data.second, index);
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
			unsigned long TimestepCount() const
			{
				return _timestepCount;
			}
			unsigned long BeamletCount() const
			{
				switch(_mode)
				{
					default:
					case AllBeamletsMode:
						return 1;
					case SingleBeamletMode:
						return _beamletsInSet;
					case BeamletChannelMode:
						return _beamletsInSet;
				}
			}
			unsigned long TimeBlockSize() const
			{
				return _timeBlockSize;
			}
			void SetTimeBlockSize(unsigned long timeBlockSize)
			{
				_timeBlockSize = timeBlockSize;
			}
			enum Mode Mode() const
			{
				return _mode;
			}
			void SetMode(enum Mode mode)
			{
				_mode = mode;
			}
			unsigned int BeamletsInSet() const
			{
				return _beamletsInSet;
			}
			void SetBeamletsInSet(unsigned int beamletsInSet)
			{
				_beamletsInSet = beamletsInSet;
			}
		private:
			RSPReader _reader;
			std::deque<BaselineData*> _baselineBuffer;
			enum Mode _mode;
			unsigned long _timestepCount;
			unsigned long _timeBlockSize;
			unsigned int _beamletsInSet;
	};

	void RSPImageSetIndex::Previous()
	{
		if(_beamlet > 0)
		{
			--_beamlet;
		} else {
			_beamlet = RSPSet().BeamletCount()-1;
			LargeStepPrevious();
		}
	}
	
	void RSPImageSetIndex::Next()
	{
		++_beamlet;
		if(_beamlet >= RSPSet().BeamletCount())
		{
			_beamlet = 0;
			LargeStepNext();
		}
	}
	
	void RSPImageSetIndex::LargeStepPrevious()
	{
		const unsigned long modulo = (RSPSet().TimestepCount()+RSPSet().TimeBlockSize()-1)/RSPSet().TimeBlockSize();
		_timeBlock = (_timeBlock + modulo - 1) % modulo;
	}
	
	void RSPImageSetIndex::LargeStepNext()
	{
		++_timeBlock;
		const unsigned long modulo = (RSPSet().TimestepCount()+RSPSet().TimeBlockSize()-1)/RSPSet().TimeBlockSize();
		if(_timeBlock >= modulo)
		{
			_timeBlock = 0;
			_isValid = false;
		}
	}
	
	RSPImageSet &RSPImageSetIndex::RSPSet() const
	{
		return static_cast<RSPImageSet&>(imageSet());
	}
}

#endif // RSPIMAGESET_H
