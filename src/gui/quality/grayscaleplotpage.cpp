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

#include "../../msio/samplerow.h"

#include "grayscaleplotpage.h"

#include "../imagepropertieswindow.h"

#include "../../quality/statisticscollection.h"
#include "../../quality/statisticsderivator.h"

GrayScalePlotPage::GrayScalePlotPage() :
	_expander("Side bar"),
	_statisticKindFrame("Statistic kind"),
	_countButton("Count"),
	_meanButton("Mean"),
	_stdDevButton("StdDev"),
	_dCountButton("DCount"),
	_dMeanButton("DMean"),
	_dStdDevButton("DStdDev"),
	_rfiPercentageButton("RFIPercentage"),
	_snrButton("SNR"),
	_polarizationFrame("Polarization"),
	_polXXButton("XX"),
	_polXYButton("XY"),
	_polYXButton("YX"),
	_polYYButton("YY"),
	_polXXandYYButton("XX/2 + YY/2"),
	_polXYandYXButton("XY/2 + YX/2"),
	_phaseFrame("Phase"),
	_amplitudePhaseButton("Amplitude"),
	_phasePhaseButton("Phase"),
	_realPhaseButton("Real"),
	_imaginaryPhaseButton("Imaginary"),
	_plotFrame("Plot"),
	_rangeMinMaxButton("Min to max"),
	_rangeWinsorizedButton("Winsorized"),
	_rangeSpecified("Specified"),
	_logarithmicScaleButton("Logarithmic"),
	_normalizeXAxisButton("Normalize X"),
	_normalizeYAxisButton("Normalize Y"),
	_meanNormButton("Mean"),
	_winsorNormButton("Winsor"),
	_medianNormButton("Median"),
	_plotPropertiesButton("Properties..."),
	_selectStatisticKind(QualityTablesFormatter::VarianceStatistic),
	_ready(false),
	_imagePropertiesWindow(0)
{
	initStatisticKinds();
	initPolarizations();
	initPhaseButtons();
	initPlotOptions();
	
	_expander.add(_sideBox);
	pack_start(_expander, Gtk::PACK_SHRINK);
	
	_imageWidget.SetCairoFilter(Cairo::FILTER_NEAREST);
	_imageWidget.SetColorMap(ImageWidget::HotColdMap);
	_imageWidget.SetRange(ImageWidget::MinMax);
	_imageWidget.SetScaleOption(ImageWidget::LogScale);
	_imageWidget.SetZAxisDescription("Statistical value");
	_imageWidget.SetManualZAxisDescription(true);
	_imageWidget.set_size_request(300, 300);
	
	pack_start(_imageWidget);
	
	show_all_children();
	
	_ready = true;
}

GrayScalePlotPage::~GrayScalePlotPage()
{
	if(_imagePropertiesWindow != 0)
		delete _imagePropertiesWindow;
}

void GrayScalePlotPage::initStatisticKinds()
{
	Gtk::RadioButtonGroup statGroup;
	_countButton.set_group(statGroup);
	_countButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectCount));
	_statisticKindBox.pack_start(_countButton, Gtk::PACK_SHRINK);
	
	_meanButton.set_group(statGroup);
	_meanButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectMean));
	_statisticKindBox.pack_start(_meanButton, Gtk::PACK_SHRINK);
	
	_stdDevButton.set_group(statGroup);
	_stdDevButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectStdDev));
	_statisticKindBox.pack_start(_stdDevButton, Gtk::PACK_SHRINK);
	
	_dCountButton.set_group(statGroup);
	_dCountButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectDCount));
	_statisticKindBox.pack_start(_dCountButton, Gtk::PACK_SHRINK);
	
	_dMeanButton.set_group(statGroup);
	_dMeanButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectDMean));
	_statisticKindBox.pack_start(_dMeanButton, Gtk::PACK_SHRINK);
	
	_dStdDevButton.set_group(statGroup);
	_dStdDevButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectDStdDev));
	_statisticKindBox.pack_start(_dStdDevButton, Gtk::PACK_SHRINK);
	
	_rfiPercentageButton.set_group(statGroup);
	_rfiPercentageButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectRFIPercentage));
	_statisticKindBox.pack_start(_rfiPercentageButton, Gtk::PACK_SHRINK);
	
	_snrButton.set_group(statGroup);
	_snrButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectSNR));
	_statisticKindBox.pack_start(_snrButton, Gtk::PACK_SHRINK);
	
	_stdDevButton.set_active();
	
	_statisticKindFrame.add(_statisticKindBox);
	
	_sideBox.pack_start(_statisticKindFrame, Gtk::PACK_SHRINK);
}

