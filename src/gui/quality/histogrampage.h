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
#ifndef GUI_QUALITY__HISTOGRAMPAGE_H
#define GUI_QUALITY__HISTOGRAMPAGE_H

#include <string>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>
#include <gtkmm/textview.h>

#include "../../quality/qualitytablesformatter.h"

#include "../plot/plot2d.h"
#include "../plot/plotwidget.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class HistogramPage : public Gtk::HBox {
	public:
		HistogramPage();
    ~HistogramPage();

		void SetStatistics(const std::string &filename)
		{
			_statFilename = filename;
			readFromFile();
			updatePlot();
		}
		void SetStatistics(class HistogramCollection &collection);
		void CloseStatistics();
		bool HasStatistics() const
		{
			return _histograms != 0;
		}
	private:
		void addHistogramToPlot(const class LogHistogram &histogram);
		void addRayleighToPlot(const class LogHistogram &histogram, double sigma, double n);
		void addRayleighDifferenceToPlot(const LogHistogram &histogram, double sigma, double n);
		void updatePlot();
		void plotPolarization(const HistogramCollection &histogramCollection, unsigned polarization);
		void plotPolarization(const class LogHistogram &totalHistogram, const class LogHistogram &rfiHistogram);
		void plotFit(const class LogHistogram &histogram, const std::string &title);
		void plotSlope(const class LogHistogram &histogram, const std::string &title, bool useLowerLimit2);
		void onPlotPropertiesClicked();
		void onDataExportClicked();
		void readFromFile();
		void updateSlopeFrame(const LogHistogram &histogram);
		void addSlopeText(std::stringstream &str, const LogHistogram &histogram, bool updateRange);
		void updateDataWindow();
		
		void onAutoRangeClicked()
		{
			bool autoRange = _fitAutoRangeButton.get_active();
			_fitStartEntry.set_sensitive(!autoRange);
			_fitEndEntry.set_sensitive(!autoRange);
			if(autoRange)
				updatePlot();
		}
		
		void onSlopeAutoRangeClicked()
		{
			bool autoRange = _slopeAutoRangeButton.get_active();
			_slopeStartEntry.set_sensitive(!autoRange);
			_slopeEndEntry.set_sensitive(!autoRange);
			if(autoRange)
				updatePlot();
		}
		
		Gtk::Expander _expander;
		Gtk::VBox _sideBox;
		
		Gtk::Frame _histogramTypeFrame;
		Gtk::VBox _histogramTypeBox;
		Gtk::CheckButton _totalHistogramButton, _rfiHistogramButton, _notRFIHistogramButton;
		
		Gtk::Frame _polarizationFrame;
		Gtk::VBox _polarizationBox;
		Gtk::CheckButton _xxPolarizationButton, _xyPolarizationButton, _yxPolarizationButton, _yyPolarizationButton, _sumPolarizationButton;
		
		Gtk::Frame _fitFrame;
		Gtk::VBox _fitBox;
		Gtk::CheckButton _fitButton, _subtractFitButton, _fitLogarithmicButton, _fitAutoRangeButton;
		Gtk::Entry _fitStartEntry, _fitEndEntry;
		Gtk::TextView _fitTextView;
		
		Gtk::Frame _functionFrame;
		Gtk::VBox _functionBox;
		Gtk::RadioButton _nsButton, _dndsButton;
		Gtk::Entry _deltaSEntry;
		
		Gtk::Button _plotPropertiesButton, _dataExportButton;
		
		Gtk::Frame _slopeFrame;
		Gtk::VBox _slopeBox;
		Gtk::TextView _slopeTextView;
		Gtk::CheckButton _drawSlopeButton, _drawSlope2Button;
		Gtk::CheckButton _slopeAutoRangeButton;
		Gtk::Entry _slopeStartEntry, _slopeEndEntry, _slopeRFIRatio;
		
		std::string _statFilename;
		Plot2D _plot;
		PlotWidget _plotWidget;
		class PlotPropertiesWindow *_plotPropertiesWindow;
		class DataWindow *_dataWindow;
		class HistogramCollection *_histograms;
		class HistogramCollection *_summedPolarizationHistograms;
};

#endif
