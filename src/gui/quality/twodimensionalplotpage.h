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
#ifndef GUI_QUALITY__2DPLOTPAGE_H
#define GUI_QUALITY__2DPLOTPAGE_H

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>

#include "../../quality/qualitytablesformatter.h"

#include "../plot/plot2d.h"
#include "../plot/plotwidget.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class TwoDimensionalPlotPage : public Gtk::HBox {
	public:
		TwoDimensionalPlotPage();
    virtual ~TwoDimensionalPlotPage();

		void SetStatistics(class StatisticsCollection *statCollection, const std::vector<AntennaInfo> &antennas)
		{
			processStatistics(statCollection, antennas);
			
			_statCollection = statCollection;
			// We need to do this here because it can not be done yet during construction in the
			// constructor (virtual methods not yet available there).
			if(!_customButtonsCreated)
			{
				addCustomPlotButtons(_plotBox);
				_customButtonsCreated = true;
			}
			updatePlot();
		}
		void CloseStatistics()
		{
			_statCollection = 0;
		}
		bool HasStatistics() const
		{
			return _statCollection != 0;
		}
	protected:
		virtual void processStatistics(class StatisticsCollection *, const std::vector<AntennaInfo> &)
		{
		}
		
		virtual const std::map<double, class DefaultStatistics> &GetStatistics() const = 0;
		
		virtual void StartLine(Plot2D &plot, const std::string &name, const std::string &yAxisDesc) = 0;
		
		virtual void processPlot(Plot2D &plot)
		{
		}
		
		virtual void addCustomPlotButtons(Gtk::VBox &container)
		{
		}
		
		class StatisticsCollection *GetStatCollection() const
		{
			return _statCollection;
		}
		void updatePlot();
		
		unsigned selectedKindCount() const;
	private:
		enum PhaseType { AmplitudePhaseType, PhasePhaseType, RealPhaseType, ImaginaryPhaseType} ;
		
		void updatePlotConfig();
		void updateDataWindow();
		
		template<enum PhaseType Phase>
		inline double getValue(const std::complex<long double> val);
		void plotStatistic(QualityTablesFormatter::StatisticKind kind);
		void plotPolarization(QualityTablesFormatter::StatisticKind kind, unsigned polarization);
		void plotPolarization(QualityTablesFormatter::StatisticKind kind, unsigned polarizationA, unsigned polarizationB);
		template<enum PhaseType Phase>
		void plotPhase(QualityTablesFormatter::StatisticKind kind, unsigned polarization);
		template<enum PhaseType Phase>
		void plotPhase(QualityTablesFormatter::StatisticKind kind, unsigned polarizationA, unsigned polarizationB);
		
		void initStatisticKindButtons();
		void initPolarizationButtons();
		void initPhaseButtons();
		void initPlotButtons();
		
		void onLogarithmicClicked()
		{
			_zeroAxisButton.set_sensitive(!_logarithmicButton.get_active());
			updatePlotConfig();
		}
		void onPlotPropertiesClicked();
		void onDataExportClicked();
		
		Gtk::Expander _expander;
		Gtk::VBox _sideBox;
		
		Gtk::Frame _statisticFrame;
		Gtk::VBox _statisticBox;
		Gtk::CheckButton _countButton, _meanButton, _stdDevButton, _varianceButton, _dCountButton, _dMeanButton, _dStdDevButton,  _rfiPercentageButton, _snrButton;
		
		Gtk::Frame _polarizationFrame;
		Gtk::VBox _polarizationBox;
		Gtk::CheckButton _polXXButton, _polXYButton, _polYXButton, _polYYButton, _polXXandYYButton, _polXYandYXButton;
		
		Gtk::Frame _phaseFrame;
		Gtk::VBox _phaseBox;
		Gtk::CheckButton _amplitudeButton, _phaseButton, _realButton, _imaginaryButton;
		
		Gtk::Frame _plotFrame;
		Gtk::VBox _plotBox;
		Gtk::CheckButton _logarithmicButton, _zeroAxisButton;
		Gtk::Button _plotPropertiesButton, _dataExportButton;
		
		class StatisticsCollection *_statCollection;
		Plot2D _plot;
		PlotWidget _plotWidget;
		
		class PlotPropertiesWindow *_plotPropertiesWindow;
		class DataWindow *_dataWindow;
		
		bool _customButtonsCreated;
		
		std::string getYDesc() const;
};

#endif
