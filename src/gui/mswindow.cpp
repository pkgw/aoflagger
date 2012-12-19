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
#include "mswindow.h"

#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/inputdialog.h>
#include <gtkmm/toolbar.h>

#include "../msio/baselinematrixloader.h"
#include "../msio/measurementset.h"
#include "../msio/image2d.h"
#include "../msio/timefrequencydata.h"
#include "../msio/timefrequencymetadata.h"
#include "../msio/segmentedimage.h"
#include "../msio/spatialmatrixmetadata.h"

#include "../strategy/actions/strategyaction.h"

#include "../strategy/control/artifactset.h"
#include "../strategy/control/defaultstrategy.h"

#include "../strategy/imagesets/msimageset.h"
#include "../strategy/imagesets/noisestatimageset.h"
#include "../strategy/imagesets/spatialmsimageset.h"
#include "../strategy/imagesets/spatialtimeimageset.h"

#include "../strategy/algorithms/baselineselector.h"
#include "../strategy/algorithms/mitigationtester.h"
#include "../strategy/algorithms/morphology.h"
#include "../strategy/algorithms/fringetestcreater.h"
#include "../strategy/algorithms/fringestoppingfitter.h"
#include "../strategy/algorithms/polarizationstatistics.h"
#include "../strategy/algorithms/rfistatistics.h"
#include "../strategy/algorithms/svdmitigater.h"
#include "../strategy/algorithms/thresholdtools.h"
#include "../strategy/algorithms/timefrequencystatistics.h"
#include "../strategy/algorithms/vertevd.h"

#include "../strategy/plots/antennaflagcountplot.h"
#include "../strategy/plots/frequencyflagcountplot.h"
#include "../strategy/plots/frequencypowerplot.h"
#include "../strategy/plots/iterationsplot.h"
#include "../strategy/plots/rfiplots.h"
#include "../strategy/plots/timeflagcountplot.h"

#include "../util/compress.h"
#include "../util/multiplot.h"

#include "plot/plot2d.h"

#include "antennamapwindow.h"
#include "complexplaneplotwindow.h"
#include "editstrategywindow.h"
#include "gotowindow.h"
#include "highlightwindow.h"
#include "histogramwindow.h"
#include "imageplanewindow.h"
#include "imagepropertieswindow.h"
#include "msoptionwindow.h"
#include "noisestatoptionwindow.h"
#include "numinputdialog.h"
#include "progresswindow.h"
#include "rawoptionwindow.h"
#include "tfstatoptionwindow.h"

#include "../imaging/model.h"
#include "../imaging/observatorium.h"

#include "../quality/histogramcollection.h"

#include <iostream>

MSWindow::MSWindow() : _imagePlaneWindow(0), _histogramWindow(0), _optionWindow(0), _editStrategyWindow(0), _gotoWindow(0), _progressWindow(0), _highlightWindow(0), _plotComplexPlaneWindow(0), _imagePropertiesWindow(0), _antennaMapWindow(0), _statistics(new RFIStatistics()),  _imageSet(0), _imageSetIndex(0), _gaussianTestSets(true), _spatialMetaData(0), _plotWindow(_plotManager)
{
	createToolbar();

	_mainVBox.pack_start(_timeFrequencyWidget);
	_timeFrequencyWidget.OnMouseMovedEvent().connect(sigc::mem_fun(*this, &MSWindow::onTFWidgetMouseMoved));
	_timeFrequencyWidget.OnMouseLeaveEvent().connect(sigc::mem_fun(*this, &MSWindow::setSetNameInStatusBar));
	_timeFrequencyWidget.OnButtonReleasedEvent().connect(sigc::mem_fun(*this, &MSWindow::onTFWidgetButtonReleased));
	_timeFrequencyWidget.SetShowXAxisDescription(false);
	_timeFrequencyWidget.SetShowYAxisDescription(false);
	_timeFrequencyWidget.SetShowZAxisDescription(false);
	_timeFrequencyWidget.show();

	_mainVBox.pack_end(_statusbar, Gtk::PACK_SHRINK);
	_statusbar.push("Ready. For suggestions, contact offringa@astro.rug.nl .");
	_statusbar.show();

	add(_mainVBox);
	_mainVBox.show();

	set_default_size(800,600);

	_strategy = rfiStrategy::DefaultStrategy::CreateStrategy(
		rfiStrategy::DefaultStrategy::GENERIC_TELESCOPE,
		rfiStrategy::DefaultStrategy::FLAG_GUI_FRIENDLY);
	_imagePlaneWindow = new ImagePlaneWindow();
}

MSWindow::~MSWindow()
{
	boost::mutex::scoped_lock lock(_ioMutex);
	while(!_actionGroup->get_actions().empty())
		_actionGroup->remove(*_actionGroup->get_actions().begin());
	
	delete _imagePlaneWindow;
	if(_histogramWindow != 0)
		delete _histogramWindow;
	if(_optionWindow != 0)
		delete _optionWindow;
	if(_editStrategyWindow != 0)
		delete _editStrategyWindow;
	if(_gotoWindow != 0)
		delete _gotoWindow;
	if(_progressWindow != 0)
		delete _progressWindow;
	if(_highlightWindow != 0)
		delete _highlightWindow;
	if(_imagePropertiesWindow != 0)
		delete _imagePropertiesWindow;
	if(_antennaMapWindow != 0)
		delete _antennaMapWindow;
	
	// The rfistrategy needs the lock to clean up
	lock.unlock();
	
	delete _statistics;
	delete _strategy;
	if(HasImageSet())
	{
		delete _imageSetIndex;
		delete _imageSet;
	}
	if(_spatialMetaData != 0)
		delete _spatialMetaData;
}

