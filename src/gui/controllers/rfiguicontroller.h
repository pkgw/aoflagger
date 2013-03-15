#ifndef RFIGUI_CONTROLLER_H
#define RFIGUI_CONTROLLER_H

#include <sigc++/signal.h>

#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

class RFIGuiController
{
	public:
		RFIGuiController(class RFIGuiWindow &rfiGuiWindow);
		~RFIGuiController();
		
		bool AreOriginalFlagsShown() const { return _showOriginalFlags; }
		void SetShowOriginalFlags(bool showFlags) {
			if(_showOriginalFlags != showFlags)
			{
				_showOriginalFlags = showFlags;
				_signalStateChange();
			}
		}
		
		bool AreAlternativeFlagsShown() const { return _showAlternativeFlags; }
		void SetShowAlternativeFlags(bool showFlags) {
			if(_showAlternativeFlags != showFlags)
			{
				_showAlternativeFlags = showFlags;
				_signalStateChange();
			}
		}
		
		sigc::signal<void> &SignalStateChange()
		{ return _signalStateChange; }
		
		void PlotDist();
		void PlotLogLogDist();
		void PlotComplexPlane();
		void PlotMeanSpectrum() { plotMeanSpectrum(false); }
		void PlotSumSpectrum() { plotMeanSpectrum(true); }
		void PlotPowerSpectrum();
		void PlotPowerSpectrumComparison();
		void PlotPowerRMS();
		void PlotPowerSNR();
		void PlotPowerTime();
		void PlotPowerTimeComparison();
		void PlotTimeScatter();
		void PlotTimeScatterComparison();
		void PlotSingularValues();
		
		void OpenTestSet(unsigned index, bool gaussianTestSets);
		
		bool IsImageLoaded() const;
		
		TimeFrequencyData ActiveData() const;
		TimeFrequencyData OriginalData() const;
		TimeFrequencyData RevisedData() const;
		TimeFrequencyData ContaminatedData() const;
		
		TimeFrequencyMetaDataCPtr MetaData() const;
		
		class PlotManager &PlotManager() { return *_plotManager; }
		
	private:
		void plotMeanSpectrum(bool weight);
		
		bool _showOriginalFlags, _showAlternativeFlags;
		
		sigc::signal<void> _signalStateChange;
		class RFIGuiWindow &_rfiGuiWindow;
		
		class PlotManager *_plotManager;
};

#endif
