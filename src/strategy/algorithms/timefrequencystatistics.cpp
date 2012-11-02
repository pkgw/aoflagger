/***************************************************************************
 *   Copyright (C) 2008-2010 by A.R. Offringa   *
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
#include "timefrequencystatistics.h"

#include <cmath>
#include <string>
#include <sstream>

TimeFrequencyStatistics::TimeFrequencyStatistics(const TimeFrequencyData &data)
  : _data(data)
{
}

num_t TimeFrequencyStatistics::GetFlaggedRatio()
{
	size_t total = 0, flagged = 0;

	for(size_t i=0;i<_data.MaskCount();++i)
	{
		Mask2DCPtr mask = _data.GetMask(i);
		flagged += mask->GetCount<true>();
		total += mask->Width() * mask->Height();
	}
	if(total != 0)
		return (num_t) flagged / (num_t) total;
	else
		return 0;
}

std::string TimeFrequencyStatistics::FormatRatio(num_t ratio)
{
	std::stringstream s;
	if(ratio > 0.01)
		s << (round(ratio*10000.0)/100.0) << "%";
	else if(ratio > 0.001)
		s << (round(ratio*100000.0)/1000.0) << "%";
	else if(ratio > 0.0001)
		s << (round(ratio*1000000.0)/10000.0) << "%";
	else
		s << (ratio*100.0) << "%";
	return s.str();
}
