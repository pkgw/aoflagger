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
#ifndef WRITEFLAGSACTION_H
#define WRITEFLAGSACTION_H

#include "action.h"

#include "../imagesets/imageset.h"

#include <stack>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>

#include "../../msio/mask2d.h"

namespace rfiStrategy {
	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class WriteFlagsAction : public Action {
		public:
			WriteFlagsAction();
			virtual ~WriteFlagsAction();

			virtual std::string Description()
			{
				return "Write flags to file";
			}

			virtual void Perform(class ArtifactSet &artifacts, ProgressListener &progress);
			virtual ActionType Type() const { return WriteFlagsActionType; }
			virtual void Finish();
			virtual void Sync() { Finish(); Initialize(); }

			void SetMaxBufferItems(size_t maxBufferItems) { _maxBufferItems = maxBufferItems; }
			void SetMinBufferItemsForWriting(size_t minBufferItemsForWriting) { _minBufferItemsForWriting = minBufferItemsForWriting; }
		private:
			struct BufferItem {
				BufferItem(const std::vector<Mask2DCPtr> &masks, const ImageSetIndex &index)
					: _masks(masks), _index(index.Copy())
				{
				}
				BufferItem(const BufferItem &source) : _masks(source._masks), _index(source._index->Copy())
				{
				}
				~BufferItem()
				{
					delete _index;
				}
				void operator=(const BufferItem &source)
				{
					delete _index;
					_masks = source._masks;
					_index = source._index->Copy();
				}
				std::vector<Mask2DCPtr> _masks;
				ImageSetIndex *_index;
			};

			struct FlushFunction
			{
				WriteFlagsAction *_parent;
				void operator()();
			};

			void pushInBuffer(const BufferItem &newItem)
			{
				boost::mutex::scoped_lock lock(_mutex);
				while(_buffer.size() >= _maxBufferItems)
					_bufferChange.wait(lock);
				_buffer.push(newItem);
				_bufferChange.notify_all();
			}

			boost::mutex _mutex;
			boost::mutex *_ioMutex;
			boost::condition _bufferChange;
			boost::thread *_flusher;
			bool _isFinishing;

			size_t _maxBufferItems;
			size_t _minBufferItemsForWriting;

			std::stack<BufferItem> _buffer;
			ImageSet *_imageSet;
	};
}
#endif
