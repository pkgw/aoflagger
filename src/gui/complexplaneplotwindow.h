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
#ifndef COMPLEXPLANEPLOTWINDOW_H
#define COMPLEXPLANEPLOTWINDOW_H

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ComplexPlanePlotWindow : public Gtk::Window {
	public:
		ComplexPlanePlotWindow(class MSWindow &_msWindow, class PlotManager &plotManager);
		~ComplexPlanePlotWindow();
	private:
		size_t XStart() const throw() { return (size_t) _xPositionScale.get_value(); }
		size_t XLength() const throw() { return (size_t) _lengthScale.get_value(); }

		size_t YStart() const throw() { return (size_t) _yPositionScale.get_value(); }
		size_t YLength() const throw() { return (size_t) _ySumLengthScale.get_value(); }

		void onPlotPressed();
		void onTimeStartChanged() {
			if(XStart() + XLength() > _xMax)
				_lengthScale.set_value(_xMax - XStart());
			else
				setDetailsLabel();
		}
		void onTimeDurationChanged() {
			if(XStart() + XLength() > _xMax)
				_xPositionScale.set_value(_xMax - XLength());
			else
				setDetailsLabel();
		}
		void onFreqChanged() {
			if(YStart() + YLength() > _yMax)
				_ySumLengthScale.set_value(_yMax - YStart());
			else
				setDetailsLabel();
		}
		void onFreqSizeChanged() {
			if(YStart() + YLength() > _yMax)
				_yPositionScale.set_value(_yMax - YLength());
			else
				setDetailsLabel();
		}
		void setDetailsLabel();

		class MSWindow &_msWindow;
		class PlotManager &_plotManager;
		Gtk::Frame _detailsFrame;
		Gtk::VBox _mainBox, _detailsBox;
		Gtk::Label
			_detailsLabel,
			_xPositionLabel, _yPositionLabel,
			_lengthLabel, _ySumLengthLabel;
		Gtk::HScale
			_xPositionScale, _yPositionScale, 
			_lengthScale, _ySumLengthScale;
		Gtk::RadioButton _realVersusImaginaryButton, _timeVersusRealButton;
		Gtk::CheckButton _allValuesButton, _unmaskedValuesButton, _maskedValuesButton, _fittedValuesButton, _individualSampleFitButton, _fringeFitButton, _dynamicFringeFitButton;

		Gtk::Button _plotButton;
		Gtk::HButtonBox _buttonBox;
		std::vector<double> _observationTimes;
		size_t _xMax, _yMax;
};

#endif
