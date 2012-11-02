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
#ifndef RAWIMAGESET_H
#define RAWIMAGESET_H

#include <deque>
#include <set>
#include <vector>

#include "../../msio/image2d.h"

#include "imageset.h"

#include "../../msio/rawreader.h"

namespace rfiStrategy {
	
	class RawImageSetIndex : public ImageSetIndex {
		public:
			RawImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _isValid(true)
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
			virtual RawImageSetIndex *Copy() const
			{
				RawImageSetIndex *index = new RawImageSetIndex(imageSet());
				index->_isValid = _isValid;
				return index;
			}
		private:
			friend class RawImageSet;

			inline class RawImageSet &RawSet() const;
			
			unsigned long _beamlet, _timeBlock;
			bool _isValid;
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class RawImageSet : public ImageSet
	{
		public:
			RawImageSet(const std::string &file) : _reader(file)
			{
			}
			~RawImageSet()
			{
			}
			virtual void Initialize()
			{
			}

			virtual RawImageSet *Copy()
			{
				return 0;
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new RawImageSetIndex(*this);
			}
			virtual std::string Name()
			{
				return "Raw file";
			}
			virtual std::string File()
			{
				return _reader.Filename();
			}
			virtual void AddReadRequest(const ImageSetIndex &index)
			{
				float data[122100];
				_reader.Read(0, 122100, data);
				Image2DPtr image = Image2D::CreateUnsetImagePtr(122100, 1);
				for(unsigned i=0;i<122100;++i)
				{
					image->SetValue(i, 0, (num_t) data[i]);
				}
				TimeFrequencyData tfData(TimeFrequencyData::AmplitudePart, SinglePolarisation, image);
				
				BaselineData *baseline = new BaselineData(tfData, TimeFrequencyMetaDataPtr(), index);
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
		private:
			RawReader _reader;
			std::deque<BaselineData*> _baselineBuffer;
	};

	void RawImageSetIndex::Previous()
	{
		LargeStepPrevious();
	}
	
	void RawImageSetIndex::Next()
	{
		LargeStepNext();
	}
	
	void RawImageSetIndex::LargeStepPrevious()
	{
	}
	
	void RawImageSetIndex::LargeStepNext()
	{
		_isValid = false;
	}
	
	RawImageSet &RawImageSetIndex::RawSet() const
	{
		return static_cast<RawImageSet&>(imageSet());
	}
}

#endif // RAWIMAGESET_H