void MSWindow::onActionDirectoryOpen()
{
  Gtk::FileChooserDialog dialog("Select a measurement set",
          Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  int result = dialog.run();

  if(result == Gtk::RESPONSE_OK)
	{
		OpenPath(dialog.get_filename());
	}
}

void MSWindow::onActionDirectoryOpenForSpatial()
{
  Gtk::FileChooserDialog dialog("Select a measurement set",
          Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  int result = dialog.run();

  if(result == Gtk::RESPONSE_OK)
	{
		boost::mutex::scoped_lock lock(_ioMutex);
		rfiStrategy::SpatialMSImageSet *imageSet = new rfiStrategy::SpatialMSImageSet(dialog.get_filename());
		imageSet->Initialize();
		lock.unlock();
		SetImageSet(imageSet);
	}
}

void MSWindow::onActionDirectoryOpenForST()
{
  Gtk::FileChooserDialog dialog("Select a measurement set",
          Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  int result = dialog.run();

  if(result == Gtk::RESPONSE_OK)
	{
		boost::mutex::scoped_lock lock(_ioMutex);
		rfiStrategy::SpatialTimeImageSet *imageSet = new rfiStrategy::SpatialTimeImageSet(dialog.get_filename());
		imageSet->Initialize();
		lock.unlock();
		SetImageSet(imageSet);
	}
}

void MSWindow::onActionFileOpen()
{
  Gtk::FileChooserDialog dialog("Select a measurement set");
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  int result = dialog.run();

  if(result == Gtk::RESPONSE_OK)
	{
		OpenPath(dialog.get_filename());
	}
}

void MSWindow::OpenPath(const std::string &path)
{
	if(_optionWindow != 0)
		delete _optionWindow;
	if(rfiStrategy::ImageSet::IsRCPRawFile(path))
	{
		_optionWindow = new RawOptionWindow(*this, path);
		_optionWindow->present();
	}
	else if(rfiStrategy::ImageSet::IsMSFile(path))
	{
		_optionWindow = new MSOptionWindow(*this, path);
		_optionWindow->present();
	}
	else if(rfiStrategy::ImageSet::IsTimeFrequencyStatFile(path))
	{
		_optionWindow = new TFStatOptionWindow(*this, path);
		_optionWindow->present();
	}
	else if(rfiStrategy::ImageSet::IsNoiseStatFile(path))
	{
		_optionWindow = new NoiseStatOptionWindow(*this, path);
		_optionWindow->present();
	}
	else
	{
		boost::mutex::scoped_lock lock(_ioMutex);
		rfiStrategy::ImageSet *imageSet = rfiStrategy::ImageSet::Create(path, DirectReadMode);
		imageSet->Initialize();
		lock.unlock();
		SetImageSet(imageSet);
	}
}

void MSWindow::onToggleFlags()
{
	_timeFrequencyWidget.SetShowOriginalMask(_originalFlagsButton->get_active());
	_timeFrequencyWidget.SetShowAlternativeMask(_altFlagsButton->get_active());
	_timeFrequencyWidget.Update();
}

void MSWindow::loadCurrentTFData()
{
	if(_imageSet != 0) {
		try {
			boost::mutex::scoped_lock lock(_ioMutex);
			_imageSet->AddReadRequest(*_imageSetIndex);
			_imageSet->PerformReadRequests();
			rfiStrategy::BaselineData *baseline = _imageSet->GetNextRequested();
			lock.unlock();
			
			_timeFrequencyWidget.SetNewData(baseline->Data(), baseline->MetaData());
			delete baseline;
			if(_spatialMetaData != 0)
			{
				delete _spatialMetaData;
				_spatialMetaData = 0;
			}
			if(dynamic_cast<rfiStrategy::SpatialMSImageSet*>(_imageSet) != 0)
			{
				_spatialMetaData = new SpatialMatrixMetaData(static_cast<rfiStrategy::SpatialMSImageSet*>(_imageSet)->SpatialMetaData(*_imageSetIndex));
			}
			// We store these seperate, as they might access the measurement set. This is
			// not only faster (the names are used in the onMouse.. events) but also less dangerous,
			// since the set can be simultaneously accessed by another thread. (thus the io mutex should
			// be locked before calling below statements).
			_imageSetName = _imageSet->Name();
			_imageSetIndexDescription = _imageSetIndex->Description();
			
			_timeFrequencyWidget.SetTitleText(_imageSetIndexDescription);
			_timeFrequencyWidget.Update();
			
			setSetNameInStatusBar();
		} catch(std::exception &e)
		{
			AOLogger::Error << e.what() << '\n';
			showError(e.what());
		}
	}
}

void MSWindow::setSetNameInStatusBar()
{
  if(HasImageSet()) {
		_statusbar.pop();
		_statusbar.push(_imageSetName + ": " + _imageSetIndexDescription);
  }
}
		
void MSWindow::onLoadPrevious()
{
	if(_imageSet != 0) {
		boost::mutex::scoped_lock lock(_ioMutex);
		_imageSetIndex->Previous();
		lock.unlock();
		loadCurrentTFData();
	}
}

void MSWindow::onLoadNext()
{
	if(_imageSet != 0) {
		boost::mutex::scoped_lock lock(_ioMutex);
		_imageSetIndex->Next();
		lock.unlock();
		loadCurrentTFData();
	}
}

void MSWindow::onLoadLargeStepPrevious()
{
	if(_imageSet != 0) {
		boost::mutex::scoped_lock lock(_ioMutex);
		_imageSetIndex->LargeStepPrevious();
		lock.unlock();
		loadCurrentTFData();
	}
}

void MSWindow::onLoadLargeStepNext()
{
	if(_imageSet != 0) {
		boost::mutex::scoped_lock lock(_ioMutex);
		_imageSetIndex->LargeStepNext();
		lock.unlock();
		loadCurrentTFData();
	}
}

void MSWindow::onEditStrategyPressed()
{
	if(_editStrategyWindow != 0)
		delete _editStrategyWindow;
	_editStrategyWindow = new EditStrategyWindow(*this);
	_editStrategyWindow->show();
}

void MSWindow::onExecuteStrategyPressed()
{
	if(_progressWindow != 0)
		delete _progressWindow;

	ProgressWindow *window = new ProgressWindow(*this);
	_progressWindow = window;
	_progressWindow->show();

	rfiStrategy::ArtifactSet artifacts(&_ioMutex);

	artifacts.SetAntennaFlagCountPlot(new AntennaFlagCountPlot());
	artifacts.SetFrequencyFlagCountPlot(new FrequencyFlagCountPlot());
	artifacts.SetFrequencyPowerPlot(new FrequencyPowerPlot());
	artifacts.SetTimeFlagCountPlot(new TimeFlagCountPlot());
	artifacts.SetIterationsPlot(new IterationsPlot());
	
	artifacts.SetPolarizationStatistics(new PolarizationStatistics());
	artifacts.SetBaselineSelectionInfo(new rfiStrategy::BaselineSelector());
	artifacts.SetImager(_imagePlaneWindow->GetImager());

	if(HasImage())
	{
		artifacts.SetOriginalData(GetOriginalData());
		artifacts.SetContaminatedData(GetContaminatedData());
		TimeFrequencyData *zero = new TimeFrequencyData(GetOriginalData());
		zero->SetImagesToZero();
		artifacts.SetRevisedData(*zero);
		delete zero;
	}
	if(_timeFrequencyWidget.GetMetaData() != 0)
			artifacts.SetMetaData(_timeFrequencyWidget.GetMetaData());
	if(HasImageSet())
	{
		artifacts.SetImageSet(_imageSet);
		artifacts.SetImageSetIndex(_imageSetIndex);
	}
	_strategy->InitializeAll();
	try {
		_strategy->StartPerformThread(artifacts, *window);
	}  catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onExecuteStrategyFinished()
{
	rfiStrategy::ArtifactSet *artifacts = _strategy->JoinThread();
	if(artifacts != 0)
	{
		bool update = false;
		if(!artifacts->RevisedData().IsEmpty())
		{
			std::cout << "Updating revised data..." << std::endl;
			_timeFrequencyWidget.SetRevisedData(artifacts->RevisedData());
			update = true;
		}

		if(!artifacts->ContaminatedData().IsEmpty())
		{
			std::cout << "Updating contaminated data..." << std::endl;
			_timeFrequencyWidget.SetContaminatedData(artifacts->ContaminatedData());
			update = true;
		}
		
		if(update)
			_timeFrequencyWidget.Update();
		
		_imagePlaneWindow->Update();
		
		if(artifacts->AntennaFlagCountPlot()->HasData())
			artifacts->AntennaFlagCountPlot()->MakePlot();
		if(artifacts->FrequencyFlagCountPlot()->HasData())
			artifacts->FrequencyFlagCountPlot()->MakePlot();
		if(artifacts->FrequencyPowerPlot()->HasData())
			artifacts->FrequencyPowerPlot()->MakePlot();
		if(artifacts->TimeFlagCountPlot()->HasData())
			artifacts->TimeFlagCountPlot()->MakePlot();
		if(artifacts->PolarizationStatistics()->HasData())
			artifacts->PolarizationStatistics()->Report();
		if(artifacts->IterationsPlot()->HasData())
			artifacts->IterationsPlot()->MakePlot();

		delete artifacts->AntennaFlagCountPlot();
		delete artifacts->FrequencyFlagCountPlot();
		delete artifacts->FrequencyPowerPlot();
		delete artifacts->TimeFlagCountPlot();
		delete artifacts->PolarizationStatistics();
		delete artifacts->BaselineSelectionInfo();
		delete artifacts->IterationsPlot();
		delete artifacts;
	}
}

void MSWindow::onToggleImage()
{
	ImageComparisonWidget::TFImage image = ImageComparisonWidget::TFOriginalImage;
	if(_backgroundImageButton->get_active())
		image = ImageComparisonWidget::TFRevisedImage;
	else if(_diffImageButton->get_active())
		image = ImageComparisonWidget::TFContaminatedImage;
	_timeFrequencyWidget.SetVisualizedImage(image);
	_timeFrequencyWidget.Update();
}

void MSWindow::SetImageSet(rfiStrategy::ImageSet *newImageSet)
{
	if(_imageSet != 0) {
		delete _imageSet;
		delete _imageSetIndex;
	}
	_imageSet = newImageSet;
	_imageSetIndex = _imageSet->StartIndex();
	
	if(dynamic_cast<rfiStrategy::MSImageSet*>(newImageSet) != 0)
	{
		onGoToPressed();
	} else {
		loadCurrentTFData();
	}
}

void MSWindow::SetImageSetIndex(rfiStrategy::ImageSetIndex *newImageSetIndex)
{
	if(HasImageSet())
	{
		delete _imageSetIndex;
		_imageSetIndex = newImageSetIndex;
		_imageSetIndexDescription = _imageSetIndex->Description();
		loadCurrentTFData();
	} else {
		delete newImageSetIndex;
	}
}

void MSWindow::openTestSet(unsigned index)
{
	unsigned width = 1024, height = 1024;
	if(HasImage())
	{
		width = _timeFrequencyWidget.Image()->Width();
		height = _timeFrequencyWidget.Image()->Height();
	}
	Mask2DPtr rfi = Mask2D::CreateSetMaskPtr<false>(width, height);
	Image2DPtr testSetReal(MitigationTester::CreateTestSet(index, rfi, width, height, _gaussianTestSets));
	Image2DPtr testSetImaginary(MitigationTester::CreateTestSet(2, rfi, width, height, _gaussianTestSets));
	TimeFrequencyData data(SinglePolarisation, testSetReal, testSetImaginary);
	data.SetGlobalMask(rfi);
	
	_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
	_timeFrequencyWidget.Update();
}

void MSWindow::createToolbar()
{
	_actionGroup = Gtk::ActionGroup::create();
	_actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
	_actionGroup->add( Gtk::Action::create("MenuGo", "_Go") );
	_actionGroup->add( Gtk::Action::create("MenuView", "_View") );
	_actionGroup->add( Gtk::Action::create("MenuPlot", "_Plot") );
	_actionGroup->add( Gtk::Action::create("MenuSimulate", "_Simulate") );
	_actionGroup->add( Gtk::Action::create("MenuPlotFlagComparison", "_Compare flags") );
	_actionGroup->add( Gtk::Action::create("MenuActions", "_Actions") );
	_actionGroup->add( Gtk::Action::create("MenuData", "_Data") );
	_actionGroup->add( Gtk::Action::create("OpenFile", Gtk::Stock::OPEN, "Open _file"),
  sigc::mem_fun(*this, &MSWindow::onActionFileOpen) );
	_actionGroup->add( Gtk::Action::create("OpenDirectory", Gtk::Stock::OPEN, "Open _directory"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpen) );
	_actionGroup->add( Gtk::Action::create("OpenDirectorySpatial", Gtk::Stock::OPEN, "Open _directory as spatial"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpenForSpatial) );
	_actionGroup->add( Gtk::Action::create("OpenDirectoryST", Gtk::Stock::OPEN, "Open _directory as spatial/time"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpenForST) );
	_actionGroup->add( Gtk::Action::create("OpenTestSet", "Open _testset") );

	Gtk::RadioButtonGroup testSetGroup;
	_gaussianTestSetsButton = Gtk::RadioAction::create(testSetGroup, "GaussianTestSets", "Gaussian");
	_gaussianTestSetsButton->set_active(true);
	_rayleighTestSetsButton = Gtk::RadioAction::create(testSetGroup, "RayleighTestSets", "Rayleigh");
	_zeroTestSetsButton = Gtk::RadioAction::create(testSetGroup, "ZeroTestSets", "Zero");
	_actionGroup->add(_gaussianTestSetsButton, sigc::mem_fun(*this, &MSWindow::onGaussianTestSets) );
	_actionGroup->add(_rayleighTestSetsButton, sigc::mem_fun(*this, &MSWindow::onRayleighTestSets) );
	_actionGroup->add(_zeroTestSetsButton, sigc::mem_fun(*this, &MSWindow::onZeroTestSets) );
	
	_actionGroup->add( Gtk::Action::create("OpenTestSetA", "Test set A"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetA) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetB", "Test set B"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetB) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetC", "Test set C"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetC) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetD", "Test set D"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetD) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetE", "Test set E"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetE) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetF", "Test set F"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetF) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetG", "Test set G"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetG) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetH", "Test set H"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetH) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetNoise", "Noise"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetNoise));
	_actionGroup->add( Gtk::Action::create("OpenTestSetModel3", "3-stars model"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSet3Model));
	_actionGroup->add( Gtk::Action::create("OpenTestSetModel5", "5-stars model"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSet5Model));
	_actionGroup->add( Gtk::Action::create("OpenTestSetNoiseModel3", "3-stars model with noise"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetNoise3Model));
	_actionGroup->add( Gtk::Action::create("OpenTestSetNoiseModel5", "5-stars model with noise"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetNoise5Model));
	_actionGroup->add( Gtk::Action::create("OpenTestSetBStrong", "Test set B (strong RFI)"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetBStrong));
	_actionGroup->add( Gtk::Action::create("OpenTestSetBWeak", "Test set B (weak RFI)"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetBWeak));
	_actionGroup->add( Gtk::Action::create("OpenTestSetBAligned", "Test set B (aligned)"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetBAligned));
	_actionGroup->add( Gtk::Action::create("OpenTestSetGaussianBroadband", "Gaussian broadband"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetGaussianBroadband));
	_actionGroup->add( Gtk::Action::create("OpenTestSetSinusoidalBroadband", "Sinusoidal broadband"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetSinusoidalBroadband));
	_actionGroup->add( Gtk::Action::create("OpenTestSetSlewedGaussianBroadband", "Slewed Gaussian"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetSlewedGaussianBroadband));
	_actionGroup->add( Gtk::Action::create("OpenTestSetBurstBroadband", "Burst"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetBurstBroadband));
	_actionGroup->add( Gtk::Action::create("OpenTestSetRFIDistributionLow", "Slope -2 dist low"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetRFIDistributionLow));
	_actionGroup->add( Gtk::Action::create("OpenTestSetRFIDistributionMid", "Slope -2 dist mid"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetRFIDistributionMid));
	_actionGroup->add( Gtk::Action::create("OpenTestSetRFIDistributionHigh", "Slope -2 dist high"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetRFIDistributionHigh));
	_actionGroup->add( Gtk::Action::create("AddTestModification", "Test modify") );
	_actionGroup->add( Gtk::Action::create("AddStaticFringe", "Static fringe"),
	sigc::mem_fun(*this, &MSWindow::onAddStaticFringe) );
	_actionGroup->add( Gtk::Action::create("Add1SigmaStaticFringe", "Static 1 sigma fringe"),
	sigc::mem_fun(*this, &MSWindow::onAdd1SigmaFringe) );
	_actionGroup->add( Gtk::Action::create("SetToOne", "Set to 1"),
	sigc::mem_fun(*this, &MSWindow::onSetToOne) );
	_actionGroup->add( Gtk::Action::create("SetToI", "Set to i"),
	sigc::mem_fun(*this, &MSWindow::onSetToI) );
	_actionGroup->add( Gtk::Action::create("SetToOnePlusI", "Set to 1+i"),
	sigc::mem_fun(*this, &MSWindow::onSetToOnePlusI) );
	_actionGroup->add( Gtk::Action::create("MultiplyData", "Multiply data..."),
	sigc::mem_fun(*this, &MSWindow::onMultiplyData) );
	_actionGroup->add( Gtk::Action::create("Compress", "Compress"),
	sigc::mem_fun(*this, &MSWindow::onCompress) );
	_actionGroup->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT),
	sigc::mem_fun(*this, &MSWindow::onQuit) );

	_actionGroup->add( Gtk::Action::create("ImageProperties", "Plot properties..."),
  	sigc::mem_fun(*this, &MSWindow::onImagePropertiesPressed) );
	_timeGraphButton = Gtk::ToggleAction::create("TimeGraph", "Time graph");
	_timeGraphButton->set_active(false); 
	_actionGroup->add(_timeGraphButton, sigc::mem_fun(*this, &MSWindow::onTimeGraphButtonPressed) );
	_actionGroup->add( Gtk::Action::create("ShowAntennaMapWindow", "Show antenna map"), sigc::mem_fun(*this, &MSWindow::onShowAntennaMapWindow) );
	
	_actionGroup->add( Gtk::Action::create("PlotDist", "Plot _distribution"),
  sigc::mem_fun(*this, &MSWindow::onPlotDistPressed) );
	_actionGroup->add( Gtk::Action::create("PlotLogLogDist", "Plot _log-log dist"),
  sigc::mem_fun(*this, &MSWindow::onPlotLogLogDistPressed) );
	_actionGroup->add( Gtk::Action::create("PlotComplexPlane", "Plot _complex plane"),
  sigc::mem_fun(*this, &MSWindow::onPlotComplexPlanePressed) );
	_actionGroup->add( Gtk::Action::create("PlotMeanSpectrum", "Plot _mean spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotMeanSpectrumPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSumSpectrum", "Plot s_um spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotSumSpectrumPressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerSpectrum", "Plot _power spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSpectrumPressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerSpectrumComparison", "Power _spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSpectrumComparisonPressed) );
	_actionGroup->add( Gtk::Action::create("PlotRMSSpectrum", "Plot _rms spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerRMSPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSNRSpectrum", "Plot spectrum snr"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSNRPressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerTime", "Plot power vs _time"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerTimePressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerTimeComparison", "Po_wer vs time"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerTimeComparisonPressed) );
	_actionGroup->add( Gtk::Action::create("PlotTimeScatter", "Plot time s_catter"),
  sigc::mem_fun(*this, &MSWindow::onPlotTimeScatterPressed) );
	_actionGroup->add( Gtk::Action::create("PlotTimeScatterComparison", "Time _scatter"),
  sigc::mem_fun(*this, &MSWindow::onPlotTimeScatterComparisonPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSingularValues", "Plot _singular values"),
  sigc::mem_fun(*this, &MSWindow::onPlotSingularValuesPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSNRToFitVariance", "Plot SNR to fit variance"),
  sigc::mem_fun(*this, &MSWindow::onPlotSNRToFitVariance) );
	_actionGroup->add( Gtk::Action::create("PlotQuality25", "Plot quality (25)"),
  sigc::mem_fun(*this, &MSWindow::onPlotQuality25Pressed) );
	_actionGroup->add( Gtk::Action::create("PlotQualityAll", "Plot quality (all)"),
  sigc::mem_fun(*this, &MSWindow::onPlotQualityAllPressed) );
	_actionGroup->add( Gtk::Action::create("ShowImagePlane", "_Show image plane"),
  sigc::mem_fun(*this, &MSWindow::onShowImagePlane) );
	_actionGroup->add( Gtk::Action::create("SetAndShowImagePlane", "S_et and show image plane"),
  sigc::mem_fun(*this, &MSWindow::onSetAndShowImagePlane) );
	_actionGroup->add( Gtk::Action::create("AddToImagePlane", "Add to _image plane"),
  sigc::mem_fun(*this, &MSWindow::onAddToImagePlane) );
	
	Gtk::RadioButtonGroup setGroup;
	_ncpSetButton = Gtk::RadioAction::create(setGroup, "NCPSet", "Use NCP set");
	_b1834SetButton = Gtk::RadioAction::create(setGroup, "B1834Set", "Use B1834 set");
	_emptySetButton = Gtk::RadioAction::create(setGroup, "EmptySet", "Use empty set");
	_ncpSetButton->set_active(true); 
	_actionGroup->add(_ncpSetButton);
	_actionGroup->add(_b1834SetButton);
	_actionGroup->add(_emptySetButton);
	
	Gtk::RadioButtonGroup chGroup;
	_sim16ChannelsButton = Gtk::RadioAction::create(chGroup, "Sim16Channels", "16 channels");
	_sim64ChannelsButton = Gtk::RadioAction::create(chGroup, "Sim64Channels", "64 channels");
	_sim256ChannelsButton = Gtk::RadioAction::create(chGroup, "Sim256Channels", "256 channels");
	_sim64ChannelsButton->set_active(true); 
	_actionGroup->add(_sim16ChannelsButton);
	_actionGroup->add(_sim64ChannelsButton);
	_actionGroup->add(_sim256ChannelsButton);
	
	_simFixBandwidthButton = Gtk::ToggleAction::create("SimFixBandwidth", "Fix bandwidth");
	_simFixBandwidthButton->set_active(false); 
	_actionGroup->add(_simFixBandwidthButton);
	
	_actionGroup->add( Gtk::Action::create("SimulateCorrelation", "Simulate correlation"),
  sigc::mem_fun(*this, &MSWindow::onSimulateCorrelation) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetA", "Simulate source set A"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetA) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetB", "Simulate source set B"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetB) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetC", "Simulate source set C"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetC) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetD", "Simulate source set D"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetD) );
	_actionGroup->add( Gtk::Action::create("SimulateOffAxisSource", "Simulate off-axis source"),
  sigc::mem_fun(*this, &MSWindow::onSimulateOffAxisSource) );
	_actionGroup->add( Gtk::Action::create("SimulateOnAxisSource", "Simulate on-axis source"),
  sigc::mem_fun(*this, &MSWindow::onSimulateOnAxisSource) );

	_actionGroup->add( Gtk::Action::create("EditStrategy", "_Edit strategy"),
  sigc::mem_fun(*this, &MSWindow::onEditStrategyPressed) );
	_actionGroup->add( Gtk::Action::create("ExecuteStrategy", "E_xecute strategy"),
  sigc::mem_fun(*this, &MSWindow::onExecuteStrategyPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStats", "Show _stats"),
  sigc::mem_fun(*this, &MSWindow::onShowStats) );
	_actionGroup->add( Gtk::Action::create("Previous", Gtk::Stock::GO_BACK),
  sigc::mem_fun(*this, &MSWindow::onLoadPrevious) );
	_actionGroup->add( Gtk::Action::create("Next", Gtk::Stock::GO_FORWARD),
  sigc::mem_fun(*this, &MSWindow::onLoadNext) );
	_actionGroup->add( Gtk::Action::create("LargeStepPrevious", Gtk::Stock::GOTO_FIRST),
  sigc::mem_fun(*this, &MSWindow::onLoadLargeStepPrevious) );
	_actionGroup->add( Gtk::Action::create("LargeStepNext", Gtk::Stock::GOTO_LAST),
  sigc::mem_fun(*this, &MSWindow::onLoadLargeStepNext) );
	_actionGroup->add( Gtk::Action::create("GoTo", "_Go to..."),
  sigc::mem_fun(*this, &MSWindow::onGoToPressed) );
  _originalFlagsButton = Gtk::ToggleAction::create("OriginalFlags", "Original\nflags");
	_originalFlagsButton->set_active(true); 
	_actionGroup->add(_originalFlagsButton, sigc::mem_fun(*this, &MSWindow::onToggleFlags) );
  _altFlagsButton = Gtk::ToggleAction::create("AlternativeFlags", "Alternative\nflags");
	_altFlagsButton->set_active(true); 
	_actionGroup->add(_altFlagsButton, sigc::mem_fun(*this, &MSWindow::onToggleFlags) );
	_actionGroup->add( Gtk::Action::create("ClearAltFlags", "Clear"),
  sigc::mem_fun(*this, &MSWindow::onClearAltFlagsPressed) );

	Gtk::RadioButtonGroup imageGroup;
	_originalImageButton = Gtk::RadioAction::create(imageGroup, "ImageOriginal", "Original");
	_originalImageButton->set_active(true);
	_backgroundImageButton = Gtk::RadioAction::create(imageGroup, "ImageBackground", "Background");
	_diffImageButton = Gtk::RadioAction::create(imageGroup, "ImageDiff", "Difference");
	_actionGroup->add(_originalImageButton, sigc::mem_fun(*this, &MSWindow::onToggleImage) );
	_actionGroup->add(_backgroundImageButton, sigc::mem_fun(*this, &MSWindow::onToggleImage) );
	_actionGroup->add(_diffImageButton, sigc::mem_fun(*this, &MSWindow::onToggleImage) );

	_actionGroup->add( Gtk::Action::create("DiffToOriginal", "Diff->Original"),
  sigc::mem_fun(*this, &MSWindow::onDifferenceToOriginalPressed) );
	_actionGroup->add( Gtk::Action::create("BackToOriginal", "Background->Original"),
  sigc::mem_fun(*this, &MSWindow::onBackgroundToOriginalPressed) );

	_actionGroup->add( Gtk::Action::create("ShowReal", "Keep _real part"),
  sigc::mem_fun(*this, &MSWindow::onShowRealPressed) );
	_actionGroup->add( Gtk::Action::create("ShowImaginary", "Keep _imaginary part"),
  sigc::mem_fun(*this, &MSWindow::onShowImaginaryPressed) );
	_actionGroup->add( Gtk::Action::create("ShowPhase", "Keep _phase part"),
  sigc::mem_fun(*this, &MSWindow::onShowPhasePressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesI", "Keep _stokesI part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesIPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesQ", "Keep stokes_Q part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesQPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesU", "Keep stokes_U part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesUPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesV", "Keep stokes_V part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesVPressed) );
	_actionGroup->add( Gtk::Action::create("ShowAutoPol", "Keep xx+yy part"),
  sigc::mem_fun(*this, &MSWindow::onShowAutoDipolePressed) );
	_actionGroup->add( Gtk::Action::create("ShowCrossPol", "Keep xy+yx part"),
  sigc::mem_fun(*this, &MSWindow::onShowCrossDipolePressed) );
	_actionGroup->add( Gtk::Action::create("ShowXX", "Keep _xx part"),
  sigc::mem_fun(*this, &MSWindow::onShowXXPressed) );
	_actionGroup->add( Gtk::Action::create("ShowXY", "Keep xy part"),
  sigc::mem_fun(*this, &MSWindow::onShowXYPressed) );
	_actionGroup->add( Gtk::Action::create("ShowYX", "Keep yx part"),
  sigc::mem_fun(*this, &MSWindow::onShowYXPressed) );
	_actionGroup->add( Gtk::Action::create("ShowYY", "Keep _yy part"),
  sigc::mem_fun(*this, &MSWindow::onShowYYPressed) );
	_actionGroup->add( Gtk::Action::create("UnrollPhase", "_Unroll phase"),
	sigc::mem_fun(*this, &MSWindow::onUnrollPhaseButtonPressed) );

	_actionGroup->add( Gtk::Action::create("Segment", "Segment"),
  sigc::mem_fun(*this, &MSWindow::onSegment) );
	_actionGroup->add( Gtk::Action::create("Cluster", "Cluster"),
  sigc::mem_fun(*this, &MSWindow::onCluster) );
	_actionGroup->add( Gtk::Action::create("Classify", "Classify"),
  sigc::mem_fun(*this, &MSWindow::onClassify) );
	_actionGroup->add( Gtk::Action::create("RemoveSmallSegments", "Remove small segments"),
  sigc::mem_fun(*this, &MSWindow::onRemoveSmallSegments) );
	_actionGroup->add( Gtk::Action::create("StoreData", "Store"),
  sigc::mem_fun(*this, &MSWindow::onStoreData) );
	_actionGroup->add( Gtk::Action::create("RecallData", "Recall"),
  sigc::mem_fun(*this, &MSWindow::onRecallData) );
	_actionGroup->add( Gtk::Action::create("SubtractDataFromMem", "Subtract from mem"),
  sigc::mem_fun(*this, &MSWindow::onSubtractDataFromMem) );

	_actionGroup->add( Gtk::Action::create("Highlight", "Highlight"),
  sigc::mem_fun(*this, &MSWindow::onHightlightPressed) );
	_actionGroup->add( Gtk::Action::create("TimeMergeUnsetValues", "Merge unset values in time"),
  sigc::mem_fun(*this, &MSWindow::onTimeMergeUnsetValues) );
	_actionGroup->add( Gtk::Action::create("VertEVD", "Vert EVD"),
  sigc::mem_fun(*this, &MSWindow::onVertEVD) );
	_actionGroup->add( Gtk::Action::create("ApplyTimeProfile", "Apply time profile"),
  sigc::mem_fun(*this, &MSWindow::onApplyTimeProfile) );
	_actionGroup->add( Gtk::Action::create("ApplyVertProfile", "Apply vert profile"),
  sigc::mem_fun(*this, &MSWindow::onApplyVertProfile) );
	_actionGroup->add( Gtk::Action::create("RestoreTimeProfile", "Restore time profile"),
  sigc::mem_fun(*this, &MSWindow::onRestoreTimeProfile) );
	_actionGroup->add( Gtk::Action::create("RestoreVertProfile", "Restore vert profile"),
  sigc::mem_fun(*this, &MSWindow::onRestoreVertProfile) );
	_actionGroup->add( Gtk::Action::create("ReapplyTimeProfile", "Reapply time profile"),
  sigc::mem_fun(*this, &MSWindow::onReapplyTimeProfile) );
	_actionGroup->add( Gtk::Action::create("ReapplyVertProfile", "Reapply vert profile"),
  sigc::mem_fun(*this, &MSWindow::onReapplyVertProfile) );

	Glib::RefPtr<Gtk::UIManager> uiManager =
		Gtk::UIManager::create();
	uiManager->insert_action_group(_actionGroup);
	add_accel_group(uiManager->get_accel_group());

	Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='MenuFile'>"
    "      <menuitem action='OpenFile'/>"
    "      <menuitem action='OpenDirectory'/>"
    "      <menuitem action='OpenDirectorySpatial'/>"
    "      <menuitem action='OpenDirectoryST'/>"
    "      <menu action='OpenTestSet'>"
		"        <menuitem action='GaussianTestSets'/>"
		"        <menuitem action='RayleighTestSets'/>"
		"        <menuitem action='ZeroTestSets'/>"
    "        <separator/>"
		"        <menuitem action='OpenTestSetA'/>"
		"        <menuitem action='OpenTestSetB'/>"
		"        <menuitem action='OpenTestSetC'/>"
		"        <menuitem action='OpenTestSetD'/>"
		"        <menuitem action='OpenTestSetE'/>"
		"        <menuitem action='OpenTestSetF'/>"
		"        <menuitem action='OpenTestSetG'/>"
		"        <menuitem action='OpenTestSetH'/>"
		"        <menuitem action='OpenTestSetNoise'/>"
		"        <menuitem action='OpenTestSetModel3'/>"
		"        <menuitem action='OpenTestSetModel5'/>"
		"        <menuitem action='OpenTestSetNoiseModel3'/>"
		"        <menuitem action='OpenTestSetNoiseModel5'/>"
		"        <menuitem action='OpenTestSetBStrong'/>"
		"        <menuitem action='OpenTestSetBWeak'/>"
		"        <menuitem action='OpenTestSetBAligned'/>"
		"        <menuitem action='OpenTestSetGaussianBroadband'/>"
		"        <menuitem action='OpenTestSetSinusoidalBroadband'/>"
		"        <menuitem action='OpenTestSetSlewedGaussianBroadband'/>"
		"        <menuitem action='OpenTestSetBurstBroadband'/>"
		"        <menuitem action='OpenTestSetRFIDistributionLow'/>"
		"        <menuitem action='OpenTestSetRFIDistributionMid'/>"
		"        <menuitem action='OpenTestSetRFIDistributionHigh'/>"
		"      </menu>"
		"      <menu action='AddTestModification'>"
		"        <menuitem action='AddStaticFringe'/>"
		"        <menuitem action='Add1SigmaStaticFringe'/>"
		"        <menuitem action='SetToOne'/>"
		"        <menuitem action='SetToI'/>"
		"        <menuitem action='SetToOnePlusI'/>"
		"        <menuitem action='MultiplyData'/>"
		"      </menu>"
    "      <menuitem action='Compress'/>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
	  "    <menu action='MenuView'>"
    "      <menuitem action='ImageProperties'/>"
    "      <menuitem action='ShowAntennaMapWindow'/>"
    "      <menuitem action='TimeGraph'/>"
    "      <separator/>"
    "      <menuitem action='ShowImagePlane'/>"
    "      <menuitem action='SetAndShowImagePlane'/>"
    "      <menuitem action='AddToImagePlane'/>"
	  "    </menu>"
	  "    <menu action='MenuPlot'>"
    "      <menu action='MenuPlotFlagComparison'>"
    "        <menuitem action='PlotPowerSpectrumComparison'/>"
    "        <menuitem action='PlotPowerTimeComparison'/>"
    "        <menuitem action='PlotTimeScatterComparison'/>"
		"      </menu>"
    "      <separator/>"
    "      <menuitem action='PlotDist'/>"
    "      <menuitem action='PlotLogLogDist'/>"
    "      <menuitem action='PlotComplexPlane'/>"
    "      <menuitem action='PlotMeanSpectrum'/>"
    "      <menuitem action='PlotSumSpectrum'/>"
    "      <menuitem action='PlotPowerSpectrum'/>"
    "      <menuitem action='PlotRMSSpectrum'/>"
    "      <menuitem action='PlotSNRSpectrum'/>"
    "      <menuitem action='PlotPowerTime'/>"
    "      <menuitem action='PlotTimeScatter'/>"
    "      <menuitem action='PlotSingularValues'/>"
    "      <menuitem action='PlotSNRToFitVariance'/>"
    "      <menuitem action='PlotQuality25'/>"
    "      <menuitem action='PlotQualityAll'/>"
	  "    </menu>"
    "    <menu action='MenuGo'>"
    "      <menuitem action='LargeStepPrevious'/>"
    "      <menuitem action='Previous'/>"
    "      <menuitem action='Next'/>"
    "      <menuitem action='LargeStepNext'/>"
    "      <separator/>"
    "      <menuitem action='GoTo'/>"
    "    </menu>"
	  "    <menu action='MenuSimulate'>"
    "      <menuitem action='NCPSet'/>"
    "      <menuitem action='B1834Set'/>"
    "      <menuitem action='EmptySet'/>"
    "      <separator/>"
    "      <menuitem action='Sim16Channels'/>"
    "      <menuitem action='Sim64Channels'/>"
    "      <menuitem action='Sim256Channels'/>"
    "      <menuitem action='SimFixBandwidth'/>"
    "      <separator/>"
    "      <menuitem action='SimulateCorrelation'/>"
    "      <menuitem action='SimulateSourceSetA'/>"
    "      <menuitem action='SimulateSourceSetB'/>"
    "      <menuitem action='SimulateSourceSetC'/>"
    "      <menuitem action='SimulateSourceSetD'/>"
    "      <menuitem action='SimulateOffAxisSource'/>"
    "      <menuitem action='SimulateOnAxisSource'/>"
	  "    </menu>"
	  "    <menu action='MenuData'>"
    "      <menuitem action='DiffToOriginal'/>"
    "      <menuitem action='BackToOriginal'/>"
    "      <separator/>"
    "      <menuitem action='ShowReal'/>"
    "      <menuitem action='ShowImaginary'/>"
    "      <menuitem action='ShowPhase'/>"
    "      <separator/>"
    "      <menuitem action='ShowStokesI'/>"
    "      <menuitem action='ShowStokesQ'/>"
    "      <menuitem action='ShowStokesU'/>"
    "      <menuitem action='ShowStokesV'/>"
    "      <separator/>"
    "      <menuitem action='ShowXX'/>"
    "      <menuitem action='ShowXY'/>"
    "      <menuitem action='ShowYX'/>"
    "      <menuitem action='ShowYY'/>"
    "      <menuitem action='ShowAutoPol'/>"
    "      <menuitem action='ShowCrossPol'/>"
    "      <menuitem action='UnrollPhase'/>"
    "      <separator/>"
    "      <menuitem action='StoreData'/>"
    "      <menuitem action='RecallData'/>"
    "      <menuitem action='SubtractDataFromMem'/>"
	  "    </menu>"
	  "    <menu action='MenuActions'>"
    "      <menuitem action='EditStrategy'/>"
    "      <menuitem action='ExecuteStrategy'/>"
    "      <separator/>"
    "      <menuitem action='Segment'/>"
    "      <menuitem action='Cluster'/>"
    "      <menuitem action='Classify'/>"
    "      <menuitem action='RemoveSmallSegments'/>"
    "      <separator/>"
    "      <menuitem action='TimeMergeUnsetValues'/>"
    "      <menuitem action='VertEVD'/>"
    "      <menuitem action='ApplyTimeProfile'/>"
    "      <menuitem action='ApplyVertProfile'/>"
    "      <menuitem action='RestoreTimeProfile'/>"
    "      <menuitem action='RestoreVertProfile'/>"
    "      <menuitem action='ReapplyTimeProfile'/>"
    "      <menuitem action='ReapplyVertProfile'/>"
	  "    </menu>"
    "  </menubar>"
    "  <toolbar  name='ToolBar'>"
    "    <toolitem action='OpenDirectory'/>"
    "    <separator/>"
    "    <toolitem action='ClearAltFlags'/>"
    "    <toolitem action='ShowStats'/>"
    "    <separator/>"
    "    <toolitem action='Previous'/>"
    "    <toolitem action='Next'/>"
    "    <toolitem action='OriginalFlags'/>"
    "    <toolitem action='AlternativeFlags'/>"
    "    <toolitem action='Highlight'/>"
    "    <separator/>"
    "    <toolitem action='ImageOriginal'/>"
    "    <toolitem action='ImageBackground'/>"
    "    <toolitem action='ImageDiff'/>"
    "  </toolbar>"
    "</ui>";

	uiManager->add_ui_from_string(ui_info);
	Gtk::Widget* pMenubar = uiManager->get_widget("/MenuBar");
	_mainVBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);
	Gtk::Widget* pToolbar = uiManager->get_widget("/ToolBar");
	static_cast<Gtk::Toolbar *>(pToolbar)->set_toolbar_style(Gtk::TOOLBAR_BOTH);
	_mainVBox.pack_start(*pToolbar, Gtk::PACK_SHRINK);
	pMenubar->show();
}

void MSWindow::onClearAltFlagsPressed()
{
	_timeFrequencyWidget.Update();
}

void MSWindow::onDifferenceToOriginalPressed()
{
	if(HasImage())
	{
		TimeFrequencyData data(_timeFrequencyWidget.ContaminatedData());
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
	}
	if(_originalImageButton->get_active())
		_timeFrequencyWidget.Update();
	else
		_originalImageButton->set_active();
}

void MSWindow::onBackgroundToOriginalPressed()
{
	if(HasImage())
	{
		TimeFrequencyData data(_timeFrequencyWidget.RevisedData());
		_timeFrequencyWidget.ClearBackground();
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
	}
	if(_originalImageButton->get_active())
		_timeFrequencyWidget.Update();
	else
		_originalImageButton->set_active();
}

void MSWindow::onHightlightPressed()
{
	if(_highlightWindow != 0)
		delete _highlightWindow;
	_highlightWindow = new HighlightWindow(*this);
	_highlightWindow->show();
}

void MSWindow::onAddStaticFringe()
{
	try {
		if(HasImage())
		{
			TimeFrequencyMetaDataCPtr metaData = TimeFrequencyMetaData();
			TimeFrequencyData data(GetActiveData());
			FringeTestCreater::AddStaticFringe(data, metaData, 1.0L);
			_timeFrequencyWidget.SetNewData(data, metaData);
			_timeFrequencyWidget.Update();
		}
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onAdd1SigmaFringe()
{
	try {
		if(HasImage())
		{
			TimeFrequencyMetaDataCPtr metaData = TimeFrequencyMetaData();
			num_t mean, stddev;
			TimeFrequencyData data(GetActiveData());
			ThresholdTools::MeanAndStdDev(data.GetRealPart(), data.GetSingleMask(), mean, stddev);
			FringeTestCreater::AddStaticFringe(data, metaData, stddev);
			_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
			_timeFrequencyWidget.Update();
		}
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onSetToOne()
{
	try {
		TimeFrequencyData data(GetActiveData());
		std::pair<Image2DCPtr, Image2DCPtr> images = data.GetSingleComplexImage();
		Image2DPtr
			real = Image2D::CreateCopy(images.first),
			imaginary = Image2D::CreateCopy(images.first);
		real->SetAll(1.0);
		imaginary->SetAll(0.0);
		TimeFrequencyData newData(data.Polarisation(), real, imaginary);
		newData.SetMask(data);
		_timeFrequencyWidget.SetNewData(newData, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onSetToI()
{
	try {
		TimeFrequencyData data(GetActiveData());
		std::pair<Image2DCPtr, Image2DCPtr> images = data.GetSingleComplexImage();
		Image2DPtr
			real = Image2D::CreateCopy(images.first),
			imaginary = Image2D::CreateCopy(images.first);
		real->SetAll(0.0);
		imaginary->SetAll(1.0);
		TimeFrequencyData newData(data.Polarisation(), real, imaginary);
		newData.SetMask(data);
		_timeFrequencyWidget.SetNewData(newData, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onSetToOnePlusI()
{
	try {
		TimeFrequencyData data(GetActiveData());
		std::pair<Image2DCPtr, Image2DCPtr> images = data.GetSingleComplexImage();
		Image2DPtr
			real = Image2D::CreateCopy(images.first),
			imaginary = Image2D::CreateCopy(images.first);
		real->SetAll(1.0);
		imaginary->SetAll(1.0);
		TimeFrequencyData newData(data.Polarisation(), real, imaginary);
		newData.SetMask(data);
		_timeFrequencyWidget.SetNewData(newData, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onShowStats()
{
	if(_timeFrequencyWidget.HasImage())
	{
		TimeFrequencyData activeData = GetActiveData();
		TimeFrequencyStatistics statistics(activeData);
		std::stringstream s;
		s << "Percentage flagged: " << TimeFrequencyStatistics::FormatRatio(statistics.GetFlaggedRatio()) << "\n";
			
		Mask2DCPtr
			original = _timeFrequencyWidget.OriginalMask(),
			alternative = _timeFrequencyWidget.AlternativeMask();
		Mask2DPtr
			intersect;
		if(original != 0 && alternative != 0)
		{
			intersect = Mask2D::CreateCopy(original);
			intersect->Intersect(alternative);
			
			unsigned intCount = intersect->GetCount<true>();
			if(intCount != 0)
			{
				if(!original->Equals(alternative))
				{
					s << "Overlap between original and alternative: " << TimeFrequencyStatistics::FormatRatio((double) intCount / ((double) (original->Width() * original->Height()))) << "\n"
					<< "(relative to alternative flags: " << TimeFrequencyStatistics::FormatRatio((double) intCount / ((double) (alternative->GetCount<true>()))) << ")\n";
					
				}
			}
		}
		
		Image2DCPtr powerImg = activeData.GetSingleImage();
		Mask2DCPtr mask = activeData.GetSingleMask();
		double power = 0.0;
		for(unsigned y=0;y<powerImg->Height();++y)
		{
			for(unsigned x=0;x<powerImg->Width();++x)
			{
				if(!mask->Value(x, y) && std::isfinite(powerImg->Value(x, y)))
				{
					power += powerImg->Value(x, y);
				}
			}
		}
		s << "Total unflagged power: " << power << "\n";
		Gtk::MessageDialog dialog(*this, s.str(), false, Gtk::MESSAGE_INFO);
		dialog.run();
	}
}

void MSWindow::onPlotDistPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Distribution");

		TimeFrequencyData activeData = GetActiveData();
		Image2DCPtr image = activeData.GetSingleImage();
		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
		Plot2DPointSet &totalSet = plot.StartLine("Total");
		RFIPlots::MakeDistPlot(totalSet, image, mask);

		Plot2DPointSet &uncontaminatedSet = plot.StartLine("Uncontaminated");
		mask = Mask2D::CreateCopy(activeData.GetSingleMask());
		RFIPlots::MakeDistPlot(uncontaminatedSet, image, mask);

		mask->Invert();
		Plot2DPointSet &rfiSet = plot.StartLine("RFI");
		RFIPlots::MakeDistPlot(rfiSet, image, mask);

		_plotManager.Update();
	}
}

void MSWindow::onPlotLogLogDistPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		TimeFrequencyData activeData = GetActiveData();
		HistogramCollection histograms(activeData.PolarisationCount());
		for(unsigned p=0;p!=activeData.PolarisationCount();++p)
		{
			TimeFrequencyData *polData = activeData.CreateTFDataFromPolarisationIndex(p);
			Image2DCPtr image = polData->GetSingleImage();
			Mask2DCPtr mask = Mask2D::CreateCopy(polData->GetSingleMask());
			histograms.Add(0, 1, p, image, mask);
		}
		if(_histogramWindow == 0)
			_histogramWindow = new HistogramWindow(histograms);
		else
			_histogramWindow->SetStatistics(histograms);
		_histogramWindow->show();
	}
}

void MSWindow::onPlotComplexPlanePressed()
{
	if(HasImage()) {
		if(_plotComplexPlaneWindow != 0)
			delete _plotComplexPlaneWindow;
		_plotComplexPlaneWindow = new ComplexPlanePlotWindow(*this, _plotManager);
		_plotComplexPlaneWindow->show();
	}
}

template<bool Weight>
void MSWindow::plotMeanSpectrumPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Mean spectrum");

		TimeFrequencyData data = _timeFrequencyWidget.GetActiveData();
		Mask2DCPtr mask =
			Mask2D::CreateSetMaskPtr<false>(data.ImageWidth(), data.ImageHeight());
		Plot2DPointSet &beforeSet = plot.StartLine("Without flagging");
		RFIPlots::MakeMeanSpectrumPlot<Weight>(beforeSet, data, mask, _timeFrequencyWidget.GetMetaData());

		mask = Mask2D::CreateCopy(data.GetSingleMask());
		if(!mask->AllFalse())
		{
			Plot2DPointSet &afterSet = plot.StartLine("Flagged");
			RFIPlots::MakeMeanSpectrumPlot<Weight>(afterSet, data, mask, _timeFrequencyWidget.GetMetaData());
		}
		
		_plotManager.Update();
	}
}

void MSWindow::onPlotPowerSpectrumPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Power spectrum");
		plot.SetLogarithmicYAxis(true);

		TimeFrequencyData data = _timeFrequencyWidget.GetActiveData();
		Image2DCPtr image = data.GetSingleImage();
		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
		Plot2DPointSet &beforeSet = plot.StartLine("Before");
		RFIPlots::MakePowerSpectrumPlot(beforeSet, image, mask, _timeFrequencyWidget.GetMetaData());

		mask = Mask2D::CreateCopy(data.GetSingleMask());
		if(!mask->AllFalse())
		{
			Plot2DPointSet &afterSet = plot.StartLine("After");
			RFIPlots::MakePowerSpectrumPlot(afterSet, image, mask, _timeFrequencyWidget.GetMetaData());
		}
		
		_plotManager.Update();
	}
}

void MSWindow::onPlotPowerSpectrumComparisonPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Power spectrum comparison");

		TimeFrequencyData data = _timeFrequencyWidget.OriginalData();
		Image2DCPtr image = data.GetSingleImage();
		Mask2DCPtr mask = data.GetSingleMask();
		Plot2DPointSet &originalSet = plot.StartLine("Original");
		RFIPlots::MakePowerSpectrumPlot(originalSet, image, mask, _timeFrequencyWidget.GetMetaData());

		data = _timeFrequencyWidget.ContaminatedData();
		image = data.GetSingleImage();
		mask = data.GetSingleMask();
		Plot2DPointSet &alternativeSet = plot.StartLine("Alternative");
		RFIPlots::MakePowerSpectrumPlot(alternativeSet, image, mask, _timeFrequencyWidget.GetMetaData());
	
		_plotManager.Update();
	}
}

void MSWindow::onPlotPowerRMSPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Spectrum RMS");
		plot.SetLogarithmicYAxis(true);

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(_timeFrequencyWidget.Image()->Width(), _timeFrequencyWidget.Image()->Height());
		Plot2DPointSet &beforeSet = plot.StartLine("Before");
		RFIPlots::MakeRMSSpectrumPlot(beforeSet, _timeFrequencyWidget.Image(), mask);

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			Plot2DPointSet &afterSet = plot.StartLine("After");
			RFIPlots::MakeRMSSpectrumPlot(afterSet, _timeFrequencyWidget.Image(), mask);
	
			//mask->Invert();
			//Plot2DPointSet &rfiSet = plot.StartLine("RFI");
			//RFIPlots::MakeRMSSpectrumPlot(rfiSet, _timeFrequencyWidget.Image(), mask);
		}

		_plotManager.Update();
	}
}

void MSWindow::onPlotPowerSNRPressed()
{
	Image2DCPtr
		image = _timeFrequencyWidget.GetActiveData().GetSingleImage(),
		model = _timeFrequencyWidget.RevisedData().GetSingleImage();
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("SNR spectrum");
		plot.SetLogarithmicYAxis(true);

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
		Plot2DPointSet &totalPlot = plot.StartLine("Total");
		RFIPlots::MakeSNRSpectrumPlot(totalPlot, image, model, mask);

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			Plot2DPointSet &uncontaminatedPlot = plot.StartLine("Uncontaminated");
			RFIPlots::MakeSNRSpectrumPlot(uncontaminatedPlot, image, model, mask);
	
			mask->Invert();
			Plot2DPointSet &rfiPlot = plot.StartLine("RFI");
			RFIPlots::MakeSNRSpectrumPlot(rfiPlot, image, model, mask);
		}

		_plotManager.Update();
	}
}

