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

#ifndef RFIACTIONBLOCK_H
#define RFIACTIONBLOCK_H

#include "../control/actioncontainer.h"

#include "../../util/types.h"

namespace rfiStrategy {

	class ActionBlock : public ActionContainer
	{
		public:
			virtual std::string Description()
			{
				return "Block";
			}
			virtual void Perform(class ArtifactSet &artifacts, ProgressListener &listener);

			virtual unsigned int Weight() const
			{
				unsigned int weight = 0;
				for(const_iterator i=begin();i!=end();++i)
				{
					weight += (*i)->Weight();
				}
				return weight;
			}
	};
}

#endif // RFIACTIONBLOCK_H
