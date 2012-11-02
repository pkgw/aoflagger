/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include <limits>

#include <boost/bind.hpp>

#include "datawindow.h"
#include "twodimensionalplotpage.h"

#include "../../quality/statisticscollection.h"
#include "../../quality/statisticsderivator.h"

#include "../plot/plotpropertieswindow.h"

TwoDimensionalPlotPage::TwoDimensionalPlotPage() :
	_expander("Side bar"),
	_statisticFrame("Statistics"),
	_countButton("Count"),
	_meanButton("Mean"),
	_stdDevButton("StdDev"),
	_varianceButton("Variance"),
	_dCountButton("DCount"),
	_dMeanButton("DMean"),
	_dStdDevButton("DStdDev"),
	_rfiPercentageButton("RFI"),
	_snrButton("SNR"),
	_polarizationFrame("Polarization"),
	_polXXButton("XX"),
	_polXYButton("XY"),
	_polYXButton("YX"),
	_polYYButton("YY"),
	_polXXandYYButton("XX/2+YY/2"),
	_polXYandYXButton("XY/2+YX/2"),
	_phaseFrame("Phase"),
	_amplitudeButton("Amplitude"),
	_phaseButton("Phase"),
	_realButton("Real"),
	_imaginaryButton("Imaginary"),
	_plotFrame("Plot"),
	_logarithmicButton("Logarithmic"),
	_zeroAxisButton("Zero axis"),
	_plotPropertiesButton("Properties..."),
	_dataExportButton("Data..."),
	_statCollection(0),
	_plotPropertiesWindow(0),
	_customButtonsCreated(false)
{
	initStatisticKindButtons();
	initPolarizationButtons();
	initPhaseButtons();
	initPlotButtons();
	
	_expander.add(_sideBox);
	pack_start(_expander, Gtk::PACK_SHRINK);
	
	_plotWidget.SetPlot(_plot);
	pack_start(_plotWidget, Gtk::PACK_EXPAND_WIDGET);
	
	show_all_children();
	
	_dataWindow = new DataWindow();
}

TwoDimensionalPlotPage::~TwoDimensionalPlotPage()
{
	delete _dataWindow;
	if(_plotPropertiesWindow != 0)
		delete _plotPropertiesWindow;
}

unsigned TwoDimensionalPlotPage::selectedKindCount() const
{
	unsigned count = 0;
	if(_countButton.get_active()) ++count;
	if(_meanButton.get_active()) ++count;
	if(_stdDevButton.get_active()) ++count;
	if(_varianceButton.get_active()) ++count;
	if(_dCountButton.get_active()) ++count;
	if(_dMeanButton.get_active()) ++count;
	if(_dStdDevButton.get_active()) ++count;
	if(_rfiPercentageButton.get_active()) ++count;
	if(_snrButton.get_active()) ++count;
	return count;
}

void TwoDimensionalPlotPage::updatePlot()
{
	if(HasStatistics())
	{
		_plot.Clear();
		
		if(_countButton.get_active())
			plotStatistic(QualityTablesFormatter::CountStatistic);
		if(_meanButton.get_active())
			plotStatistic(QualityTablesFormatter::MeanStatistic);
		if(_stdDevButton.get_active())
			plotStatistic(QualityTablesFormatter::StandardDeviationStatistic);
		if(_varianceButton.get_active())
			plotStatistic(QualityTablesFormatter::VarianceStatistic);
		if(_dCountButton.get_active())
			plotStatistic(QualityTablesFormatter::DCountStatistic);
		if(_dMeanButton.get_active())
			plotStatistic(QualityTablesFormatter::DMeanStatistic);
		if(_dStdDevButton.get_active())
			plotStatistic(QualityTablesFormatter::DStandardDeviationStatistic);
		if(_rfiPercentageButton.get_active())
			plotStatistic(QualityTablesFormatter::RFIPercentageStatistic);
		if(_snrButton.get_active())
			plotStatistic(QualityTablesFormatter::SignalToNoiseStatistic);
		
		processPlot(_plot);
		
		_plotWidget.Update();
		
		if(_dataWindow->get_visible())
		{
			updateDataWindow();
		}
	}
}

void TwoDimensionalPlotPage::updatePlotConfig()
{
	_plot.SetIncludeZeroYAxis(_zeroAxisButton.get_active());
	_plot.SetLogarithmicYAxis(_logarithmicButton.get_active());
	_plotWidget.Update();
}

template<enum TwoDimensionalPlotPage::PhaseType Phase>
double TwoDimensionalPlotPage::getValue(const std::complex<long double> val)
{
	switch(Phase)
	{
		case AmplitudePhaseType: return sqrt(val.real()*val.real() + val.imag()*val.imag());
		case PhasePhaseType: return atan2(val.imag(), val.real());
		case RealPhaseType: return val.real();
		case ImaginaryPhaseType: return val.imag();
	}
}