void MSWindow::onPlotPowerTimePressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Power over time");
		plot.SetLogarithmicYAxis(true);

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(_timeFrequencyWidget.Image()->Width(), _timeFrequencyWidget.Image()->Height());
		Plot2DPointSet &totalPlot = plot.StartLine("Total");
		RFIPlots::MakePowerTimePlot(totalPlot, _timeFrequencyWidget.Image(), mask, _timeFrequencyWidget.GetMetaData());

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			Plot2DPointSet &uncontaminatedPlot = plot.StartLine("Uncontaminated");
			RFIPlots::MakePowerTimePlot(uncontaminatedPlot, _timeFrequencyWidget.Image(), mask, _timeFrequencyWidget.GetMetaData());
	
			mask->Invert();
			Plot2DPointSet &rfiPlot = plot.StartLine("RFI");
			RFIPlots::MakePowerTimePlot(rfiPlot, _timeFrequencyWidget.Image(), mask, _timeFrequencyWidget.GetMetaData());
		}

		_plotManager.Update();
	}
}

void MSWindow::onPlotPowerTimeComparisonPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Time comparison");

		TimeFrequencyData data = _timeFrequencyWidget.OriginalData();
		Mask2DCPtr mask = data.GetSingleMask();
		Image2DCPtr image = data.GetSingleImage();
		Plot2DPointSet &originalPlot = plot.StartLine("Original");
		RFIPlots::MakePowerTimePlot(originalPlot, image, mask, _timeFrequencyWidget.GetMetaData());

		data = _timeFrequencyWidget.ContaminatedData();
		mask = data.GetSingleMask();
		image = data.GetSingleImage();
		Plot2DPointSet &alternativePlot = plot.StartLine("Original");
		plot.StartLine("Alternative");
		RFIPlots::MakePowerTimePlot(alternativePlot, image, mask, _timeFrequencyWidget.GetMetaData());

		_plotManager.Update();
	}
}

