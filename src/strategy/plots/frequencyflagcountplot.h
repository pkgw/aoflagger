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
#ifndef FREQUENCYFLAGCOUNTPLOT_H
#define FREQUENCYFLAGCOUNTPLOT_H

#include <map>

#include "../../msio/timefrequencymetadata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class FrequencyFlagCountPlot{
	public:
		FrequencyFlagCountPlot() : _ignoreFirstChannel(true) { }
		~FrequencyFlagCountPlot() { }

		void Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta);
		void WriteCounts();
		bool HasData() { return !_counts.empty(); }
		void MakePlot();
		void Report();
	private:
		struct MapItem {
			MapItem() : count(0), total(0) { }
			long long count, total;
		};
		std::string formatPercentage(double percentage);
		std::string formatFrequency(double frequencyHz);
		void formatToThreeDigits(std::stringstream &stream, int number)
		{
			if(number < 100) stream << '0';
			if(number < 10) stream << '0';
			stream << number;
		}
		std::string formatIndex(int index)
		{
			std::stringstream s;
			if(index < 100) s << ' ';
			if(index < 10) s << ' ';
			s << index;
			return s.str();
		}
		// In lofar, the first channel of every subband is flagged, because it overlaps with
		// the previous subband. 
		bool _ignoreFirstChannel;

		std::map<double, struct MapItem> _counts;
};

#endif