template<enum TwoDimensionalPlotPage::PhaseType Phase>
void TwoDimensionalPlotPage::plotPhase(QualityTablesFormatter::StatisticKind kind, unsigned polarization)
{
	std::ostringstream s;
	s << "Polarization " << polarization;
	StartLine(_plot, s.str(), getYDesc());
	StatisticsDerivator derivator(*_statCollection);
	const std::map<double, DefaultStatistics> &statistics = GetStatistics();
	for(std::map<double, DefaultStatistics>::const_iterator i=statistics.begin();i!=statistics.end();++i)
	{
		const double x = i->first;
		const std::complex<long double> val = derivator.GetComplexStatistic(kind, i->second, polarization);
		_plot.PushDataPoint(x, getValue<Phase>(val));
	}
}

template<enum TwoDimensionalPlotPage::PhaseType Phase>
void TwoDimensionalPlotPage::plotPhase(QualityTablesFormatter::StatisticKind kind, unsigned polarizationA, unsigned polarizationB)
{
	std::ostringstream s;
	s << "Polarization " << polarizationA << " and " << polarizationB;
	StartLine(_plot, s.str(), getYDesc());
	StatisticsDerivator derivator(*_statCollection);
	const std::map<double, DefaultStatistics> &statistics = GetStatistics();
	for(std::map<double, DefaultStatistics>::const_iterator i=statistics.begin();i!=statistics.end();++i)
	{
		const double x = i->first;
		const std::complex<long double>
			valA = derivator.GetComplexStatistic(kind, i->second, polarizationA),
			valB = derivator.GetComplexStatistic(kind, i->second, polarizationB),
			val = valA*0.5l + valB*0.5l;
		_plot.PushDataPoint(x, getValue<Phase>(val));
	}
}

void TwoDimensionalPlotPage::plotStatistic(QualityTablesFormatter::StatisticKind kind)
{
	if(_polXXButton.get_active())
		plotPolarization(kind, 0);
	if(_polXYButton.get_active())
		plotPolarization(kind, 1);
	if(_polYXButton.get_active())
		plotPolarization(kind, 2);
	if(_polYYButton.get_active())
		plotPolarization(kind, 3);
	if(_polXXandYYButton.get_active())
		plotPolarization(kind, 0, 3);
	if(_polXYandYXButton.get_active())
		plotPolarization(kind, 1, 2);
}

void TwoDimensionalPlotPage::plotPolarization(QualityTablesFormatter::StatisticKind kind, unsigned polarization)
{
	if(_amplitudeButton.get_active())
		plotPhase<AmplitudePhaseType>(kind, polarization);
	if(_phaseButton.get_active())
		plotPhase<PhasePhaseType>(kind, polarization);
	if(_realButton.get_active())
		plotPhase<RealPhaseType>(kind, polarization);
	if(_imaginaryButton.get_active())
		plotPhase<ImaginaryPhaseType>(kind, polarization);
}

void TwoDimensionalPlotPage::plotPolarization(QualityTablesFormatter::StatisticKind kind, unsigned polarizationA, unsigned polarizationB)
{
	if(_amplitudeButton.get_active())
		plotPhase<AmplitudePhaseType>(kind, polarizationA, polarizationB);
	if(_phaseButton.get_active())
		plotPhase<PhasePhaseType>(kind, polarizationA, polarizationB);
	if(_realButton.get_active())
		plotPhase<RealPhaseType>(kind, polarizationA, polarizationB);
	if(_imaginaryButton.get_active())
		plotPhase<ImaginaryPhaseType>(kind, polarizationA, polarizationB);
}

void TwoDimensionalPlotPage::initStatisticKindButtons()
{
	_countButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_countButton, Gtk::PACK_SHRINK);
	
	_meanButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_meanButton, Gtk::PACK_SHRINK);
	
	_stdDevButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_stdDevButton.set_active(true);
	_statisticBox.pack_start(_stdDevButton, Gtk::PACK_SHRINK);
	
	_varianceButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_varianceButton, Gtk::PACK_SHRINK);
	
	_dCountButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_dCountButton, Gtk::PACK_SHRINK);
	
	_dMeanButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_dMeanButton, Gtk::PACK_SHRINK);
	
	_dStdDevButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_dStdDevButton, Gtk::PACK_SHRINK);
	
	_rfiPercentageButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_rfiPercentageButton, Gtk::PACK_SHRINK);
	
	_snrButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_statisticBox.pack_start(_snrButton, Gtk::PACK_SHRINK);
	
	_statisticFrame.add(_statisticBox);
	
	_sideBox.pack_start(_statisticFrame, Gtk::PACK_SHRINK);
}