void GrayScalePlotPage::initPolarizations()
{
	Gtk::RadioButtonGroup polGroup;
	_polXXButton.set_group(polGroup);
	_polXXButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_polarizationBox.pack_start(_polXXButton, Gtk::PACK_SHRINK);
	
	_polXYButton.set_group(polGroup);
	_polXYButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_polarizationBox.pack_start(_polXYButton, Gtk::PACK_SHRINK);

	_polYXButton.set_group(polGroup);
	_polYXButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_polarizationBox.pack_start(_polYXButton, Gtk::PACK_SHRINK);

	_polYYButton.set_group(polGroup);
	_polYYButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_polarizationBox.pack_start(_polYYButton, Gtk::PACK_SHRINK);

	_polXXandYYButton.set_group(polGroup);
	_polXXandYYButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_polarizationBox.pack_start(_polXXandYYButton, Gtk::PACK_SHRINK);

	_polXYandYXButton.set_group(polGroup);
	_polXYandYXButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_polarizationBox.pack_start(_polXYandYXButton, Gtk::PACK_SHRINK);

	_polXXandYYButton.set_active();
	
	_polarizationFrame.add(_polarizationBox);
	
	_sideBox.pack_start(_polarizationFrame, Gtk::PACK_SHRINK);
}

void GrayScalePlotPage::initPhaseButtons()
{
	Gtk::RadioButtonGroup phaseGroup;
	
	_amplitudePhaseButton.set_group(phaseGroup);
	_amplitudePhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_phaseBox.pack_start(_amplitudePhaseButton, Gtk::PACK_SHRINK);
	
	_phasePhaseButton.set_group(phaseGroup);
	_phasePhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_phaseBox.pack_start(_phasePhaseButton, Gtk::PACK_SHRINK);
	
	_realPhaseButton.set_group(phaseGroup);
	_realPhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_phaseBox.pack_start(_realPhaseButton, Gtk::PACK_SHRINK);
	
	_imaginaryPhaseButton.set_group(phaseGroup);
	_imaginaryPhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::UpdateImage));
	_phaseBox.pack_start(_imaginaryPhaseButton, Gtk::PACK_SHRINK);
	
	_amplitudePhaseButton.set_active();
	
	_phaseFrame.add(_phaseBox);
	
	_sideBox.pack_start(_phaseFrame, Gtk::PACK_SHRINK);
}

void GrayScalePlotPage::initPlotOptions()
{
	Gtk::RadioButtonGroup rangeGroup;
	_rangeMinMaxButton.set_group(rangeGroup);
	_rangeMinMaxButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectMinMaxRange));
	_plotBox.pack_start(_rangeMinMaxButton, Gtk::PACK_SHRINK);

	_rangeWinsorizedButton.set_group(rangeGroup);
	_rangeWinsorizedButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectWinsorizedRange));
	_plotBox.pack_start(_rangeWinsorizedButton, Gtk::PACK_SHRINK);

	_rangeSpecified.set_group(rangeGroup);
	_rangeSpecified.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onSelectSpecifiedRange));
	_plotBox.pack_start(_rangeSpecified, Gtk::PACK_SHRINK);
	
	_logarithmicScaleButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onLogarithmicScaleClicked));
	_plotBox.pack_start(_logarithmicScaleButton, Gtk::PACK_SHRINK);
	_logarithmicScaleButton.set_active(true);
	
	_normalizeXAxisButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onNormalizeAxesButtonClicked));
	_plotBox.pack_start(_normalizeXAxisButton, Gtk::PACK_SHRINK);
	
	_normalizeYAxisButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onNormalizeAxesButtonClicked));
	_plotBox.pack_start(_normalizeYAxisButton, Gtk::PACK_SHRINK);
	
	Gtk::RadioButtonGroup normMethodGroup;
	_meanNormButton.set_group(normMethodGroup);
	_meanNormButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onChangeNormMethod));
	_plotBox.pack_start(_meanNormButton, Gtk::PACK_SHRINK);
	
	_winsorNormButton.set_group(normMethodGroup);
	_winsorNormButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onChangeNormMethod));
	_plotBox.pack_start(_winsorNormButton, Gtk::PACK_SHRINK);
	
	_medianNormButton.set_group(normMethodGroup);
	_medianNormButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onChangeNormMethod));
	_plotBox.pack_start(_medianNormButton, Gtk::PACK_SHRINK);
	
	_plotPropertiesButton.signal_clicked().connect(sigc::mem_fun(*this, &GrayScalePlotPage::onPropertiesClicked));
	_plotBox.pack_start(_plotPropertiesButton, Gtk::PACK_SHRINK);
	
	_plotFrame.add(_plotBox);
	
	_sideBox.pack_start(_plotFrame, Gtk::PACK_SHRINK);
}


