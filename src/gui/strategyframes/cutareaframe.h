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
#ifndef CUTAREAFRAME_H
#define CUTAREAFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "../../strategy/actions/cutareaaction.h"

#include "../editstrategywindow.h"

class CutAreaFrame : public Gtk::Frame {
	public:
		CutAreaFrame(rfiStrategy::CutAreaAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Cut area"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_startTimeLabel("Start time steps to cut:"),
		_startTimeScale(0, 64, 1),
		_endTimeLabel("End time steps to cut:"),
		_endTimeScale(0, 64, 1),
		_topChannelsLabel("Top channels to cut:"),
		_topChannelsScale(0, 64, 1),
		_bottomChannelsLabel("Bottom channels to cut:"),
		_bottomChannelsScale(0, 64, 1),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_startTimeLabel);
			_startTimeLabel.show();
			_box.pack_start(_startTimeScale);
			_startTimeScale.set_value(_action.StartTimeSteps());
			_startTimeScale.show();

			_box.pack_start(_endTimeLabel);
			_endTimeLabel.show();
			_box.pack_start(_endTimeScale);
			_endTimeScale.set_value(_action.EndTimeSteps());
			_endTimeScale.show();

			_box.pack_start(_topChannelsLabel);
			_topChannelsLabel.show();
			_box.pack_start(_topChannelsScale);
			_topChannelsScale.set_value(_action.TopChannels());
			_topChannelsScale.show();

			_box.pack_start(_bottomChannelsLabel);
			_bottomChannelsLabel.show();
			_box.pack_start(_bottomChannelsScale);
			_bottomChannelsScale.set_value(_action.BottomChannels());
			_bottomChannelsScale.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &CutAreaFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::CutAreaAction &_action;

		Gtk::Label _startTimeLabel;
		Gtk::HScale _startTimeScale;
		Gtk::Label _endTimeLabel;
		Gtk::HScale _endTimeScale;
		Gtk::Label _topChannelsLabel;
		Gtk::HScale _topChannelsScale;
		Gtk::Label _bottomChannelsLabel;
		Gtk::HScale _bottomChannelsScale;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetStartTimeSteps((int) _startTimeScale.get_value());
			_action.SetEndTimeSteps((int) _endTimeScale.get_value());
			_action.SetTopChannels((int) _topChannelsScale.get_value());
			_action.SetBottomChannels((int) _bottomChannelsScale.get_value());

			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // CUTAREAFRAME_H