void MSWindow::onPlotTimeScatterPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		MultiPlot plot(_plotManager.NewPlot2D("Time scatter"), 4);
		RFIPlots::MakeScatterPlot(plot, GetActiveData(), _timeFrequencyWidget.GetMetaData());
		plot.Finish();
		_plotManager.Update();
	}
}

void MSWindow::onPlotTimeScatterComparisonPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		MultiPlot plot(_plotManager.NewPlot2D("Time scatter comparison"), 8);
		RFIPlots::MakeScatterPlot(plot, GetOriginalData(), _timeFrequencyWidget.GetMetaData(), 0);
		RFIPlots::MakeScatterPlot(plot, GetContaminatedData(), _timeFrequencyWidget.GetMetaData(), 4);
		plot.Finish();
		_plotManager.Update();
	}
}

void MSWindow::onPlotSingularValuesPressed()
{
	if(HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Singular values");

		SVDMitigater::CreateSingularValueGraph(GetActiveData(), plot);
		_plotManager.Update();
	}
}

void MSWindow::onPlotQuality25Pressed()
{
	if(HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Quality over 25");
		RFIPlots::MakeQualityPlot(plot.StartLine(), GetActiveData(), _timeFrequencyWidget.RevisedData(), 25);
		_plotManager.Update();
	}
}

