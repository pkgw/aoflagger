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
#ifndef WRITEDATAACTION_H
#define WRITEDATAACTION_H

#include <boost/thread/mutex.hpp>

#include "action.h"

#include "../control/artifactset.h"

#include "../imagesets/imageset.h"

namespace rfiStrategy {
	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class WriteDataAction : public Action {
		public:
			WriteDataAction()
			{
			}

			virtual ~WriteDataAction()
			{
			}

			virtual std::string Description()
			{
				return "Write data to file";
			}

			virtual void Perform(class ArtifactSet &artifacts, ProgressListener &)
			{
				boost::mutex::scoped_lock lock(artifacts.IOMutex());
				ImageSet &set = *artifacts.ImageSet();
				set.PerformWriteDataTask(*artifacts.ImageSetIndex(), artifacts.RevisedData());
			}

			virtual ActionType Type() const
			{
				return WriteDataActionType;
			}

		private:
	};
}
#endif