void GrayScalePlotPage::UpdateImage()
{
	if(_ready)
	{
		std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr> pair = ConstructImage();
		TimeFrequencyData &data = pair.first;
		if(!data.IsEmpty())
		{
			setToSelectedPolarization(data);
			
			setToSelectedPhase(data);
			
			Image2DCPtr image = data.GetSingleImage();
			if(_normalizeXAxisButton.get_active())
				image = normalizeXAxis(image);
			if(_normalizeYAxisButton.get_active())
				image = normalizeYAxis(image);
			
			_imageWidget.SetZAxisDescription(StatisticsDerivator::GetDescWithUnits(GetSelectedStatisticKind()));
			_imageWidget.SetImage(image);
			_imageWidget.SetOriginalMask(data.GetSingleMask());
			if(pair.second != 0)
				_imageWidget.SetMetaData(pair.second);
			_imageWidget.Update();
		}
	}
}

void GrayScalePlotPage::setToSelectedPolarization(TimeFrequencyData &data)
{
	try {
		TimeFrequencyData *newData = 0;
		if(_polXXButton.get_active())
			newData = data.CreateTFData(XXPolarisation);
		else if(_polXYButton.get_active())
			newData = data.CreateTFData(XYPolarisation);
		else if(_polYXButton.get_active())
			newData = data.CreateTFData(YXPolarisation);
		else if(_polYYButton.get_active())
			newData = data.CreateTFData(YYPolarisation);
		else if(_polXXandYYButton.get_active())
		{
			newData = data.CreateTFData(AutoDipolePolarisation);
			newData->MultiplyImages(0.5);
		}
		else if(_polXYandYXButton.get_active())
			newData = data.CreateTFData(CrossDipolePolarisation);
		if(newData != 0)
		{
			data = *newData;
			delete newData;
		}
	} catch(std::exception &e)
	{
		// probably a conversion error -- polarisation was not available.
		// Best solution is probably to ignore.
	}
}

void GrayScalePlotPage::setToSelectedPhase(TimeFrequencyData &data)
{
	TimeFrequencyData *newData = 0;
	if(_amplitudePhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::AmplitudePart);
	else if(_phasePhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::PhasePart);
	else if(_realPhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::RealPart);
	else if(_imaginaryPhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::ImaginaryPart);
	if(newData != 0)
	{
		data = *newData;
		delete newData;
	}
}

Image2DCPtr GrayScalePlotPage::normalizeXAxis(Image2DCPtr input)
{
	Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
	for(size_t x=0;x<input->Width();++x)
	{
		SampleRowPtr row = SampleRow::CreateFromColumn(input, x);
		num_t norm;
		if(_meanNormButton.get_active())
			norm = 1.0 / row->MeanWithMissings();
		else if(_winsorNormButton.get_active())
			norm = 1.0 / row->WinsorizedMeanWithMissings();
		else // _medianNormButton
			norm = 1.0 / row->MedianWithMissings();
		for(size_t y=0;y<input->Height();++y)
			output->SetValue(x, y, input->Value(x, y) * norm);
	}
	return output;
}

Image2DCPtr GrayScalePlotPage::normalizeYAxis(Image2DCPtr input)
{
	Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
	for(size_t y=0;y<input->Height();++y)
	{
		SampleRowPtr row = SampleRow::CreateFromRow(input, y);
		num_t norm;
		if(_meanNormButton.get_active())
			norm = 1.0 / row->MeanWithMissings();
		else if(_winsorNormButton.get_active())
			norm = 1.0 / row->WinsorizedMeanWithMissings();
		else // _medianNormButton
			norm = 1.0 / row->MedianWithMissings();
		for(size_t x=0;x<input->Width();++x)
			output->SetValue(x, y, input->Value(x, y) * norm);
	}
	return output;
}

void GrayScalePlotPage::onPropertiesClicked()
{
	if(_imagePropertiesWindow == 0)
		_imagePropertiesWindow = new ImagePropertiesWindow(_imageWidget, "Plotting properties");
	_imagePropertiesWindow->show();
	_imagePropertiesWindow->raise();
}