void MSWindow::onPlotQualityAllPressed()
{
	if(HasImage())
	{
		Plot2D &plot = _plotManager.NewPlot2D("Quality over all");
		RFIPlots::MakeQualityPlot(plot.StartLine(), GetActiveData(), _timeFrequencyWidget.RevisedData(), _timeFrequencyWidget.RevisedData().ImageWidth());
		_plotManager.Update();
	}
}

void MSWindow::onPlotSNRToFitVariance()
{
	bool relative = false;

	FringeStoppingFitter fitter;
	fitter.SetMetaData(_timeFrequencyWidget.GetMetaData());
	
	Plot2D
		&plotA = _plotManager.NewPlot2D("/tmp/snrplot-a.pdf"),
		&plotB = _plotManager.NewPlot2D("/tmp/snrplot-b.pdf");
	plotA.StartLine("Stddev", "SNR (dB)", "Error (sigma-epsilon)");
	plotA.SetTitle("Fit errors");
	plotA.SetLogarithmicYAxis(false);
	plotB.StartLine("Stddev", "SNR (ratio, non-logarithmic)", "Error (sigma-epsilon)");
	plotB.SetTitle("Fit errors");
	plotB.SetLogarithmicYAxis(false);

	const unsigned iterations = 2500;
	std::vector<long double> medians, means, maxs, snrDbs, snrRatios;

	long double start = 4.6;
	long double stop = 0.0001;
	if(relative)
		stop = 0.01;

	for(long double snr = start;snr>stop;snr *= 0.9) {
		long double amplitudes[iterations], mean = 0, stddev = 0;
		long double db = 10.0 * logl(snr) / logl(10.0L);
		long double max = -100e10;
		for(size_t i=0;i<iterations;++i)
		{
			unsigned width = 1024, height = 1;
			if(HasImage())
			{
				width = _timeFrequencyWidget.OriginalData().ImageWidth();
				//height = _timeFrequencyWidget.Image()->Height();
			}
			width /= 16;

			Mask2DPtr rfi = Mask2D::CreateSetMaskPtr<false>(width, height);
			Image2DPtr testSetReal(MitigationTester::CreateTestSet(2, rfi, width, height, _gaussianTestSets));
			Image2DPtr testSetImaginary(MitigationTester::CreateTestSet(2, rfi, width, height, _gaussianTestSets));
			TimeFrequencyData *data = new TimeFrequencyData(SinglePolarisation, testSetReal, testSetImaginary);
	
			TimeFrequencyMetaDataCPtr metaData = TimeFrequencyMetaData();
			FringeTestCreater::AddStaticFringe(*data, metaData, snr);
	
			fitter.Initialize(*data);
			amplitudes[i] =
				fitter.GetAmplitude(height/2, height/2+1) - snr;
			mean += amplitudes[i];
	
			delete data;
		}
		mean /= iterations;
		for(size_t i=0;i<iterations;++i)
		{
			stddev += (amplitudes[i] - mean) * (amplitudes[i] - mean);
			if(amplitudes[i] > max) max = amplitudes[i];
		}
		stddev = sqrtl(stddev / iterations);
		unsigned medianIndex = iterations/2;
		std::nth_element(amplitudes, amplitudes + medianIndex, amplitudes+iterations);
		std::cout << "Snr: " << snr << " (=" << db << " dB), stddev: " << stddev << ", median: " << amplitudes[medianIndex] << ", max: " << max << std::endl;
		if(relative)
		{
			plotA.PushDataPoint(db, stddev/snr);
			plotB.PushDataPoint(snr, stddev/snr);
		} else {
			plotA.PushDataPoint(db, stddev);
			plotB.PushDataPoint(snr, stddev);
		}
		medians.push_back(amplitudes[medianIndex]);
		snrRatios.push_back(snr);
		snrDbs.push_back(db);
		means.push_back(mean);
		maxs.push_back(max);
	}
	plotA.StartLine("median");
	plotB.StartLine("median");
	for(unsigned i=0;i<snrDbs.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], medians[i]/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], medians[i]/snrRatios[i]);
		} else {
			plotA.PushDataPoint(snrDbs[i], medians[i]);
			plotB.PushDataPoint(snrRatios[i], medians[i]);
		}
	}
	plotA.StartLine("mean");
	plotB.StartLine("mean");
	for(unsigned i=0;i<means.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], means[i]/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], means[i]/snrRatios[i]);
		} else {
			plotA.PushDataPoint(snrDbs[i], means[i]);
			plotB.PushDataPoint(snrRatios[i], means[i]);
		}
	}
	plotA.StartLine("max");
	plotB.StartLine("max");
	for(unsigned i=0;i<maxs.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], maxs[i]/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], maxs[i]/snrRatios[i]);
		} else {
			plotA.PushDataPoint(snrDbs[i], maxs[i]);
			plotB.PushDataPoint(snrRatios[i], maxs[i]);
		}
	}
	plotA.StartLine("snr=error");
	plotB.StartLine("snr=error");
	for(unsigned i=0;i<means.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], powl(10.0L, snrDbs[i]/10.0L)/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], 1);
		} else {
			if(powl(10.0L, snrDbs[i]/10.0L) < 0.45)
			{
				plotA.PushDataPoint(snrDbs[i], powl(10.0L, snrDbs[i]/10.0L));
			}
			plotB.PushDataPoint(snrRatios[i], snrRatios[i]);
		}
	}
	_plotManager.Update();
}

