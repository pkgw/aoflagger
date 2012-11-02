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
#ifndef FOREACHCOMPLEXCOMPONENTFRAME_H
#define FOREACHCOMPLEXCOMPONENTFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "../../strategy/actions/foreachcomplexcomponentaction.h"

#include "../editstrategywindow.h"

class ForEachComplexComponentFrame : public Gtk::Frame {
	public:
		ForEachComplexComponentFrame(rfiStrategy::ForEachComplexComponentAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("For each complex component"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_onAmplitudeButton("On amplitude"),
		_onPhaseButton("On phase"),
		_onRealButton("On real"),
		_onImaginaryButton("On imaginary"),
		_restoreFromAmplitudeButton("Restore from amplitude"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_onAmplitudeButton);
			_onAmplitudeButton.set_active(_action.OnAmplitude());
			_onAmplitudeButton.show();

			_box.pack_start(_onPhaseButton);
			_onPhaseButton.set_active(_action.OnPhase());
			_onPhaseButton.show();

			_box.pack_start(_onRealButton);
			_onRealButton.set_active(_action.OnReal());
			_onRealButton.show();

			_box.pack_start(_onImaginaryButton);
			_onImaginaryButton.set_active(_action.OnImaginary());
			_onImaginaryButton.show();

			_box.pack_start(_restoreFromAmplitudeButton);
			_restoreFromAmplitudeButton.set_active(_action.RestoreFromAmplitude());
			_restoreFromAmplitudeButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &ForEachComplexComponentFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::ForEachComplexComponentAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::CheckButton
			_onAmplitudeButton, _onPhaseButton, _onRealButton, _onImaginaryButton,
			_restoreFromAmplitudeButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetOnAmplitude(_onAmplitudeButton.get_active());
			_action.SetOnPhase(_onPhaseButton.get_active());
			_action.SetOnReal(_onRealButton.get_active());
			_action.SetOnImaginary(_onImaginaryButton.get_active());
			_action.SetRestoreFromAmplitude(_restoreFromAmplitudeButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // FOREACHCOMPLEXCOMPONENTFRAME_H
