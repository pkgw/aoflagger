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
#ifndef PLOTFRAME_H
#define PLOTFRAME_H

#include <gtkmm/box.h>

#include "../msio/timefrequencydata.h"

#include "plot/plotwidget.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class PlotFrame : public Gtk::HBox {
	public:
		PlotFrame();
		~PlotFrame();

		void SetTimeFrequencyData(const TimeFrequencyData &data)
		{
			_data = data;
		}
		void SetSelectedSample(size_t x, size_t y)
		{
			_selectedXStart = x;
			_selectedYStart = y;
			_selectedXEnd = x+1;
			_selectedYEnd = y+1;
		}
		void Update() { plot(); }
	private:
		TimeFrequencyData _data;
		PlotWidget _plot;
		class Plot2D *_plotData;

		size_t _selectedXStart, _selectedYStart;
		size_t _selectedXEnd, _selectedYEnd;

		void plot();
		void plotTimeGraph(const TimeFrequencyData &data, const std::string &label, enum PolarisationType polarisation);
		void plotTimeGraph(const TimeFrequencyData &data, const std::string &label);
};

#endif
