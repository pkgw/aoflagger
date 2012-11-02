/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#ifndef AO_LISTSETS_H
#define AO_LISTSETS_H

#include <iostream>
#include <set>
#include <string>
#include <boost/filesystem.hpp>

#include "reffile.h"

namespace AOTools
{
	class ListSets
	{
		public:
		static void Make(std::ostream &stream, const std::string &refFilePath)
		{
			RefFile file(refFilePath);
			std::set<std::string> nodes;
			
			for(RefFile::const_iterator i = file.begin(); i != file.end() ; ++i)
			{
				boost::filesystem::path entryPath("/net");
				entryPath = entryPath / i->Node() / i->Path();
				stream << entryPath << "\n";
			}
		}
	};
}

#endif // AO_COPYALLSCRIPT_H
