#include "rfiguicontroller.h"

#include "../rfiguiwindow.h"

RFIGuiController::RFIGuiController(RFIGuiWindow &rfiGuiWindow) :
	_rfiGuiWindow(rfiGuiWindow)
{
}

void RFIGuiController::plotMeanSpectrum(bool weight)
{
	if(IsImageLoaded())
	{
		/*Plot2D &plot = _plotManager.NewPlot2D("Mean spectrum");

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
		
		_plotManager.Update();*/
	}
}

bool RFIGuiController::IsImageLoaded() const
{
	return _rfiGuiWindow.GetTimeFrequencyWidget().HasImage();
}
