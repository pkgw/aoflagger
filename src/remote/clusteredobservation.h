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

#ifndef AOREMOTE__CLUSTERED_OBSERVATION_H
#define AOREMOTE__CLUSTERED_OBSERVATION_H

#include <string>
#include <vector>
#include <map>

namespace aoRemote
{

class ClusteredObservationItem
{
	public:
		ClusteredObservationItem() :
			_index(0), _localPath(), _hostName()
		{ }
		ClusteredObservationItem(unsigned index, const std::string &localPath, const std::string &hostName) :
			_index(index), _localPath(localPath), _hostName(hostName)
		{ }
		ClusteredObservationItem(const ClusteredObservationItem &source) :
			_index(source._index), _localPath(source._localPath), _hostName(source._hostName)
		{ }
		ClusteredObservationItem &operator=(const ClusteredObservationItem &source)
		{
			_index = source._index;
			_localPath = source._localPath;
			_hostName = source._hostName;
			return *this;
		}
		bool operator<(const ClusteredObservationItem &rhs) const
		{
			return _index < rhs._index;
		}
		unsigned Index() const { return _index; }
		const std::string &LocalPath() const { return _localPath; }
		const std::string &HostName() const { return _hostName; }
	private:
		unsigned _index;
		std::string _localPath;
		std::string _hostName;
};

class ClusteredObservation
{
	public:
		ClusteredObservation();
		
		static bool IsClusteredFilename(const std::string &filename)
		{
			return IsVdsFilename(filename) || IsRefFilename(filename);
		}
		
		static bool IsVdsFilename(const std::string &filename);
		
		static bool IsRefFilename(const std::string &filename);
		
		static ClusteredObservation *Load(const std::string &filename);
		
		static ClusteredObservation *LoadFromVds(const std::string &vdsFilename);
		
		static ClusteredObservation *LoadFromRef(const std::string &refFilename);
		
		void AddItem(const ClusteredObservationItem &item)
		{
			_items.push_back(item);
		}
		
		const std::vector<ClusteredObservationItem> &GetItems() const
		{
			return _items;
		}
		
		static bool IsRemoteModuleEnabled();
		
		size_t Size() const { return _items.size(); }
		
	private:
		// disable copying
		ClusteredObservation(const ClusteredObservation &source)
		{
		}
		// disable assignment
		void operator=(const ClusteredObservation &source)
		{
		}
		
		static void throwIfNotEnabled();
		
		std::vector<ClusteredObservationItem> _items;
};


}

#endif
