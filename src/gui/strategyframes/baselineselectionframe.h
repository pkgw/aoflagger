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
#ifndef BASELINESELECTIONFRAME_H
#define BASELINESELECTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "../../strategy/actions/baselineselectionaction.h"

#include "../editstrategywindow.h"

class BaselineSelectionFrame : public Gtk::Frame {
	public:
		BaselineSelectionFrame(rfiStrategy::BaselineSelectionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Baseline selection"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_preparationStepButton("This is the preparation step"),
		_flagBadBaselinesButton("Flag bad baselines"),
		_makePlotButton("Make a length-rfi plot"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_preparationStepButton);
			_preparationStepButton.set_active(_action.PreparationStep());
			_preparationStepButton.show();

			_box.pack_start(_flagBadBaselinesButton);
			_flagBadBaselinesButton.set_active(_action.FlagBadBaselines());
			_flagBadBaselinesButton.show();

			_box.pack_start(_makePlotButton);
			_makePlotButton.set_active(_action.MakePlot());
			_makePlotButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselineSelectionFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::BaselineSelectionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::CheckButton _preparationStepButton, _flagBadBaselinesButton, _makePlotButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetPreparationStep(_preparationStepButton.get_active());
			_action.SetFlagBadBaselines(_flagBadBaselinesButton.get_active());
			_action.SetMakePlot(_makePlotButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // BASELINESELECTIONFRAME_H
