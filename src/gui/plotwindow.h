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

#include <gtkmm/box.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "plot/plotwidget.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class PlotWindow : public Gtk::Window {
	public:
		PlotWindow(class PlotManager &plotManager);
		
		~PlotWindow()
		{
		}
		
	private:
		class PlotListColumns : public Gtk::TreeModel::ColumnRecord
		{
		public:
			PlotListColumns()
			{ add(_index); add(_name); }

			Gtk::TreeModelColumn<unsigned int> _index;
			Gtk::TreeModelColumn<Glib::ustring> _name;
		} _plotListColumns;
		
		void handleUpdate();
		void updatePlotList();
		
		PlotWidget _plotWidget;
		class PlotManager &_plotManager;
		Gtk::HBox _hBox;
		Glib::RefPtr<Gtk::ListStore> _plotListStore;
		Gtk::TreeView _plotListView;
};

#endif
