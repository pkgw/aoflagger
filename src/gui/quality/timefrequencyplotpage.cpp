/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include <limits>

#include "timefrequencyplotpage.h"

#include "../../quality/statisticscollection.h"
#include "../../quality/statisticsderivator.h"

TimeFrequencyPlotPage::TimeFrequencyPlotPage() :
	_statCollection(0)
{
	GrayScaleWidget().OnMouseMovedEvent().connect(sigc::mem_fun(*this, &TimeFrequencyPlotPage::onMouseMoved));
	GrayScaleWidget().SetXAxisDescription("Time index");
	GrayScaleWidget().SetYAxisDescription("Frequency index");
}

TimeFrequencyPlotPage::~TimeFrequencyPlotPage()
{
}

std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr> TimeFrequencyPlotPage::ConstructImage()
{
	if(HasStatistics())
	{
		const QualityTablesFormatter::StatisticKind kind = GetSelectedStatisticKind();
		
		StatisticsDerivator derivator(*_statCollection);
		
		std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr> data = derivator.CreateTFData(kind);
		if(data.second == 0)
		{
			GrayScaleWidget().SetXAxisDescription("Time index");
			GrayScaleWidget().SetYAxisDescription("Frequency index");
		} else {
			GrayScaleWidget().SetXAxisDescription("Time");
			GrayScaleWidget().SetYAxisDescription("Frequency (MHz)");
		}
		return data;
	} else {
		return std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr>(TimeFrequencyData(), TimeFrequencyMetaDataCPtr());
	}
}

void TimeFrequencyPlotPage::onMouseMoved(size_t x, size_t y)
{
	std::stringstream text;

	const QualityTablesFormatter::StatisticKind kind = GetSelectedStatisticKind();
	const std::string &kindName = QualityTablesFormatter::KindToName(kind);
	
	text << kindName << " = " << GrayScaleWidget().Image()->Value(x, y) << " (" << x << ", " << y << ")";
	_signalStatusChange(text.str());
}
