#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include <sigc++/signal.h>

namespace rfiStrategy
{
	class Strategy;
}

class StrategyController
{
public:
	virtual void SetStrategy(rfiStrategy::Strategy *strategy) = 0;
	virtual rfiStrategy::Strategy &Strategy() = 0;
	virtual void NotifyChange() { _signalOnStrategyChanged(); }
	virtual sigc::signal<void> SignalOnStrategyChanged() { return _signalOnStrategyChanged; }
private:
	sigc::signal<void> _signalOnStrategyChanged;
};

#endif
