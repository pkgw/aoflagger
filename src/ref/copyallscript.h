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

#ifndef AO_COPYALLSCRIPT_H
#define AO_COPYALLSCRIPT_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "reffile.h"

namespace AOTools
{
	class CopyAllScript
	{
		public:
		static void Make(std::ostream &stream, const std::string &refFilePath, const std::string &destination)
		{
			typedef std::vector<std::string> PathList;
			typedef std::map<std::string, PathList> PathsPerNodeType;
			typedef std::pair<std::string, PathList> NodeAndPaths;

			std::string correctedDest;
			if(*destination.rbegin() == '/')
				correctedDest = destination;
			else
				correctedDest = destination + "/";

			RefFile file(refFilePath);
			
			PathsPerNodeType pathsPerNode;

			for(RefFile::const_iterator i = file.begin(); i != file.end() ; ++i)
			{
				if(pathsPerNode.count(i->Node()) != 0)
					pathsPerNode.find(i->Node())->second.push_back(i->Path());
				else
				{
					PathList newPathList;
					newPathList.push_back(i->Path());
					pathsPerNode.insert(NodeAndPaths(i->Node(), newPathList));
				} 
			}

			stream <<
				"#! /bin/bash\n"
				"# Created by aorefscript to move sets in \n# " << refFilePath << "\n"
				"# to local path\n# " << correctedDest << "\n"
				"# Set contains " << file.Count() << " MS directories\n"
				"# Number of nodes: " << pathsPerNode.size() << "\n";
			for(PathsPerNodeType::const_iterator i=pathsPerNode.begin(); i!=pathsPerNode.end(); ++i)
			{
				const std::string node = i->first;
				const PathList &paths = i->second;

				stream
					<< "function copy_" << node << " {\n"
					<< "  ssh " << node << " -C \"mkdir -p " << correctedDest << "\"\n";
				for(PathList::const_iterator p=paths.begin();p!=paths.end();++p)
				{
					const std::string &path = *p;
					stream
						<< "  echo " << path << " \\(" << node << "\\)\n"
						<< "  ssh " << node << " -C \"cp -r " << path << " " << correctedDest << "\"\n";
				}
				stream
					<< "}\n\n";
			}
			for(PathsPerNodeType::const_iterator i=pathsPerNode.begin(); i!=pathsPerNode.end(); ++i)
			{
				const std::string node = i->first;
				stream << "copy_" << node << " &\n";
			}
			stream
				<< "wait\n"
				<< "echo All done.\n";
		}
	};
}

#endif // AO_COPYALLSCRIPT_H
