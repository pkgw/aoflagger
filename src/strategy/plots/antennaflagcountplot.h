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
#ifndef ANTENNAFLAGCOUNTPLOT_H
#define ANTENNAFLAGCOUNTPLOT_H

#include <string>
#include <map>

#include "../../msio/timefrequencymetadata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AntennaFlagCountPlot {
	public:
		AntennaFlagCountPlot() { }
		~AntennaFlagCountPlot() { }

		void Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta);
		void MakePlot();
		bool HasData() { return !_counts.empty(); }
		void Report();
	private:
		void WriteCounts();
		std::string formatPercentage(double percentage);

		struct MapItem {
			MapItem() : name(), autoCount(0), autoTotal(0), crossCount(0), crossTotal(0)
			{
			}
			MapItem(const MapItem &source) : name(source.name), autoCount(source.autoCount), autoTotal(source.autoTotal), crossCount(source.crossCount), crossTotal(source.crossTotal)
			{
			}
			MapItem &operator=(const MapItem &source)
			{
				name = source.name;
				autoCount = source.autoCount;
				autoTotal = source.autoTotal;
				crossCount = source.crossCount;
				crossTotal = source.crossTotal;
				return *this;
			}
			std::string name;
			long long autoCount, autoTotal, crossCount, crossTotal;
		};
		std::map<int, MapItem> _counts;
};

#endif
