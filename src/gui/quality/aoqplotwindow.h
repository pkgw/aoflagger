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
#ifndef AOQPLOT_WINDOW_H
#define AOQPLOT_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/notebook.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/window.h>

#include "../imagewidget.h"

#include "../../quality/qualitytablesformatter.h"

#include "antennaeplotpage.h"
#include "baselineplotpage.h"
#include "blengthplotpage.h"
#include "frequencyplotpage.h"
#include "histogrampage.h"
#include "openoptionswindow.h"
#include "summarypage.h"
#include "timefrequencyplotpage.h"
#include "timeplotpage.h"

#include "../../msio/antennainfo.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AOQPlotWindow : public Gtk::Window {
	public:
		AOQPlotWindow();
    virtual ~AOQPlotWindow()
    { 
			close();
		}
    
		void Open(const std::string &filename);
		void SetStatus(const std::string &newStatus)
		{
			onStatusChange(newStatus);
		}
	private:
		void onOpenOptionsSelected(std::string filename, bool downsampleTime, bool downsampleFreq, size_t timeSize, size_t freqSize, bool correctHistograms);
		void close();
		void readStatistics(bool downsampleTime, bool downsampleFreq, size_t timeSize, size_t freqSize, bool correctHistograms);
		void onHide()
		{
			Gtk::Main::quit();
		}
		void onStatusChange(const std::string &newStatus);
		void onSwitchPage(GtkNotebookPage *page, guint pageNr)
		{
			switch(pageNr)
			{
				case 0: SetStatus("Baseline statistics"); break;
				case 1: SetStatus("Antennae statistics"); break;
				case 2: SetStatus("Baseline length statistics");  break;
				case 3: SetStatus("Time statistics"); break;
				case 4: SetStatus("Frequency statistics"); break;
				case 5: SetStatus("Time-frequency statistics");  break;
				case 6: SetStatus("Summary"); break;
			}
		}
		void setShowHistograms(bool show)
		{
			_histogramPage.set_visible(show);
		}
		
		Gtk::VBox _vBox;
		Gtk::Notebook _notebook;
		Gtk::Statusbar _statusBar;
		
		BaselinePlotPage _baselinePlotPage;
		AntennaePlotPage _antennaePlotPage;
		BLengthPlotPage  _bLengthPlotPage;
		TimeFrequencyPlotPage _timeFrequencyPlotPage;
		TimePlotPage _timePlotPage;
		FrequencyPlotPage _frequencyPlotPage;
		SummaryPage _summaryPage;
		HistogramPage _histogramPage;
		
		OpenOptionsWindow _openOptionsWindow;

		bool _isOpen;
		std::string _filename;
		class StatisticsCollection *_statCollection;
		class HistogramCollection *_histCollection;
		class StatisticsCollection *_fullStats;
		std::vector<class AntennaInfo> _antennas;
};

#endif
