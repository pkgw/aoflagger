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

#ifndef AO_CONCATENATESCRIPT_H
#define AO_CONCATENATESCRIPT_H

#include <iostream>
#include <string>

#include "reffile.h"

namespace AOTools
{
	class ConcatenateScript
	{
		public:
		static void Make(std::ostream &stream, const std::string &refFilePath, const std::string &destination)
		{
			RefFile file(refFilePath);

			stream <<
				"#!/usr/bin/env python\n\n"
				"import pyrap.tables as pt\n\n"
				"pt.msconcat([";

			RefFile::const_iterator i=file.begin();
			if(i!=file.end())
			{
				stream << "\"/net/" << i->Node() << i->Path() << '\"';
				++i;
			}

			while(i!=file.end())
			{
				stream << ", \"/net/" << i->Node() << i->Path() << '\"';
				++i;
			}

			stream << "], \"" << destination << "\")\n"; 
		}
	};
}

#endif // AO_CONCATENATESCRIPT_H
