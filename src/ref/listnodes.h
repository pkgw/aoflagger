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

#ifndef AO_LISTNODES_H
#define AO_LISTNODES_H

#include <iostream>
#include <set>
#include <string>

#include "reffile.h"

namespace AOTools
{
	class ListNodes
	{
		public:
		static void Make(std::ostream &stream, const std::string &refFilePath)
		{
			RefFile file(refFilePath);
			std::set<std::string> nodes;
			
			for(RefFile::const_iterator i = file.begin(); i != file.end() ; ++i)
			{
				nodes.insert(i->Node());
			}

			for(std::set<std::string>::const_iterator i = nodes.begin(); i!=nodes.end(); ++i)
			{
				stream << *i << "\n";
			}
		}
	};
}

#endif // AO_COPYALLSCRIPT_H
