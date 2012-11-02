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
#ifndef RFISTRATEGYIMAGERACTION_H
#define RFISTRATEGYIMAGERACTION_H

#include <boost/thread.hpp>

#include "action.h"

#include "../control/artifactset.h"

namespace rfiStrategy {

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class ImagerAction : public Action {
		public:
			enum ImagingType {
				Set, Add, Subtract
			};

			ImagerAction() : _type(Add)
			{
			}
			virtual ~ImagerAction()
			{
			}
			virtual std::string Description()
			{
				switch(_type)
				{
					case Set:
					return "Image (set)";
					case Add:
					default:
					return "Image (add)";
					case Subtract:
					return "Image (subtract)";
				}
			}
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress);

			virtual ActionType Type() const { return ImagerActionType; }
			enum ImagingType ImagingType() const { return _type; }
			void SetImagingType(enum ImagingType type) throw() { _type = type; }

		private:
			enum ImagingType _type;
			boost::mutex _imagerMutex;
	};

}

#endif
