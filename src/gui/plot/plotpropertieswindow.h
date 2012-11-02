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
#ifndef PLOTPROPERTIESWINDOW_H
#define PLOTPROPERTIESWINDOW_H

#include <string>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

#include <boost/function.hpp>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class PlotPropertiesWindow : public Gtk::Window {
	public:
		PlotPropertiesWindow(class Plot2D &plot, const std::string &title);
		~PlotPropertiesWindow() { }
		
		boost::function<void()> OnChangesApplied;
	private:
		void onApplyClicked();
		void onCloseClicked();
		void onExportClicked();
		
		void onVRangeChanged()
		{
			_vRangeMinEntry.set_sensitive(_specifiedVRangeButton.get_active());
			_vRangeMaxEntry.set_sensitive(_specifiedVRangeButton.get_active());
		}
		
		void onHRangeChanged()
		{
			_hRangeMinEntry.set_sensitive(!_automaticHRangeButton.get_active());
			_hRangeMaxEntry.set_sensitive(!_automaticHRangeButton.get_active());
		}
		
		void initVRangeWidgets();
		void initHRangeWidgets();
		void initOptionsWidgets();
		
		void updateMinMaxEntries();

		class Plot2D &_plot;

		Gtk::HButtonBox _bottomButtonBox;
		Gtk::VBox _topVBox;
		Gtk::HBox _framesHBox;
		
		Gtk::Button _applyButton, _exportButton, _closeButton;
		Gtk::Image _saveImage;
		
		Gtk::Frame _vRangeFrame;
		Gtk::VBox _vRangeBox;
		Gtk::RadioButton _minMaxVRangeButton, _winsorizedVRangeButton, _specifiedVRangeButton;
		Gtk::Label _vRangeMinLabel, _vRangeMaxLabel;
		Gtk::Entry _vRangeMinEntry, _vRangeMaxEntry;
		
		Gtk::Frame _hRangeFrame;
		Gtk::HBox _hRangeBox;
		Gtk::CheckButton _automaticHRangeButton;
		Gtk::Label _hRangeMinLabel, _hRangeMaxLabel;
		Gtk::Entry _hRangeMinEntry, _hRangeMaxEntry;
		
		Gtk::Frame _optionsFrame;
		Gtk::VBox _optionsBox;
		Gtk::RadioButton _normalOptionsButton, _logScaleButton, _zeroSymmetricButton;
		
		Gtk::CheckButton _showAxes, _showAxisDescriptionsButton;
};

#endif // PLOTPROPERTIESWINDOW_H
