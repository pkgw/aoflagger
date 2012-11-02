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
#ifndef GUI_QUALITY__FREQUENCYPLOTPAGE_H
#define GUI_QUALITY__FREQUENCYPLOTPAGE_H

#include "twodimensionalplotpage.h"

#include "../../quality/statisticscollection.h"
#include "../../quality/statisticsderivator.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class FrequencyPlotPage : public TwoDimensionalPlotPage {
	public:
    FrequencyPlotPage() : _ftButton("FT")
		{
		}
		
		virtual void processStatistics(class StatisticsCollection *statCollection, const std::vector<AntennaInfo> &antennas)
		{
			_statistics.clear();
			
			const std::map<double, class DefaultStatistics> &map = statCollection->FrequencyStatistics();
			
			for(std::map<double, class DefaultStatistics>::const_iterator i=map.begin();i!=map.end();++i)
			{
				_statistics.insert(std::pair<double, DefaultStatistics>(i->first/1000000.0, i->second));
			}
		}
		
		virtual const std::map<double, class DefaultStatistics> &GetStatistics() const
		{
			return _statistics;
		}
		
		virtual void StartLine(Plot2D &plot, const std::string &name, const std::string &yAxisDesc)
		{
			if(_ftButton.get_active())
				plot.StartLine(name, "Time (μs)", yAxisDesc, false);
			else
				plot.StartLine(name, "Frequency (MHz)", yAxisDesc, false);
		}
		
		virtual void processPlot(Plot2D &plot)
		{
			if(_ftButton.get_active())
			{
				performFt(plot);
			}
		}
		
		virtual void addCustomPlotButtons(Gtk::VBox &container)
		{
			_ftButton.signal_clicked().connect(sigc::mem_fun(*this, &FrequencyPlotPage::onFTButtonClicked));
			container.pack_start(_ftButton);
			_ftButton.show();
		}
	private:
		void onFTButtonClicked()
		{
			updatePlot();
		}
		
		void performFt(Plot2D &plot)
		{
			size_t count = plot.PointSetCount();
			for(size_t line=0;line<count;++line)
			{
				Plot2DPointSet &pointSet = plot.GetPointSet(line);
				std::vector<std::pair<double, std::complex<double> > > output;
				const double min = pointSet.MinX();
				const double width = pointSet.MaxX() - min;
				const double fStart = -2.0 * M_PI * (double) pointSet.Size() / width;
				const double fEnd = 2.0 * M_PI * (double) pointSet.Size() / width;
				const double fStep = (fEnd - fStart) / (double) pointSet.Size();
				for(double f = fStart; f < fEnd ; f += fStep)
				{
					std::pair<double, std::complex<double> > newElement(f/(2.0*M_PI), std::complex<double>(0.0, 0.0));
					std::complex<double> &nextStat = newElement.second;
					for(size_t i=0; i != pointSet.Size(); ++i)
					{
						const double t_f = pointSet.GetX(i) * f;
						const double val = pointSet.GetY(i);
						nextStat += std::complex<double>(val * cos(t_f), val * sin(t_f));
					}
					output.push_back(newElement);
				}
				
				pointSet.Clear();
				for(std::vector<std::pair<double, std::complex<double> > >::const_iterator i=output.begin();i!=output.end();++i)
				{
					double real = i->second.real(), imag=i->second.imag();
					pointSet.PushDataPoint(i->first, sqrt(real*real + imag*imag));
				}
			}
		}
		
		std::map<double, class DefaultStatistics> _statistics;
		Gtk::CheckButton _ftButton;
};

#endif
