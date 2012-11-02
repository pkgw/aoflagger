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
#ifndef ABSTHRESHOLDFRAME_H
#define ABSTHRESHOLDFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/absthresholdaction.h"

#include "../editstrategywindow.h"

class AbsThresholdFrame : public Gtk::Frame {
	public:
		AbsThresholdFrame(rfiStrategy::AbsThresholdAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Absolute threshold"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_thresholdLabel("Threshold:"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_thresholdLabel);

			_box.pack_start(_thresholdEntry);
			std::stringstream s;
			s << _action.Threshold();
			_thresholdEntry.set_text(s.str());

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &AbsThresholdFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::AbsThresholdAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _thresholdLabel;
		Gtk::Entry _thresholdEntry;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetThreshold(atof(_thresholdEntry.get_text().c_str()));
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // ABSTHRESHOLDFRAME_H
