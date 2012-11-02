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
#ifndef FREQUENCYPOWERPLOT_H
#define FREQUENCYPOWERPLOT_H

#include <map>
#include <string>

#include "../../msio/timefrequencymetadata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class FrequencyPowerPlot {
	public:
		FrequencyPowerPlot() : _plot(0), _logYAxis(false) { }
		~FrequencyPowerPlot();

		void Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta);
		void WriteToText();
		bool HasData() { return !_values.empty(); }
		void MakePlot();
		void StartNewLine(const std::string &lineTitle);
		void SetLogYAxis(bool logYAxis) { _logYAxis = logYAxis; }
	private:
		void AddCurrentLine(class Plot &plot);

		struct MapItem {
			MapItem() : count(0), total(0.0L) { }
			long long count;
			long double total;
		};
		std::map<double, struct MapItem> _values;
		std::string _currentLineText;
		class Plot *_plot;
		bool _logYAxis;
};

#endif
