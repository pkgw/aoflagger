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

#ifndef AOREMOTE__NODE_COMMAND_MAP_H
#define AOREMOTE__NODE_COMMAND_MAP_H

#include <stdexcept>

#include "clusteredobservation.h"

namespace aoRemote {

class NodeCommandMap
{
	public:
		/**
		 * Adds all measurement sets in the observation to the 'command list'. Each node
		 * will receive the list of measurement sets that are stored on the specific
		 * node.
		 */
		void Initialize(const ClusteredObservation &observation)
		{
			const std::vector<ClusteredObservationItem> &items = observation.GetItems();
			for(std::vector<ClusteredObservationItem>::const_iterator i=items.begin();i!=items.end();++i)
			{
				_nodeMap[i->HostName()].push_back(*i);
			}
		}
		
		const ClusteredObservationItem &Top(const std::string &hostname) const
		{
			NodeMap::const_iterator iter = _nodeMap.find(hostname);
			if(iter == _nodeMap.end())
				throw std::runtime_error("Could not find hostname in map. This might mean that your host knows the host under a different name (check case).");
			const std::deque<ClusteredObservationItem> &items = iter->second;
			return items.front();
		}
		
		/**
		 * Removes the top clustered observation item ('command') from the node map and
		 * returns it. The item can be re-requested by calling Current(), until the GetNext()
		 * is called again or the host is removed.
		 * @returns @c true when the node had another item ('command')
		 */
		bool Pop(const std::string &hostname, ClusteredObservationItem &item)
		{
			NodeMap::iterator iter = _nodeMap.find(hostname);
			if(iter == _nodeMap.end())
			{
				return false;
			}
			else {
				std::deque<ClusteredObservationItem> &items = iter->second;
				if(items.empty())
				{
					_nodeMap.erase(iter);
					return false;
				}
				else
				{
					item = items.front();
					items.pop_front();
					_lastItem[hostname] = item;
					return true;
				}
			}
		}
		
		/**
		 * Removes all commands that had to be executed for the given node.
		 * Also removes the 'current' item for this host.
		 * @returns @c true when the hostname was found and removed.
		 */
		bool RemoveNode(const std::string &hostname)
		{
			_lastItem.erase(hostname);
			return _nodeMap.erase(hostname) != 0;
		}
		
		bool Empty() const
		{
			return _nodeMap.empty();
		}
		
		void NodeList(std::vector<std::string> &dest) const
		{
			dest.resize(_nodeMap.size());
			size_t p = 0;
			for(std::map<std::string, std::deque<ClusteredObservationItem> >::const_iterator i=_nodeMap.begin();i!=_nodeMap.end();++i)
			{
				dest[p] = i->first;
				++p;
			}
		}
		
		bool Current(const std::string &hostname, ClusteredObservationItem &item) const
		{
			std::map<std::string, ClusteredObservationItem>::const_iterator iter = _lastItem.find(hostname);
			if(iter == _lastItem.end())
				return false;
			else
			{
				item = iter->second;
				return true;
			}
		}
	private:
		typedef std::map<std::string, std::deque<ClusteredObservationItem> > NodeMap;
		NodeMap _nodeMap;
		
		std::map<std::string, ClusteredObservationItem> _lastItem;
};

}

#endif
