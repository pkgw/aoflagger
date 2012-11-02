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

#ifndef SINGLEIMAGESET_H
#define SINGLEIMAGESET_H

#include <string>
#include <stdexcept>

#include "../../msio/types.h"

#include "imageset.h"

namespace rfiStrategy {

	class SingleImageSetIndex : public ImageSetIndex {
		public:
			SingleImageSetIndex(ImageSet &set, std::string description) : ImageSetIndex(set), _valid(true), _description(description) { }
			virtual ~SingleImageSetIndex() { }
			virtual void Previous() { _valid = false; }
			virtual void Next() { _valid = false; }
			virtual void LargeStepPrevious() { _valid = false; }
			virtual void LargeStepNext() { _valid = false; }
			virtual std::string Description() const { return _description; }
			virtual bool IsValid() const { return _valid; }
			virtual ImageSetIndex *Copy() const
			{
				SingleImageSetIndex *index = new SingleImageSetIndex(imageSet(), _description);
				index->_valid = _valid;
				return index;
			}
		private:
			bool _valid;
			std::string _description;
	};
	
	class SingleImageSet : public ImageSet {
		public:
			SingleImageSet() : ImageSet(), _readCount(0), _lastRead(0)
			{
			}
			
			virtual ~SingleImageSet()
			{
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new SingleImageSetIndex(*this, Name());
			}
			
			virtual std::string Name() = 0;
			virtual std::string File() = 0;
			
			virtual void AddReadRequest(const ImageSetIndex &)
			{
				if(_lastRead != 0)
				{
					delete _lastRead;
					_lastRead = 0;
					_readCount = 1;
				} else {
					++_readCount;
				}
			}
			virtual void PerformReadRequests()
			{
				_lastRead = Read();
			}
			virtual BaselineData *GetNextRequested()
			{
				if(_readCount == 0)
					throw std::runtime_error("All data reads have already been requested");
				if(_lastRead == 0)
					throw std::runtime_error("GetNextRequested() was called before PerformReadRequests()");
				return _lastRead;
			}
			
			virtual BaselineData *Read() = 0;
		private:
			int _readCount;
			BaselineData *_lastRead;
	};

}

#endif
