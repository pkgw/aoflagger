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
#include <fstream>
#include <iomanip>

#include "timeflagcountplot.h"

#include "../../util/plot.h"

#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

void TimeFlagCountPlot::Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta)
{
	const std::vector<double> &times = meta->ObservationTimes();
	if(!HasData())
	{
		_startTime = times[0];
	}
	for(size_t maskIndex=0;maskIndex<data.MaskCount();++maskIndex)
	{
		Mask2DCPtr mask = data.GetMask(maskIndex);
		for(size_t x=0;x<mask->Width();++x)
		{
			double time = times[x] - _startTime;
			size_t count = 0;

			for(size_t y=0;y<mask->Height();++y)
			{
				if(mask->Value(x, y))
					++count;
			}
			MapItem item = _counts[time];
			item.count += count;
			item.total += mask->Height();
			_counts[time] = item;
		}
	}
	WriteCounts();
} 

void TimeFlagCountPlot::WriteCounts()
{
	std::ofstream file("time-vs-counts.txt");
	file << std::setprecision(14);
	for(std::map<double, struct MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
	{
		file << i->first << "\t" << i->second.total << "\t" << i->second.count << "\t" << (100.0L * (long double) i->second.count / (long double) i->second.total) << "\n";
	}
	file.close();
}

void TimeFlagCountPlot::MakePlot()
{
	Plot plot("time-vs-counts.pdf");
	plot.SetXAxisText("Time (s)");
	plot.SetYAxisText("Flagged (%)");
	plot.SetYRangeAutoMax(0);
	plot.StartScatter();
	for(std::map<double, struct MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
	{
		plot.PushDataPoint(i->first, 100.0L * (long double) i->second.count / (long double) i->second.total);
	}
	plot.Close();
	plot.Show();
}
