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
#ifndef GUI_QUALITY__ANTENNAEPLOTPAGE_H
#define GUI_QUALITY__ANTENNAEPLOTPAGE_H

#include "twodimensionalplotpage.h"

#include "../../quality/statisticscollection.h"

#include "../../msio/measurementset.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AntennaePlotPage : public TwoDimensionalPlotPage {
	protected:
		virtual void processStatistics(class StatisticsCollection *statCollection, const std::vector<AntennaInfo> &antennas)
		{
			_antennas = antennas;
			const BaselineStatisticsMap &map = statCollection->BaselineStatistics();
			
			vector<std::pair<unsigned, unsigned> > baselines = map.BaselineList();
			for(vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
			{
				if(i->first != i->second)
				{
					const DefaultStatistics &stats = map.GetStatistics(i->first, i->second);
					_statistics.insert(std::pair<double, DefaultStatistics>(i->first, stats));
					_statistics.insert(std::pair<double, DefaultStatistics>(i->second, stats));
				}
			}
		}
		
		virtual const std::map<double, class DefaultStatistics> &GetStatistics() const
		{
			return _statistics;
		}
		
		virtual void StartLine(Plot2D &plot, const std::string &name, const std::string &yAxisDesc)
		{
			Plot2DPointSet &pointSet = plot.StartLine(name, "Antenna index", yAxisDesc, false, Plot2DPointSet::DrawColumns);
			
			std::vector<std::string> labels;
			for(std::vector<AntennaInfo>::const_iterator i=_antennas.begin();i!=_antennas.end();++i)
				labels.push_back(i->name);
			pointSet.SetTickLabels(labels);
			pointSet.SetRotateUnits(true);
		}
		
	private:
		std::map<double, DefaultStatistics> _statistics;
		std::vector<AntennaInfo> _antennas;
};

#endif
