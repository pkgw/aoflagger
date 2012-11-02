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

#include "parmimageset.h"

#include <set>

#include "../../msio/parmtable.h"

namespace rfiStrategy
{
	ParmImageSet::~ParmImageSet()
	{
		if(_parmTable != 0)
			delete _parmTable;
	}
			
	void ParmImageSet::Initialize()
	{
		_parmTable = new ParmTable(_path);
		const std::set<std::string> antennaSet = _parmTable->GetAntennas();
		for(std::set<std::string>::const_iterator i=antennaSet.begin();i!=antennaSet.end();++i)
			_antennas.push_back(*i);
	}
	
	TimeFrequencyData *ParmImageSet::LoadData(const ImageSetIndex &index)
	{
		const ParmImageSetIndex parmIndex = static_cast<const ParmImageSetIndex &>(index);
		const std::string antenna = _antennas[parmIndex.AntennaIndex()];
		return new TimeFrequencyData(_parmTable->Read(antenna));
	}
}

