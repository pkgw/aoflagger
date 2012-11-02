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

#include "baselineplotpage.h"

#include "../../quality/statisticscollection.h"
#include "../../quality/statisticsderivator.h"

BaselinePlotPage::BaselinePlotPage() :
	_statCollection(0),
	_antennas(0)
{
	GrayScaleWidget().OnMouseMovedEvent().connect(sigc::mem_fun(*this, &BaselinePlotPage::onMouseMoved));
	GrayScaleWidget().SetXAxisDescription("Antenna 1 index");
	GrayScaleWidget().SetYAxisDescription("Antenna 2 index");
}

BaselinePlotPage::~BaselinePlotPage()
{
}

std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr> BaselinePlotPage::ConstructImage()
{
	if(HasStatistics())
	{
		const QualityTablesFormatter::StatisticKind kind = GetSelectedStatisticKind();
		
		const unsigned polarizationCount = _statCollection->PolarizationCount();
		std::vector<std::pair<unsigned, unsigned> > baselines = _statCollection->BaselineStatistics().BaselineList();
		StatisticsDerivator derivator(*_statCollection);
		
		const unsigned antennaCount = _statCollection->BaselineStatistics().AntennaCount();

		Image2DPtr realImages[polarizationCount];
		Image2DPtr imagImages[polarizationCount];
		Mask2DPtr mask[polarizationCount];
		for(unsigned p=0;p<polarizationCount;++p)
		{
			realImages[p] = Image2D::CreateUnsetImagePtr(antennaCount, antennaCount);
			realImages[p]->SetAll(std::numeric_limits<num_t>::quiet_NaN());
			imagImages[p] = Image2D::CreateUnsetImagePtr(antennaCount, antennaCount);
			imagImages[p]->SetAll(std::numeric_limits<num_t>::quiet_NaN());
			mask[p] = Mask2D::CreateSetMaskPtr<true>(antennaCount, antennaCount);
		}
		
		for(std::vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
		{
			const unsigned antenna1 = i->first, antenna2 = i->second;
			for(unsigned p=0;p<polarizationCount;++p)
			{
				const std::complex<long double> val = derivator.GetComplexBaselineStatistic(kind, antenna1, antenna2, p);
				realImages[p]->SetValue(antenna1, antenna2, val.real());
				imagImages[p]->SetValue(antenna1, antenna2, val.imag());
				mask[p]->SetValue(antenna1, antenna2, false);
			}
		}
		TimeFrequencyData data;
		if(polarizationCount == 1)
		{
			data = TimeFrequencyData(TimeFrequencyData::ComplexRepresentation, SinglePolarisation, realImages[0], imagImages[0]);
			data.SetGlobalMask(mask[0]);
		}
		else if(polarizationCount == 2)
		{
			data = TimeFrequencyData(AutoDipolePolarisation, realImages[0], imagImages[0], realImages[1], imagImages[1]);
			data.SetIndividualPolarisationMasks(mask[0], mask[1]);
		}
		else if(polarizationCount == 4)
		{
			data = TimeFrequencyData(realImages[0], imagImages[0], realImages[1], imagImages[1], realImages[2], imagImages[2], realImages[3], imagImages[3]);
			data.SetIndividualPolarisationMasks(mask[0], mask[1], mask[2], mask[3]);
		}
		else {
			std::stringstream s;
			s << "Set has not 1, 2 or 4 polarizations (?!?) : StatisticsCollection.PolarizationCount() == " << polarizationCount;
			throw std::runtime_error(s.str());
		}
		return std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr>(data, TimeFrequencyMetaDataCPtr());
	} else {
		return std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr>(TimeFrequencyData(), TimeFrequencyMetaDataCPtr());
	}
}

void BaselinePlotPage::onMouseMoved(size_t x, size_t y)
{
	std::stringstream text;
	std::string antenna1Name, antenna2Name;
	if(_antennas == 0)
	{
		std::stringstream a1, a2;
		a1 << x;
		a2 << y;
		antenna1Name = a1.str();
		antenna2Name = a2.str();
	} else {
		antenna1Name = (*_antennas)[x].name;
		antenna2Name = (*_antennas)[y].name;
	}
	const QualityTablesFormatter::StatisticKind kind = GetSelectedStatisticKind();
	const std::string &kindName = QualityTablesFormatter::KindToName(kind);
	
	text << "Correlation " << antenna1Name << " x " << antenna2Name << ", " << kindName << " = " << GrayScaleWidget().Image()->Value(x, y);
	_signalStatusChange(text.str());
}
