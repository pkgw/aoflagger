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
#ifndef GUI_QUALITY__GRAYSCALEPLOTPAGE_H
#define GUI_QUALITY__GRAYSCALEPLOTPAGE_H

#include <gtkmm/box.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>
#include <gtkmm/window.h>
#include <gtkmm/radiobutton.h>

#include "../imagewidget.h"

#include "../../quality/qualitytablesformatter.h"

#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class GrayScalePlotPage : public Gtk::HBox {
	public:
		GrayScalePlotPage();
    virtual ~GrayScalePlotPage();
		
	protected:
		virtual std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr> ConstructImage() = 0;
		
		QualityTablesFormatter::StatisticKind GetSelectedStatisticKind() const
		{
			return _selectStatisticKind;
		}
		
		void UpdateImage();
		
		ImageWidget &GrayScaleWidget() { return _imageWidget; }
	private:
		void initStatisticKinds();
		void initPolarizations();
		void initPhaseButtons();
		void initPlotOptions();
		
		void onSelectCount() { _selectStatisticKind = QualityTablesFormatter::CountStatistic; UpdateImage(); }
		void onSelectMean() { _selectStatisticKind = QualityTablesFormatter::MeanStatistic; UpdateImage(); }
		void onSelectStdDev() { _selectStatisticKind = QualityTablesFormatter::StandardDeviationStatistic; UpdateImage(); }
		void onSelectDCount() { _selectStatisticKind = QualityTablesFormatter::DCountStatistic; UpdateImage(); }
		void onSelectDMean() { _selectStatisticKind = QualityTablesFormatter::DMeanStatistic; UpdateImage(); }
		void onSelectDStdDev() { _selectStatisticKind = QualityTablesFormatter::DStandardDeviationStatistic; UpdateImage(); }
		void onSelectRFIPercentage() { _selectStatisticKind = QualityTablesFormatter::RFIPercentageStatistic; UpdateImage(); }
		void onSelectSNR() { _selectStatisticKind = QualityTablesFormatter::SignalToNoiseStatistic; UpdateImage(); }
		void onPropertiesClicked();
		
		void onSelectMinMaxRange() { _imageWidget.SetRange(ImageWidget::MinMax); _imageWidget.Update(); }
		void onSelectWinsorizedRange() { _imageWidget.SetRange(ImageWidget::Winsorized); _imageWidget.Update(); }
		void onSelectSpecifiedRange() { _imageWidget.SetRange(ImageWidget::Specified); _imageWidget.Update(); }
		void onLogarithmicScaleClicked()
		{
			if(_logarithmicScaleButton.get_active())
				_imageWidget.SetScaleOption(ImageWidget::LogScale);
			else
				_imageWidget.SetScaleOption(ImageWidget::NormalScale);
			 _imageWidget.Update();
		}
		void onNormalizeAxesButtonClicked()
		{
			UpdateImage();
		}
		void onChangeNormMethod()
		{
			if(_normalizeYAxisButton.get_active())
				UpdateImage();
		}
		Image2DCPtr normalizeXAxis(Image2DCPtr input);
		Image2DCPtr normalizeYAxis(Image2DCPtr input);
		
		void setToSelectedPolarization(TimeFrequencyData &data);
		void setToSelectedPhase(TimeFrequencyData &data);
		
		Gtk::Expander _expander;
		Gtk::VBox _sideBox;
		
		Gtk::Frame _statisticKindFrame;
		Gtk::VBox _statisticKindBox;
		
		Gtk::RadioButton _countButton, _meanButton, _stdDevButton, _dCountButton, _dMeanButton, _dStdDevButton, _rfiPercentageButton, _snrButton;
		
		Gtk::Frame _polarizationFrame;
		Gtk::VBox _polarizationBox;
		
		Gtk::RadioButton _polXXButton, _polXYButton, _polYXButton, _polYYButton, _polXXandYYButton, _polXYandYXButton;
		
		Gtk::Frame _phaseFrame;
		Gtk::VBox _phaseBox;
		
		Gtk::RadioButton _amplitudePhaseButton, _phasePhaseButton, _realPhaseButton, _imaginaryPhaseButton;
		
		Gtk::Frame _plotFrame;
		Gtk::VBox _plotBox;
		
		Gtk::RadioButton _rangeMinMaxButton, _rangeWinsorizedButton, _rangeSpecified;
		Gtk::CheckButton _logarithmicScaleButton, _normalizeXAxisButton, _normalizeYAxisButton;
		Gtk::RadioButton _meanNormButton, _winsorNormButton, _medianNormButton;
		Gtk::Button _plotPropertiesButton;
		
		QualityTablesFormatter::StatisticKind _selectStatisticKind;
		ImageWidget _imageWidget;
		
		bool _ready;
		
		class ImagePropertiesWindow *_imagePropertiesWindow;
};

#endif
