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
#include "iterationsplot.h"

#include "../../util/aologger.h"
#include "../../util/plot.h"

#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

#include "../algorithms/thresholdtools.h"

void IterationsPlot::Add(TimeFrequencyData &data, TimeFrequencyMetaDataCPtr)
{
	Item item;
	Mask2DCPtr mask = data.GetSingleMask();
	item.mode = ThresholdTools::Mode(data.GetSingleImage(), mask);
	item.winsorizedMode = ThresholdTools::WinsorizedMode(data.GetSingleImage(), mask);
	item.flaggedRatio = (double) mask->GetCount<true>() / ((double) mask->Width() * (double) mask->Height());
	_stats.push_back(item);
}

void IterationsPlot::MakePlot()
{
	Plot plotA("iterations-flags.pdf");
	plotA.SetXAxisText("Iteration number");
	plotA.SetYAxisText("Flagged (%)");
	plotA.StartLine();
	plotA.SetXRange(1, _stats.size());
	for(unsigned i=0;i<_stats.size();++i)
	{
		plotA.PushDataPoint(i+1, _stats[i].flaggedRatio * 100.0);
	}
	plotA.Close();
	plotA.Show();

	Plot plotB("iterations-modes.pdf");
	plotB.SetXAxisText("Iteration number");
	plotB.SetYAxisText("Mode");
	plotB.StartLine("Normal mode");
	plotB.SetXRange(1, _stats.size());
	for(unsigned i=0;i<_stats.size();++i)
	{
		plotB.PushDataPoint(i+1, _stats[i].mode);
	}
	plotB.StartLine("Winsorized mode");
	for(unsigned i=0;i<_stats.size();++i)
	{
		plotB.PushDataPoint(i+1, _stats[i].winsorizedMode);
	}
	plotB.Close();
	plotB.Show();
}