void TwoDimensionalPlotPage::initPolarizationButtons()
{
	_polXXButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_polarizationBox.pack_start(_polXXButton, Gtk::PACK_SHRINK);
	
	_polXYButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_polarizationBox.pack_start(_polXYButton, Gtk::PACK_SHRINK);
	
	_polYXButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_polarizationBox.pack_start(_polYXButton, Gtk::PACK_SHRINK);
	
	_polYYButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_polarizationBox.pack_start(_polYYButton, Gtk::PACK_SHRINK);
	
	_polXXandYYButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_polXXandYYButton.set_active(true);
	_polarizationBox.pack_start(_polXXandYYButton, Gtk::PACK_SHRINK);
	
	_polXYandYXButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_polarizationBox.pack_start(_polXYandYXButton, Gtk::PACK_SHRINK);
	
	_polarizationFrame.add(_polarizationBox);
	
	_sideBox.pack_start(_polarizationFrame, Gtk::PACK_SHRINK);
}

void TwoDimensionalPlotPage::initPhaseButtons()
{
	_amplitudeButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_amplitudeButton.set_active(true);
	_phaseBox.pack_start(_amplitudeButton, Gtk::PACK_SHRINK);
	
	_phaseButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_phaseBox.pack_start(_phaseButton, Gtk::PACK_SHRINK);
	
	_realButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_phaseBox.pack_start(_realButton, Gtk::PACK_SHRINK);
	
	_imaginaryButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlot));
	_phaseBox.pack_start(_imaginaryButton, Gtk::PACK_SHRINK);
	
	_phaseFrame.add(_phaseBox);
	
	_sideBox.pack_start(_phaseFrame, Gtk::PACK_SHRINK);
}

void TwoDimensionalPlotPage::initPlotButtons()
{
	_logarithmicButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::onLogarithmicClicked));
	_plotBox.pack_start(_logarithmicButton, Gtk::PACK_SHRINK);
	
	_zeroAxisButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::updatePlotConfig));
	_zeroAxisButton.set_active(true);
	_plotBox.pack_start(_zeroAxisButton, Gtk::PACK_SHRINK);
	_plot.SetIncludeZeroYAxis(true);
	
	_plotPropertiesButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::onPlotPropertiesClicked));
	_plotBox.pack_start(_plotPropertiesButton, Gtk::PACK_SHRINK);

	_dataExportButton.signal_clicked().connect(sigc::mem_fun(*this, &TwoDimensionalPlotPage::onDataExportClicked));
	_plotBox.pack_start(_dataExportButton, Gtk::PACK_SHRINK);
	
	_plotFrame.add(_plotBox);
	
	_sideBox.pack_start(_plotFrame, Gtk::PACK_SHRINK);
}

void TwoDimensionalPlotPage::onPlotPropertiesClicked()
{
	if(_plotPropertiesWindow == 0)
	{
		_plotPropertiesWindow = new PlotPropertiesWindow(_plot, "Plot properties");
		_plotPropertiesWindow->OnChangesApplied = boost::bind(&TwoDimensionalPlotPage::updatePlot, this);
	}
	
	_plotPropertiesWindow->show();
	_plotPropertiesWindow->raise();
}

void TwoDimensionalPlotPage::updateDataWindow()
{
	if(_dataWindow->is_visible())
		_dataWindow->SetData(_plot);
}

void TwoDimensionalPlotPage::onDataExportClicked()
{
	_dataWindow->show();
	_dataWindow->raise();
	updateDataWindow();
}

std::string TwoDimensionalPlotPage::getYDesc() const
{
	if(selectedKindCount() != 1)
		return "Value";
	else
	{
		QualityTablesFormatter::StatisticKind kind;
		if(_countButton.get_active()) kind = QualityTablesFormatter::CountStatistic;
		if(_meanButton.get_active()) kind = QualityTablesFormatter::MeanStatistic;
		if(_stdDevButton.get_active()) kind = QualityTablesFormatter::StandardDeviationStatistic;
		if(_varianceButton.get_active()) kind = QualityTablesFormatter::VarianceStatistic;
		if(_dCountButton.get_active()) kind = QualityTablesFormatter::DCountStatistic;
		if(_dMeanButton.get_active()) kind = QualityTablesFormatter::DMeanStatistic;
		if(_dStdDevButton.get_active()) kind = QualityTablesFormatter::DStandardDeviationStatistic;
		if(_rfiPercentageButton.get_active()) kind = QualityTablesFormatter::RFIPercentageStatistic;
		if(_snrButton.get_active()) kind = QualityTablesFormatter::SignalToNoiseStatistic;
		return StatisticsDerivator::GetDescWithUnits(kind);
	}
}

