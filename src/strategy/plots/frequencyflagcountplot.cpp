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
#include "frequencyflagcountplot.h"

#include <fstream>
#include <iomanip>

#include "../../util/aologger.h"
#include "../../util/plot.h"

#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

void FrequencyFlagCountPlot::Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta)
{
	_ignoreFirstChannel = _ignoreFirstChannel && data.ImageHeight() != 1;

	size_t yStart = _ignoreFirstChannel ? 1 : 0;

	for(size_t maskIndex=0;maskIndex<data.MaskCount();++maskIndex)
	{
		Mask2DCPtr mask = data.GetMask(maskIndex);
		for(size_t y=yStart;y<mask->Height();++y)
		{
			double frequency = meta->Band().channels[y].frequencyHz;
			size_t count = 0;

			for(size_t x=0;x<mask->Width();++x)
			{
				if(mask->Value(x, y))
					++count;
			}
			MapItem item = _counts[frequency];
			item.count += count;
			item.total += mask->Width();
			_counts[frequency] = item;
		}
	}
} 

void FrequencyFlagCountPlot::WriteCounts()
{
	std::ofstream file("frequency-vs-counts.txt");
	file << std::setprecision(14);
	for(std::map<double, struct MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
	{
		file << i->first << "\t" << i->second.total << "\t" << i->second.count << "\t" << (100.0L * (long double) i->second.count / (long double) i->second.total) << "\n";
	}
	file.close();
}

void FrequencyFlagCountPlot::MakePlot()
{
	Plot plot("frequency-vs-counts.pdf");
	plot.SetXAxisText("Frequency (MHz)");
	plot.SetYAxisText("Flagged (%)");
	plot.SetYRangeAutoMax(0);
	plot.StartScatter();
	for(std::map<double, struct MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
	{
		plot.PushDataPoint(i->first / 1000000.0L, 100.0L * (long double) i->second.count / (long double) i->second.total);
	}
	plot.Close();
	plot.Show();
}

void FrequencyFlagCountPlot::Report()
{
	if(_counts.size() > 0)
	{
		size_t index = _ignoreFirstChannel ? 1 : 0;
		size_t column = 0;
		std::stringstream rowStr;
		AOLogger::Info
			<< "Summary of RFI per channel: ("
			<< formatFrequency(_counts.begin()->first) << " Hz - "
			<< formatFrequency(_counts.rbegin()->first) << " Hz)\n";
		for(std::map<double, struct MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
		{
			if(column == 0)
			{
				AOLogger::Info << "Channel " << formatIndex(index) << "-";
			}
			std::string percString = formatPercentage(100.0L * (long double) i->second.count / (long double) i->second.total);
			for(unsigned j=percString.size();j<6;++j)
				rowStr << ' ';
			rowStr << percString << '%';
			++column;
			if(column >= 8)
			{
				AOLogger::Info << formatIndex(index) << ":" << rowStr.str() << '\n';
				rowStr.str(std::string());
				column = 0;
			}
			++index;
		}
		if(column != 0)
		{
			AOLogger::Info << formatIndex(index-1) << ":" << rowStr.str() << '\n';
		}
	}
}

std::string FrequencyFlagCountPlot::formatPercentage(double percentage)
{
	std::stringstream s;
	if(percentage >= 1.0)
		s << round(percentage*10.0)/10.0;
	else if(percentage >= 0.1)
		s << round(percentage*100.0)/100.0;
	else
		s << round(percentage*1000.0)/1000.0;
	if(s.str().find('.') == std::string::npos) s << ".0";
	return s.str();
}

std::string FrequencyFlagCountPlot::formatFrequency(double frequencyHz)
{
	std::stringstream s;
	bool commaPlaced = false;
	if(frequencyHz > 1000000000.0)
	{
		s << (int) floor(frequencyHz / 1000000000.0) << ',';
		commaPlaced = true;
	}
	if(frequencyHz > 1000000.0)
	{
		if(commaPlaced)
			formatToThreeDigits(s, (int) floor(fmod(frequencyHz / 1000000.0, 1000.0)));
		else
			s << (int) floor(fmod(frequencyHz / 1000000.0, 1000.0));
		s << ',';
		commaPlaced = true;
	}
	if(frequencyHz > 1000.0)
	{
		if(commaPlaced)
			formatToThreeDigits(s, (int) floor(fmod(frequencyHz / 1000.0, 1000.0)));
		else
			s << (int) floor(fmod(frequencyHz / 1000.0, 1000.0));
		s << ',';
		commaPlaced = true;
	}
	if(commaPlaced)
		formatToThreeDigits(s, (int) floor(fmod(frequencyHz, 1000.0)));
	else
		s << (int) floor(fmod(frequencyHz, 1000.0));
	return s.str();
}
