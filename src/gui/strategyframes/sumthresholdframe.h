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
#ifndef THRESHOLDFRAME_H
#define THRESHOLDFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/sumthresholdaction.h"

#include "../editstrategywindow.h"

class SumThresholdFrame : public Gtk::Frame {
	public:
		SumThresholdFrame(rfiStrategy::SumThresholdAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("SumThreshold"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_sensitivityLabel("Base sensitivity: (low = sensitive)"),
		_sensitivityScale(0, 10, 0.1),
		_timeDirectionButton("In time direction"),
		_frequencyDirectionButton("In frequency direction"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_sensitivityLabel);
			_sensitivityLabel.show();

			_box.pack_start(_sensitivityScale);
			_sensitivityScale.set_value(_action.BaseSensitivity());
			_sensitivityScale.show();
			
			_timeDirectionButton.set_active(_action.TimeDirectionFlagging());
			_buttonBox.pack_start(_timeDirectionButton);
			_timeDirectionButton.show();

			_frequencyDirectionButton.set_active(_action.FrequencyDirectionFlagging());
			_buttonBox.pack_start(_frequencyDirectionButton);
			_frequencyDirectionButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &SumThresholdFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::SumThresholdAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _sensitivityLabel;
		Gtk::HScale _sensitivityScale;
		Gtk::CheckButton _timeDirectionButton, _frequencyDirectionButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetBaseSensitivity(_sensitivityScale.get_value());
			_action.SetTimeDirectionFlagging(_timeDirectionButton.get_active());
			_action.SetFrequencyDirectionFlagging(_frequencyDirectionButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // THRESHOLDFRAME_H
