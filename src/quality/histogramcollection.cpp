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
#include "histogramcollection.h"

#include "histogramtablesformatter.h"

#include "../gui/plot/plot2d.h"

void HistogramCollection::Save(HistogramTablesFormatter &histogramTables)
{
	histogramTables.InitializeEmptyTables();
	for(size_t p=0;p<_polarizationCount;++p)
	{
		LogHistogram totalHistogram;
		GetTotalHistogramForCrossCorrelations(p, totalHistogram);
		const unsigned totalIndex = histogramTables.StoreOrQueryTypeIndex(HistogramTablesFormatter::TotalHistogram, p);
		for(LogHistogram::iterator i=totalHistogram.begin();i!=totalHistogram.end();++i)
		{
			histogramTables.StoreValue(totalIndex, i.binStart(), i.binEnd(), i.unnormalizedCount());
		}
		
		LogHistogram rfiHistogram;
		GetRFIHistogramForCrossCorrelations(p, rfiHistogram);
		const unsigned rfiIndex = histogramTables.StoreOrQueryTypeIndex(HistogramTablesFormatter::RFIHistogram, p);
		for(LogHistogram::iterator i=rfiHistogram.begin();i!=rfiHistogram.end();++i)
		{
			histogramTables.StoreValue(rfiIndex, i.binStart(), i.binEnd(), i.unnormalizedCount());
		}
	}
}

void HistogramCollection::Load(HistogramTablesFormatter &histogramTables)
{
	Clear();
	for(unsigned p=0;p<_polarizationCount;++p)
	{
		const unsigned totalHistogramIndex = histogramTables.QueryTypeIndex(HistogramTablesFormatter::TotalHistogram, p);
		std::vector<HistogramTablesFormatter::HistogramItem> totalHistogram;
		histogramTables.QueryHistogram(totalHistogramIndex, totalHistogram);
		GetTotalHistogram(0, 1, p).SetData(totalHistogram);

		const unsigned rfiHistogramIndex = histogramTables.QueryTypeIndex(HistogramTablesFormatter::RFIHistogram, p);
		std::vector<HistogramTablesFormatter::HistogramItem> rfiHistogram;
		histogramTables.QueryHistogram(rfiHistogramIndex, rfiHistogram);
		GetRFIHistogram(0, 1, p).SetData(rfiHistogram);
	}
}

void HistogramCollection::Add(const unsigned antenna1, const unsigned antenna2, const unsigned polarization, Image2DCPtr image, Mask2DCPtr mask)
{
	LogHistogram &totalHistogram = GetTotalHistogram(antenna1, antenna2, polarization);
	LogHistogram &rfiHistogram = GetRFIHistogram(antenna1, antenna2, polarization);
	
	for(size_t y=0;y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			const double amplitude = image->Value(x, y);
			totalHistogram.Add(amplitude);
			if(mask->Value(x, y))
				rfiHistogram.Add(amplitude);
		}
	}
}

void HistogramCollection::Plot(class Plot2D &plot, unsigned polarization)
{
	LogHistogram totalHistogram, rfiHistogram;
	GetTotalHistogramForCrossCorrelations(polarization, totalHistogram);
	GetRFIHistogramForCrossCorrelations(polarization, rfiHistogram);
	
	plot.StartLine("Total");
	for(LogHistogram::iterator i=totalHistogram.begin();i!=totalHistogram.end();++i)
	{
		const double x = i.value();
		const double logx = log10(x);
		const double logc = log10(i.normalizedCount());
		if(std::isfinite(logx) && std::isfinite(logc))
			plot.PushDataPoint(logx, logc);
	}

	plot.StartLine("RFI");
	for(LogHistogram::iterator i=rfiHistogram.begin();i!=rfiHistogram.end();++i)
	{
		const double x = i.value();
		const double logx = log10(x);
		const double logc = log10(i.normalizedCount());
		if(std::isfinite(logx) && std::isfinite(logc))
			plot.PushDataPoint(logx, logc);
	}
	
	plot.StartLine("Non RFI");
	for(LogHistogram::iterator i=totalHistogram.begin();i!=totalHistogram.end();++i)
	{
		const double x = i.value();
		const double logx = log10(x);
		const double logc = log10(i.normalizedCount() - rfiHistogram.NormalizedCount(x));
		if(std::isfinite(logx) && std::isfinite(logc))
			plot.PushDataPoint(logx, logc);
	}
}


