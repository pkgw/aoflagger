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
#ifndef HIGHLIGHTWINDOW_H
#define HIGHLIGHTWINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/scale.h>
#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>

#include "../msio/types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class HighlightWindow : public Gtk::Window {
	public:
		HighlightWindow(class RFIGuiWindow &rfiGuiWindow);
		~HighlightWindow();
	private:
		void onValueChange();
		void onHighlightingToggled();

		class RFIGuiWindow &_rfiGuiWindow;
		Gtk::VScale _highlightThresholdHighScale;
		Gtk::VScale _highlightThresholdLowScale;
		Gtk::HScale _connectedCountScale;
		Gtk::VBox _mainBox;
		Gtk::HBox _highlightThresholdBox;
		Gtk::CheckButton _highlightButton;

		num_t _max;
};

#endif
