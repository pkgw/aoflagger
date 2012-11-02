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
#ifndef CHANGERESOLUTIONFRAME_H
#define CHANGERESOLUTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/changeresolutionaction.h"

#include "../editstrategywindow.h"

class ChangeResolutionFrame : public Gtk::Frame {
	public:
		ChangeResolutionFrame(rfiStrategy::ChangeResolutionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Change resolution"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_timeDecreaseFactorLabel("Time decrease factor:"),
		_timeDecreaseFactorScale(0, 128, 1),
		_frequencyDecreaseFactorLabel("Frequency decrease factor:"),
		_frequencyDecreaseFactorScale(0, 256, 1),
		_setRevisedToChangedImage("Set revised images to changed image"),
		_setContaminatedToChangedImage("Set contaminated images to changed image"),
		_setMasksToChangedMasks("Set masks to changed masks"),
		_useMaskForAveraging("Use mask for averaging"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_timeDecreaseFactorLabel);

			_box.pack_start(_timeDecreaseFactorScale);
			_timeDecreaseFactorScale.set_value(_action.TimeDecreaseFactor());

			_box.pack_start(_frequencyDecreaseFactorLabel);

			_box.pack_start(_frequencyDecreaseFactorScale);
			_frequencyDecreaseFactorScale.set_value(_action.FrequencyDecreaseFactor());

			_box.pack_start(_setRevisedToChangedImage);
			_setRevisedToChangedImage.set_active(_action.RestoreRevised());

			_box.pack_start(_setContaminatedToChangedImage);
			_setContaminatedToChangedImage.set_active(_action.RestoreContaminated());

			_box.pack_start(_setMasksToChangedMasks);
			_setMasksToChangedMasks.set_active(_action.RestoreMasks());

			_box.pack_start(_useMaskForAveraging);
			_useMaskForAveraging.set_active(_action.UseMaskInAveraging());
			
			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &ChangeResolutionFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::ChangeResolutionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _timeDecreaseFactorLabel;
		Gtk::HScale _timeDecreaseFactorScale;
		Gtk::Label _frequencyDecreaseFactorLabel;
		Gtk::HScale _frequencyDecreaseFactorScale;
		Gtk::CheckButton _setRevisedToChangedImage;
		Gtk::CheckButton _setContaminatedToChangedImage;
		Gtk::CheckButton _setMasksToChangedMasks;
		Gtk::CheckButton _useMaskForAveraging;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetTimeDecreaseFactor((size_t) _timeDecreaseFactorScale.get_value());
			_action.SetFrequencyDecreaseFactor((size_t) _frequencyDecreaseFactorScale.get_value());
			_action.SetRestoreRevised(_setRevisedToChangedImage.get_active());
			_action.SetRestoreContaminated(_setContaminatedToChangedImage.get_active());
			_action.SetRestoreMasks(_setMasksToChangedMasks.get_active());
			_action.SetUseMaskInAveraging(_useMaskForAveraging.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // CHANGERESOLUTIONFRAME_H
