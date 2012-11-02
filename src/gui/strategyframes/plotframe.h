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
#ifndef STRATEGYFRAMES_PLOTFRAME_H
#define STRATEGYFRAMES_PLOTFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/plotaction.h"

#include "../editstrategywindow.h"

class StrategyPlotFrame : public Gtk::Frame {
	public:
		StrategyPlotFrame(rfiStrategy::PlotAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("For each baseline"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_plotKindLabel("Plot kind:"),
		_antennaVsFlagsButton("Antenna vs. flags"),
		_frequencyVsFlagsButton("Frequency vs. flags"),
		_frequencyVsPowerButton("Frequency vs. power"),
		_timeVsFlagsButton("Time vs. flgs"),
		_polarizationVsFlagsButton("Polarization vs. flgs"),
		_baselineSpectrumPlotButton("Spectrum per baseline"),
		_baselineRMSButton("Baseline RMS"),
		_iterationsButton("Iterations"),
		_logYScaleButton("Logarithmic y-axis"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_plotKindLabel);
			_plotKindLabel.show();

			Gtk::RadioButton::Group group;

			_box.pack_start(_antennaVsFlagsButton);
			_antennaVsFlagsButton.set_group(group);
			_antennaVsFlagsButton.show();

			_box.pack_start(_frequencyVsFlagsButton);
			_frequencyVsFlagsButton.set_group(group);
			_frequencyVsFlagsButton.show();

			_box.pack_start(_frequencyVsPowerButton);
			_frequencyVsPowerButton.set_group(group);
			_frequencyVsPowerButton.show();

			_box.pack_start(_timeVsFlagsButton);
			_timeVsFlagsButton.set_group(group);
			_timeVsFlagsButton.show();

			_box.pack_start(_polarizationVsFlagsButton);
			_polarizationVsFlagsButton.set_group(group);
			_polarizationVsFlagsButton.show();

			_box.pack_start(_baselineSpectrumPlotButton);
			_baselineSpectrumPlotButton.set_group(group);
			_baselineSpectrumPlotButton.show();

			_box.pack_start(_baselineRMSButton);
			_baselineRMSButton.set_group(group);
			_baselineRMSButton.show();

			_box.pack_start(_iterationsButton);
			_iterationsButton.set_group(group);
			_iterationsButton.show();

			switch(_action.PlotKind())
			{
				case rfiStrategy::PlotAction::AntennaFlagCountPlot:
					_antennaVsFlagsButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::FrequencyFlagCountPlot:
					_frequencyVsFlagsButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::FrequencyPowerPlot:
					_frequencyVsPowerButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::TimeFlagCountPlot:
					_timeVsFlagsButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::BaselineSpectrumPlot:
					_baselineSpectrumPlotButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::PolarizationStatisticsPlot:
					_polarizationVsFlagsButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::BaselineRMSPlot:
					_baselineRMSButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::IterationsPlot:
					_iterationsButton.set_active(true);
					break;
			}

			_logYScaleButton.set_active(_action.LogarithmicYAxis());
			_box.pack_start(_logYScaleButton);
			_logYScaleButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &StrategyPlotFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::PlotAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _plotKindLabel;
		Gtk::RadioButton
			_antennaVsFlagsButton, _frequencyVsFlagsButton, _frequencyVsPowerButton, _timeVsFlagsButton,
			_polarizationVsFlagsButton, _baselineSpectrumPlotButton, _baselineRMSButton, _iterationsButton;
		Gtk::CheckButton _logYScaleButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			if(_antennaVsFlagsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::AntennaFlagCountPlot);
			else if(_frequencyVsFlagsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::FrequencyFlagCountPlot);
			else if(_frequencyVsPowerButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::FrequencyPowerPlot);
			else if(_timeVsFlagsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::TimeFlagCountPlot);
			else if(_baselineSpectrumPlotButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::BaselineSpectrumPlot);
			else if(_polarizationVsFlagsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::PolarizationStatisticsPlot);
			else if(_baselineRMSButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::BaselineRMSPlot);
			else if(_iterationsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::IterationsPlot);
			_action.SetLogarithmicYAxis(_logYScaleButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // STRATEGYFRAMES_PLOTFRAME_H