void MSWindow::onImagePropertiesPressed()
{
	if(_imagePropertiesWindow != 0)
		delete _imagePropertiesWindow;
	_imagePropertiesWindow = new ImagePropertiesWindow(_timeFrequencyWidget, "Time-frequency plotting options");
	_imagePropertiesWindow->show();
}

void MSWindow::showPhasePart(enum TimeFrequencyData::PhaseRepresentation phaseRepresentation)
{
	if(HasImage())
	{
		try {
			TimeFrequencyData *newPart =  _timeFrequencyWidget.GetActiveData().CreateTFData(phaseRepresentation);
			_timeFrequencyWidget.SetNewData(*newPart, _timeFrequencyWidget.GetMetaData());
			delete newPart;
			_timeFrequencyWidget.Update();
		} catch(std::exception &e)
		{
			std::stringstream errstr;
			errstr
				<< "The data that was currently in memory could not be converted to the requested "
				   "type. The error given by the converter was:\n"
				<< e.what()
				<< "\n\n"
				<< "Note that if the original data should be convertable to this type, but "
				   "you have already used one of the 'Keep ..' buttons, you first need to reload "
					 "the full data with Goto -> Load.\n\n"
					 "(alternatively, if loading takes a lot of time, you can use the Store and Recall"
					 " options in the Data menu)";
			showError(errstr.str());
		}
	}
}

