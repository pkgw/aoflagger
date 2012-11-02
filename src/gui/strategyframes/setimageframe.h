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
#ifndef SETIMAGEFRAME_H
#define SETIMAGEFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "../../strategy/actions/setimageaction.h"

#include "../editstrategywindow.h"

class SetImageFrame : public Gtk::Frame {
	public:
		SetImageFrame(rfiStrategy::SetImageAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Set image"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_zeroButton("Zero"),
		_originalButton("Original"),
		_revisedButton("Revised"),
		_contaminatedToOriginalButton("Original=contaminated"),
		_swapButton("Swap revised and contaminated"),
		_replaceFlaggedValuesButton("Replace flagged values"),
		_setFlaggedValuesToZeroButton("Set flagged values to zero"),
		_interpolateNansButton("Interpolate NaNs"),
		_addButton("Add instead of set"),
		_applyButton(Gtk::Stock::APPLY)
		{
			Gtk::RadioButton::Group group;

			_box.pack_start(_zeroButton);
			_zeroButton.set_group(group);

			_box.pack_start(_originalButton);
			_originalButton.set_group(group);

			_box.pack_start(_revisedButton);
			_revisedButton.set_group(group);
			
			_box.pack_start(_contaminatedToOriginalButton);
			_contaminatedToOriginalButton.set_group(group);

			_box.pack_start(_swapButton);
			_swapButton.set_group(group);

			_box.pack_start(_replaceFlaggedValuesButton);
			_replaceFlaggedValuesButton.set_group(group);
			
			_box.pack_start(_setFlaggedValuesToZeroButton);
			_setFlaggedValuesToZeroButton.set_group(group);

			_box.pack_start(_interpolateNansButton);
			_interpolateNansButton.set_group(group);

			switch(_action.NewImage())
			{
				case rfiStrategy::SetImageAction::Zero:
				_zeroButton.set_active(true);
					break;
				case rfiStrategy::SetImageAction::FromOriginal:
				_originalButton.set_active(true);
					break;
				case rfiStrategy::SetImageAction::FromRevised:
				_revisedButton.set_active(true);
					break;
				case rfiStrategy::SetImageAction::ContaminatedToOriginal:
				_contaminatedToOriginalButton.set_active(true);
					break;
				case rfiStrategy::SetImageAction::SwapRevisedAndContaminated:
				_swapButton.set_active(true);
					break;
				case rfiStrategy::SetImageAction::ReplaceFlaggedValues:
				_replaceFlaggedValuesButton.set_active(true);
					break;
				case rfiStrategy::SetImageAction::SetFlaggedValuesToZero:
				_setFlaggedValuesToZeroButton.set_active(true);
					break;
				case rfiStrategy::SetImageAction::InterpolateNans:
				_interpolateNansButton.set_active(true);
					break;
			}

			_zeroButton.show();
			_originalButton.show();
			_revisedButton.show();
			_contaminatedToOriginalButton.show();
			_swapButton.show();
			_replaceFlaggedValuesButton.show();
			_setFlaggedValuesToZeroButton.show();
			_interpolateNansButton.show();

			_box.pack_start(_addButton);
			_addButton.set_active(_action.Add());
			_addButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &SetImageFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::SetImageAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _baselinesLabel;
		Gtk::RadioButton
			_zeroButton, _originalButton, _revisedButton, _contaminatedToOriginalButton, _swapButton, _replaceFlaggedValuesButton, _setFlaggedValuesToZeroButton, _interpolateNansButton;
		Gtk::CheckButton
			_addButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			if(_zeroButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::Zero);
			else if(_originalButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::FromOriginal);
			else if(_revisedButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::FromRevised);
			else if(_contaminatedToOriginalButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::ContaminatedToOriginal);
			else if(_swapButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::SwapRevisedAndContaminated);
			else if(_replaceFlaggedValuesButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::ReplaceFlaggedValues);
			else if(_setFlaggedValuesToZeroButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::SetFlaggedValuesToZero);
			else if(_interpolateNansButton.get_active())
				_action.SetNewImage(rfiStrategy::SetImageAction::InterpolateNans);
			_action.SetAdd(_addButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // SETIMAGEFRAME_H
