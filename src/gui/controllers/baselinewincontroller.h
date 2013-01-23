#ifndef BASELINE_WIN_CONTROLLER_H
#define BASELINE_WIN_CONTROLLER_H

#include <sigc++/signal.h>

class BaselineWindowController
{
	public:
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
	private:
		bool _showOriginalFlags, _showAlternativeFlags;
		
		sigc::signal<void> _signalStateChange;
};

#endif
