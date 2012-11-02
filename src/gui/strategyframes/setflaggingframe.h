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
#ifndef SETFLAGGINGFRAME_H
#define SETFLAGGINGFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "../../strategy/actions/setflaggingaction.h"

#include "../editstrategywindow.h"

class SetFlaggingFrame : public Gtk::Frame {
	public:
		SetFlaggingFrame(rfiStrategy::SetFlaggingAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Set flags"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_setNoneButton("Set none"),
		_setAllButton("Set all"),
		_setFromOriginalButton("Restore original"),
		_setToOriginalButton("Change original"),
		_setInvertButton("Invert"),
		_setPolarisationsEqualButton("Equal polarisations"),
		_flagZerosButton("Flag zeros"),
		_orOriginalButton("Or with original"),
		_applyButton(Gtk::Stock::APPLY)
		{
			Gtk::RadioButton::Group group;

			_box.pack_start(_setNoneButton);
			_setNoneButton.set_group(group);

			_box.pack_start(_setAllButton);
			_setAllButton.set_group(group);

			_box.pack_start(_setFromOriginalButton);
			_setFromOriginalButton.set_group(group);

			_box.pack_start(_setToOriginalButton);
			_setToOriginalButton.set_group(group);

			_box.pack_start(_setInvertButton);
			_setInvertButton.set_group(group);
			
			_box.pack_start(_setPolarisationsEqualButton);
			_setPolarisationsEqualButton.set_group(group);

			_box.pack_start(_flagZerosButton);
			_flagZerosButton.set_group(group);

			_box.pack_start(_orOriginalButton);
			_orOriginalButton.set_group(group);

			switch(_action.NewFlagging())
			{
				case rfiStrategy::SetFlaggingAction::None:
					_setNoneButton.set_active(true);
					break;
				case rfiStrategy::SetFlaggingAction::Everything:
					_setAllButton.set_active(true);
					break;
				case rfiStrategy::SetFlaggingAction::FromOriginal:
					_setFromOriginalButton.set_active(true);
					break;
				case rfiStrategy::SetFlaggingAction::ToOriginal:
					_setToOriginalButton.set_active(true);
					break;
				case rfiStrategy::SetFlaggingAction::Invert:
					_setInvertButton.set_active(true);
					break;
				case rfiStrategy::SetFlaggingAction::PolarisationsEqual:
					_setPolarisationsEqualButton.set_active(true);
					break;
				case rfiStrategy::SetFlaggingAction::FlagZeros:
					_flagZerosButton.set_active(true);
					break;
				case rfiStrategy::SetFlaggingAction::OrOriginal:
					_orOriginalButton.set_active(true);
					break;
			}

			_setNoneButton.show();
			_setAllButton.show();
			_setFromOriginalButton.show();
			_setToOriginalButton.show();
			_setInvertButton.show();
			_setPolarisationsEqualButton.show();
			_flagZerosButton.show();
			_orOriginalButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &SetFlaggingFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::SetFlaggingAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _baselinesLabel;
		Gtk::RadioButton
			_setNoneButton, _setAllButton, _setFromOriginalButton, _setToOriginalButton, _setInvertButton, _setPolarisationsEqualButton, _flagZerosButton, _orOriginalButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			if(_setNoneButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::None);
			else if(_setAllButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::Everything);
			else if(_setFromOriginalButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::FromOriginal);
			else if(_setToOriginalButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::ToOriginal);
			else if(_setInvertButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::Invert);
			else if(_setPolarisationsEqualButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::PolarisationsEqual);
			else if(_flagZerosButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::FlagZeros);
			else if(_orOriginalButton.get_active())
				_action.SetNewFlagging(rfiStrategy::SetFlaggingAction::OrOriginal);
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // SETFLAGGINGFRAME_H
