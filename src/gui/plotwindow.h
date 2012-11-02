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
#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <boost/bind/bind.hpp>

#include <gtkmm/window.h>

#include "plot/plotmanager.h"
#include "plot/plotwidget.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class PlotWindow : public Gtk::Window {
	public:
		PlotWindow(PlotManager &plotManager) : _plotManager(plotManager)
		{
			plotManager.OnUpdate() = boost::bind(&PlotWindow::handleUpdate, this);
			add(_plotWidget);
			_plotWidget.show();
		}
		~PlotWindow()
		{
		}
		
	private:
		void handleUpdate()
		{
			const std::vector<Plot2D*> &plots = _plotManager.Items();
			Plot2D &lastPlot = **plots.rbegin();
			_plotWidget.SetPlot(lastPlot);
			show();
			raise();
		}
		
		PlotWidget _plotWidget;
		PlotManager &_plotManager;
};

#endif
