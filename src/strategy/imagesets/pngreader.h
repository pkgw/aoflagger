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

#ifndef PNGREADER_H
#define PNGREADER_H

#include <string>
#include <fstream>
#include <set>
#include <map>
#include <cmath>

#include "../../msio/types.h"

#include "singleimageset.h"

#include "../../util/aologger.h"

namespace rfiStrategy {

	class PngReader : public SingleImageSet {
		public:
			PngReader(const std::string &path) : SingleImageSet(), _path(path)
			{
			}

			virtual ImageSet *Copy()
			{
				return 0;
			}

			virtual void Initialize()
			{
			}

			virtual std::string Name()
			{
				return "Png format";
			}
			
			virtual std::string File()
			{
				return _path;
			}
			
			virtual BaselineData *Read();

		private:
			std::string _path;
	};
	
}

#endif
