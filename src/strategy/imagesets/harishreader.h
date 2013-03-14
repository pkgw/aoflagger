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

#ifndef HARISHREADER_H
#define HARISHREADER_H

#include <string>
#include <fstream>
#include <set>
#include <map>
#include <cmath>

#include "../../msio/types.h"

#include "singleimageset.h"

#include "../../util/aologger.h"

namespace rfiStrategy {

	class HarishReader : public SingleImageSet {
		public:
			HarishReader(const std::string &path) : SingleImageSet(), _path(path)
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
				return "Harish format";
			}
			
			virtual std::string File()
			{
				return _path;
			}
			
			virtual BaselineData *Read()
			{
				size_t timeStepCount = 176190;
				Image2DPtr image = Image2D::CreateZeroImagePtr(timeStepCount, 80);
				std::ifstream file(_path.c_str());
				for(size_t f=0;f<80;++f)
				{
					std::vector<double> buffer(timeStepCount);
					file.read(reinterpret_cast<char *>(&buffer[0]), timeStepCount * sizeof(double));
					for(size_t t=0;t<timeStepCount;++t)
					{
						image->SetValue(t, f, buffer[t]);
					}
				}
				
				TimeFrequencyData tfData(TimeFrequencyData::AmplitudePart,
					SinglePolarisation,
					image);
				return new BaselineData(tfData, TimeFrequencyMetaDataCPtr());
			}

		private:
			std::string _path;
	};
	
}

#endif
