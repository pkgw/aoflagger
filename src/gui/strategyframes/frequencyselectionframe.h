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
#ifndef FREQUENCYSELECTIONFRAME_H
#define FREQUENCYSELECTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/frequencyselectionaction.h"

#include "../editstrategywindow.h"

class FrequencySelectionFrame : public Gtk::Frame {
	public:
		FrequencySelectionFrame(rfiStrategy::FrequencySelectionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Frequency selection"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_thresholdLabel("Threshold:"),
		_thresholdScale(0, 10, 0.1),
		_clipDownButton("Clip values below rms as well as above"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_thresholdLabel);

			_box.pack_start(_thresholdScale);
			_thresholdScale.set_value(_action.Threshold());

			_box.pack_start(_clipDownButton);
			_clipDownButton.set_active(_action.ClipDown());
			_clipDownButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &FrequencySelectionFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::FrequencySelectionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _thresholdLabel;
		Gtk::HScale _thresholdScale;
		Gtk::CheckButton _clipDownButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetThreshold(_thresholdScale.get_value());
			_action.SetClipDown(_clipDownButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // FREQUENCYSELECTIONFRAME_H
