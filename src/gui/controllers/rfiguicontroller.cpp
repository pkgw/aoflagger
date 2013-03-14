#include "rfiguicontroller.h"

#include "../plot/plotmanager.h"

#include "../../strategy/plots/rfiplots.h"

#include "../rfiguiwindow.h"

RFIGuiController::RFIGuiController(RFIGuiWindow& rfiGuiWindow) :
	_rfiGuiWindow(rfiGuiWindow)
{
	_plotManager = new class PlotManager();
}

RFIGuiController::~RFIGuiController()
{
	delete _plotManager;
}

bool RFIGuiController::IsImageLoaded() const
{
	return _rfiGuiWindow.GetTimeFrequencyWidget().HasImage();
}

TimeFrequencyData RFIGuiController::ActiveData() const
{
	return _rfiGuiWindow.GetTimeFrequencyWidget().GetActiveData();
}

TimeFrequencyMetaDataCPtr RFIGuiController::MetaData() const
{
	return _rfiGuiWindow.GetTimeFrequencyWidget().GetMetaData();
}

void RFIGuiController::plotMeanSpectrum(bool weight)
{
	if(IsImageLoaded())
	{
		Plot2D &plot = _plotManager->NewPlot2D("Mean spectrum");

		TimeFrequencyData data = ActiveData();
		Mask2DCPtr mask =
			Mask2D::CreateSetMaskPtr<false>(data.ImageWidth(), data.ImageHeight());
		Plot2DPointSet &beforeSet = plot.StartLine("Without flagging");
		if(weight)
			RFIPlots::MakeMeanSpectrumPlot<true>(beforeSet, data, mask, MetaData());
		else
			RFIPlots::MakeMeanSpectrumPlot<false>(beforeSet, data, mask, MetaData());

		mask = Mask2D::CreateCopy(data.GetSingleMask());
		if(!mask->AllFalse())
		{
			Plot2DPointSet &afterSet = plot.StartLine("Flagged");
			if(weight)
				RFIPlots::MakeMeanSpectrumPlot<true>(afterSet, data, mask, MetaData());
			else
				RFIPlots::MakeMeanSpectrumPlot<false>(afterSet, data, mask, MetaData());
		}
		
		_plotManager->Update();
	}
}
