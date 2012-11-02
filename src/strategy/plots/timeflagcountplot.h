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
#ifndef TIMEFLAGCOUNTPLOT_H
#define TIMEFLAGCOUNTPLOT_H

#include <map>

#include "../../msio/timefrequencymetadata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class TimeFlagCountPlot {
	public:
		TimeFlagCountPlot() : _startTime(0) { };
		~TimeFlagCountPlot() { }

		void Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta);
		bool HasData() { return !_counts.empty(); }
		void MakePlot();
	private:
		void WriteCounts();
		struct MapItem {
			MapItem() : count(0), total(0) { }
			long long count, total;
		};

		std::map<double, struct MapItem> _counts;
		double _startTime;
};

#endif // TIMEFLAGCOUNTPLOT_H
