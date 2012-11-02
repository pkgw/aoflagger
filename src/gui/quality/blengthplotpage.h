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
#ifndef GUI_QUALITY__BLENGTHPLOTPAGE_H
#define GUI_QUALITY__BLENGTHPLOTPAGE_H

#include "twodimensionalplotpage.h"

#include "../../quality/statisticscollection.h"

#include "../../msio/measurementset.h"

#include "../../remote/clusteredobservation.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class BLengthPlotPage : public TwoDimensionalPlotPage {
	public:
    BLengthPlotPage() :
			_includeAutoCorrelationsButton("Auto-correlations")
		{
		}
	protected:
		virtual void processStatistics(class StatisticsCollection *statCollection, const std::vector<AntennaInfo> &antennas)
		{
			_statisticsWithAutocorrelations.clear();
			_statisticsWithoutAutocorrelations.clear();
			
			const BaselineStatisticsMap &map = statCollection->BaselineStatistics();
			
			vector<std::pair<unsigned, unsigned> > baselines = map.BaselineList();
			for(vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
			{
				Baseline bline(antennas[i->first], antennas[i->second]);
				const DefaultStatistics &statistics = map.GetStatistics(i->first, i->second);
				_statisticsWithAutocorrelations.insert(std::pair<double, DefaultStatistics>(bline.Distance(), statistics));
				if(i->first != i->second)
					_statisticsWithoutAutocorrelations.insert(std::pair<double, DefaultStatistics>(bline.Distance(), statistics));
			}
		}
		
		virtual const std::map<double, class DefaultStatistics> &GetStatistics() const
		{
			return _includeAutoCorrelationsButton.get_active() ? _statisticsWithAutocorrelations : _statisticsWithoutAutocorrelations;
		}
		
		virtual void StartLine(Plot2D &plot, const std::string &name, const std::string &yAxisDesc)
		{
			plot.StartLine(name, "Baseline length (m)", yAxisDesc, false, Plot2DPointSet::DrawPoints);
		}
		virtual void addCustomPlotButtons(Gtk::VBox &container)
		{
			_includeAutoCorrelationsButton.signal_clicked().connect(sigc::mem_fun(*this, &BLengthPlotPage::onAutoCorrelationsClicked));
			container.pack_start(_includeAutoCorrelationsButton);
			_includeAutoCorrelationsButton.show();
		}
	private:
		void onAutoCorrelationsClicked()
		{
			updatePlot();
		}
		
		std::map<double, DefaultStatistics> _statisticsWithAutocorrelations;
		std::map<double, DefaultStatistics> _statisticsWithoutAutocorrelations;
		Gtk::CheckButton _includeAutoCorrelationsButton;
};

#endif
