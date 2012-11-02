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
#ifndef IMAGEPROPERTIESWINDOW_H
#define IMAGEPROPERTIESWINDOW_H

#include <string>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ImagePropertiesWindow : public Gtk::Window {
	public:
		ImagePropertiesWindow(class ImageWidget &imageWidget, const std::string &title);
		~ImagePropertiesWindow() { }
	private:
		void onApplyClicked();
		void onCloseClicked();
		void onExportClicked();
		
		void onScaleChanged()
		{
			_scaleMinEntry.set_sensitive(_specifiedScaleButton.get_active());
			_scaleMaxEntry.set_sensitive(_specifiedScaleButton.get_active());
		}
		
		void initColorMapButtons();
		void initScaleWidgets();
		void initOptionsWidgets();
		void initZoomWidgets();
		void initAxisWidgets();
		
		void updateMinMaxEntries();

		class ImageWidget &_imageWidget;

		Gtk::HButtonBox _bottomButtonBox;
		Gtk::VBox _topVBox;
		Gtk::HBox _framesHBox;
		
		Gtk::Button _applyButton, _exportButton, _closeButton;
		Gtk::Image _saveImage;
		Gtk::Frame _colorMapFrame;
		Gtk::VBox _colorMapBox;
		Gtk::RadioButton _grayScaleButton, _invGrayScaleButton, _hotColdScaleButton, _redBlueScaleButton, _blackRedScaleButton, _redBlueYellowScaleButton;
		
		Gtk::Frame _scaleFrame;
		Gtk::VBox _scaleBox;
		Gtk::RadioButton _minMaxScaleButton, _winsorizedScaleButton, _specifiedScaleButton;
		Gtk::Label _scaleMinLabel, _scaleMaxLabel;
		Gtk::Entry _scaleMinEntry, _scaleMaxEntry;
		
		Gtk::Frame _optionsFrame;
		Gtk::VBox _optionsBox;
		Gtk::RadioButton _normalOptionsButton, _logScaleButton, _zeroSymmetricButton;
		
		Gtk::Frame _zoomFrame;
		Gtk::HBox _zoomHBox;
		Gtk::VBox _zoomVSubBox;
		Gtk::HScale _hStartScale, _hStopScale;
		Gtk::VScale _vStartScale, _vStopScale;
		
		Gtk::Frame _axesFrame;
		Gtk::HBox _axesHBox;
		Gtk::VBox _axesGeneralBox;
		Gtk::VBox _axesVisibilityBox;
		Gtk::CheckButton _showXYAxes, _showColorScale;
		Gtk::HBox _xAxisBox, _yAxisBox, _zAxisBox;
		Gtk::CheckButton _showXAxisDescriptionButton, _showYAxisDescriptionButton, _showZAxisDescriptionButton;
		Gtk::CheckButton _manualXAxisDescription, _manualYAxisDescription, _manualZAxisDescription;
		Gtk::Entry _xAxisDescriptionEntry, _yAxisDescriptionEntry, _zAxisDescriptionEntry;
};

#endif // IMAGEPROPERTIESWINDOW_H