void MSWindow::showPolarisation(enum PolarisationType polarisation)
{
	if(HasImage())
	{
		try {
			TimeFrequencyData *newData =
				_timeFrequencyWidget.GetActiveData().CreateTFData(polarisation);
			_timeFrequencyWidget.SetNewData(*newData, _timeFrequencyWidget.GetMetaData());
			delete newData;
			_timeFrequencyWidget.Update();
		} catch(std::exception &e)
		{
			std::stringstream errstr;
			errstr
				<< "The data that was currently in memory could not be converted to the requested "
				   "polarization. The error given by the converter was:\n"
				<< e.what()
				<< "\n\n"
				<< "Note that if the original data should be convertable to this polarization, but "
				   "you have already used one of the 'Keep ..' buttons, you first need to reload "
					 "the full data with Goto -> Load.\n\n"
					 "(alternatively, if loading takes a lot of time, you can use the Store and Recall"
					 " options in the Data menu)";
			showError(errstr.str());
		}
	}
}

void MSWindow::onGoToPressed()
{
	if(HasImageSet())
	{
		rfiStrategy::MSImageSet *msSet = dynamic_cast<rfiStrategy::MSImageSet*>(_imageSet);
		if(msSet != 0)
		{
			if(_gotoWindow != 0)
				delete _gotoWindow;
			_gotoWindow = new GoToWindow(*this);
			_gotoWindow->show();
			} else {
			showError("Can not goto in this image set; format does not support goto");
		}
	}
}

void MSWindow::onTFWidgetMouseMoved(size_t x, size_t y)
{
	Image2DCPtr image = _timeFrequencyWidget.Image();
	num_t v = image->Value(x, y);
	_statusbar.pop();
	std::stringstream s;
		s << "x=" << x << ",y=" << y << ",value=" << v;
	TimeFrequencyMetaDataCPtr metaData =_timeFrequencyWidget.GetMetaData();
	if(metaData != 0)
	{
		if(metaData->HasObservationTimes() && metaData->HasBand())
		{
			const std::vector<double> &times = _timeFrequencyWidget.GetMetaData()->ObservationTimes();
			s << " (t=" << Date::AipsMJDToString(times[x]) <<
			", f=" << Frequency::ToString(_timeFrequencyWidget.GetMetaData()->Band().channels[y].frequencyHz);
		}
		
		if(metaData->HasUVW())
		{
			UVW uvw = metaData->UVW()[x];
			s << ", uvw=" << uvw.u << "," << uvw.v << "," << uvw.w;
		}
		s << ')';
	}
	_statusbar.push(s.str(), 0);
}

void MSWindow::onShowImagePlane()
{
	_imagePlaneWindow->show();
}

void MSWindow::onSetAndShowImagePlane()
{
	_imagePlaneWindow->GetImager()->Empty();
	onAddToImagePlane();
	_imagePlaneWindow->show();
	_imagePlaneWindow->GetImager()->ApplyWeightsToUV();
	_imagePlaneWindow->Update();
}

void MSWindow::onAddToImagePlane()
{
	try {
		if(_timeFrequencyWidget.GetMetaData() != 0 && _timeFrequencyWidget.GetMetaData()->HasUVW())
		{
			TimeFrequencyData activeData = GetActiveData();
			if(activeData.PolarisationCount() != 1)
			{
				TimeFrequencyData *singlePolarization = activeData.CreateTFData(StokesIPolarisation);
				activeData = *singlePolarization;
				delete singlePolarization;
			}
			_imagePlaneWindow->AddData(activeData, _timeFrequencyWidget.GetMetaData());
		}
		else if(_spatialMetaData != 0)
			_imagePlaneWindow->AddData(GetActiveData(), _spatialMetaData);
		else
			showError("No meta data found.");
	}  catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onMultiplyData()
{
	TimeFrequencyData data(GetActiveData());
	data.MultiplyImages(2.0L);
	_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
	_timeFrequencyWidget.Update();
}

void MSWindow::onSegment()
{
	_segmentedImage = SegmentedImage::CreateUnsetPtr(GetOriginalData().ImageWidth(),  GetOriginalData().ImageHeight());
	Morphology morphology;
	morphology.SegmentByLengthRatio(GetActiveData().GetSingleMask(), _segmentedImage);
	_timeFrequencyWidget.SetSegmentedImage(_segmentedImage);
	Update();
}

void MSWindow::onCluster()
{
	if(_segmentedImage != 0)
	{
		Morphology morphology;
		morphology.Cluster(_segmentedImage);
	_timeFrequencyWidget.SetSegmentedImage(_segmentedImage);
		Update();
	}
}

void MSWindow::onClassify()
{
	if(_segmentedImage != 0)
	{
		Morphology morphology;
		morphology.Classify(_segmentedImage);
		_timeFrequencyWidget.SetSegmentedImage(_segmentedImage);
		Update();
	}
}

void MSWindow::onRemoveSmallSegments()
{
	if(_segmentedImage != 0)
	{
		Morphology morphology;
		morphology.RemoveSmallSegments(_segmentedImage, 4);
	_timeFrequencyWidget.SetSegmentedImage(_segmentedImage);
		Update();
	}
}

void MSWindow::onTimeGraphButtonPressed()
{
	if(_timeGraphButton->get_active())
	{
		_mainVBox.remove(_timeFrequencyWidget);
		_mainVBox.pack_start(_panedArea);
		_panedArea.pack1(_timeFrequencyWidget, true, true);
		_panedArea.pack2(_plotFrame, true, true);

		_panedArea.show();
		_timeFrequencyWidget.show();
		_plotFrame.show();
	} else {
		_mainVBox.remove(_panedArea);
		_panedArea.remove(_timeFrequencyWidget);
		_panedArea.remove(_plotFrame);

		_mainVBox.pack_start(_timeFrequencyWidget);
		_timeFrequencyWidget.show();
	}
}

void MSWindow::onTFWidgetButtonReleased(size_t x, size_t y)
{
	if(HasImage())
	{
		if(_plotFrame.is_visible())
		{
			_plotFrame.SetTimeFrequencyData(GetActiveData());
			_plotFrame.SetSelectedSample(x, y);
		
			_plotFrame.Update();
		}
	}
}

void MSWindow::onUnrollPhaseButtonPressed()
{
	if(HasImage())
	{
		TimeFrequencyData *data =
			GetActiveData().CreateTFData(TimeFrequencyData::PhasePart);
		for(unsigned i=0;i<data->ImageCount();++i)
		{
			Image2DPtr image = Image2D::CreateCopy(data->GetImage(i));
			ThresholdTools::UnrollPhase(image);
			data->SetImage(i, image);
		}
		_timeFrequencyWidget.SetNewData(*data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
		delete data;
	}
}

void MSWindow::showError(const std::string &description)
{
	Gtk::MessageDialog dialog(*this, description, false, Gtk::MESSAGE_ERROR);
	dialog.run();
}

DefaultModels::SetLocation MSWindow::getSetLocation(bool empty)
{
	if(empty)
		return DefaultModels::EmptySet;
	if(_ncpSetButton->get_active())
		return DefaultModels::NCPSet;
	else if(_b1834SetButton->get_active())
		return DefaultModels::B1834Set;
	else
		return DefaultModels::EmptySet;
}

void MSWindow::loadDefaultModel(DefaultModels::Distortion distortion, bool withNoise, bool empty)
{
	unsigned channelCount;
	if(_sim16ChannelsButton->get_active())
		channelCount = 16;
	else if(_sim64ChannelsButton->get_active())
		channelCount = 64;
	else
		channelCount = 256;
	double bandwidth;
	if(_simFixBandwidthButton->get_active())
		bandwidth = 16.0 * 2500000.0;
	else
		bandwidth = (double) channelCount / 16.0 * 2500000.0;
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> pair = DefaultModels::LoadSet(getSetLocation(empty), distortion, withNoise ? 1.0 : 0.0, channelCount, bandwidth);
	TimeFrequencyData data = pair.first;
	TimeFrequencyMetaDataCPtr metaData = pair.second;
	
	_timeFrequencyWidget.SetNewData(data, metaData);
	_timeFrequencyWidget.Update();
}

void MSWindow::onCompress()
{
	Compress compress = Compress(GetActiveData());
	compress.AllToStdOut();
}

void MSWindow::onShowAntennaMapWindow()
{
	if(_antennaMapWindow != 0)
		delete _antennaMapWindow;
	AntennaMapWindow *newWindow = new AntennaMapWindow();
	_antennaMapWindow = newWindow;
	rfiStrategy::MSImageSet *msImageSet = dynamic_cast<rfiStrategy::MSImageSet*>(_imageSet);
	if(msImageSet != 0)
	{
		MeasurementSet &set = msImageSet->Reader()->Set();
		newWindow->SetMeasurementSet(set);
	}
	newWindow->show();
}

void MSWindow::onVertEVD()
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		TimeFrequencyData old(data);
		VertEVD::Perform(data, true);
		TimeFrequencyData *diff = TimeFrequencyData::CreateTFDataFromDiff(old, data);
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.SetRevisedData(*diff);
		delete diff;
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onApplyTimeProfile()
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_horProfile.size() != data.ImageWidth())
		{
			_horProfile.clear();
			for(unsigned i=0;i<data.ImageWidth();++i)
				_horProfile.push_back(1.0);
		}
		
		Image2DCPtr weights = data.GetSingleImage();
		for(unsigned i=0;i<data.ImageCount();++i)
		{
			Image2DCPtr input = data.GetImage(i);
			Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
			for(unsigned x=0;x<weights->Width();++x)
			{
				num_t timeAvg = 0.0;
				for(unsigned y=0;y<weights->Height();++y)
				{
					if(std::isfinite(weights->Value(x, y)))
						timeAvg += weights->Value(x, y);
				}
				timeAvg /= (num_t) weights->Height();
				_horProfile[x] = timeAvg;
				for(unsigned y=0;y<input->Height();++y)
				{
					output->SetValue(x, y, input->Value(x, y) * timeAvg);
				}
			}
			data.SetImage(i, output);
		}
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onApplyVertProfile()
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_vertProfile.size() != data.ImageHeight())
		{
			_vertProfile.clear();
			for(unsigned i=0;i<data.ImageHeight();++i)
				_vertProfile.push_back(1.0);
		}

		Image2DCPtr weights = data.GetSingleImage();
		for(unsigned i=0;i<data.ImageCount();++i)
		{
			Image2DCPtr input = data.GetImage(i);
			Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
			for(unsigned y=0;y<weights->Height();++y)
			{
				num_t vertAvg = 0.0;
				for(unsigned x=0;x<weights->Width();++x)
				{
					if(std::isfinite(weights->Value(x, y)))
						vertAvg += weights->Value(x, y);
				}
				vertAvg /= (num_t) weights->Width();
				_vertProfile[y] = vertAvg;
				for(unsigned x=0;x<input->Width();++x)
				{
					output->SetValue(x, y, input->Value(x, y) * vertAvg);
				}
			}
			data.SetImage(i, output);
		}
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onUseTimeProfile(bool inverse)
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_horProfile.size()==data.ImageWidth())
		{
			for(unsigned i=0;i<data.ImageCount();++i)
			{
				Image2DCPtr input = data.GetImage(i);
				Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
				for(unsigned x=0;x<input->Width();++x)
				{
					for(unsigned y=0;y<input->Height();++y)
					{
						if(inverse)
						{
							if(_horProfile[x] != 0.0)
								output->SetValue(x, y, input->Value(x, y) / _horProfile[x]);
							else
								output->SetValue(x, y, 0.0);
						} else {
								output->SetValue(x, y, input->Value(x, y) * _horProfile[x]);
						}
					}
				}
				data.SetImage(i, output);
			}
			_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
			_timeFrequencyWidget.Update();
		}
	}
}

void MSWindow::onUseVertProfile(bool inverse)
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_vertProfile.size()==data.ImageHeight())
		{
			TimeFrequencyData data = GetActiveData();
			for(unsigned i=0;i<data.ImageCount();++i)
			{
				Image2DCPtr input = data.GetImage(i);
				Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
				for(unsigned x=0;x<input->Width();++x)
				{
					for(unsigned y=0;y<input->Height();++y)
					{
						if(inverse)
						{
							if(_vertProfile[y] != 0.0)
								output->SetValue(x, y, input->Value(x, y) / _vertProfile[y]);
							else
								output->SetValue(x, y, 0.0);
						} else {
								output->SetValue(x, y, input->Value(x, y) * _vertProfile[y]);
						}
					}
				}
				data.SetImage(i, output);
			}
			_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
			_timeFrequencyWidget.Update();
		}
	}
}

void MSWindow::onStoreData()
{
	if(HasImage())
	{
		_storedData = _timeFrequencyWidget.GetActiveData();
	}
}

void MSWindow::onRecallData()
{
	_timeFrequencyWidget.SetNewData(_storedData, _timeFrequencyWidget.GetMetaData());
	_timeFrequencyWidget.Update();
}

void MSWindow::onSubtractDataFromMem()
{
	if(HasImage())
	{
		TimeFrequencyData activeData = _timeFrequencyWidget.GetActiveData();
		TimeFrequencyData *diffData = TimeFrequencyData::CreateTFDataFromDiff(_storedData, activeData);
		_timeFrequencyWidget.SetNewData(*diffData, _timeFrequencyWidget.GetMetaData());
		delete diffData;
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onTimeMergeUnsetValues()
{
	if(HasImage())
	{
		TimeFrequencyData activeData = _timeFrequencyWidget.GetActiveData();
		TimeFrequencyMetaDataPtr metaData(new class TimeFrequencyMetaData(*_timeFrequencyWidget.GetMetaData()));
		rfiStrategy::NoiseStatImageSet::MergeInTime(activeData, metaData);
		_timeFrequencyWidget.SetNewData(activeData, metaData);
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::SetStrategy(rfiStrategy::Strategy* newStrategy)
{
	delete _strategy;
	_strategy = newStrategy;
}
