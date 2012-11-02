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
#ifndef SPATIALCOMPOSITIONFRAME_H
#define SPATIALCOMPOSITIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "../../strategy/actions/spatialcompositionaction.h"

#include "../editstrategywindow.h"

class SpatialCompositionFrame : public Gtk::Frame {
	public:
		SpatialCompositionFrame(rfiStrategy::SpatialCompositionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Set image"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_sumCrossButton("Sum cross correlations"),
		_sumAutoButton("Sum auto correlations"),
		_eigenvalueButton("First eigenvalues of complete matrix"),
		_removeEigenvalueButton("Remove first eigenvalues"),
		_applyButton(Gtk::Stock::APPLY)
		{
			Gtk::RadioButton::Group group;

			_box.pack_start(_sumCrossButton);
			_sumCrossButton.set_group(group);

			_box.pack_start(_sumAutoButton);
			_sumAutoButton.set_group(group);

			_box.pack_start(_eigenvalueButton);
			_eigenvalueButton.set_group(group);
			
			_box.pack_start(_removeEigenvalueButton);
			_removeEigenvalueButton.set_group(group);
			
			switch(_action.Operation())
			{
				case rfiStrategy::SpatialCompositionAction::SumCrossCorrelationsOperation:
				_sumCrossButton.set_active(true);
					break;
				case rfiStrategy::SpatialCompositionAction::SumAutoCorrelationsOperation:
				_sumAutoButton.set_active(true);
					break;
				case rfiStrategy::SpatialCompositionAction::EigenvalueDecompositionOperation:
				_eigenvalueButton.set_active(true);
					break;
				case rfiStrategy::SpatialCompositionAction::EigenvalueRemovalOperation:
				_removeEigenvalueButton.set_active(true);
					break;
			}

			_sumCrossButton.show();
			_sumAutoButton.show();
			_eigenvalueButton.show();
			_removeEigenvalueButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &SpatialCompositionFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::SpatialCompositionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _baselinesLabel;
		Gtk::RadioButton
			_sumCrossButton, _sumAutoButton, _eigenvalueButton, _removeEigenvalueButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			if(_sumCrossButton.get_active())
				_action.SetOperation(rfiStrategy::SpatialCompositionAction::SumCrossCorrelationsOperation);
			else if(_sumAutoButton.get_active())
				_action.SetOperation(rfiStrategy::SpatialCompositionAction::SumAutoCorrelationsOperation);
			else if(_eigenvalueButton.get_active())
				_action.SetOperation(rfiStrategy::SpatialCompositionAction::EigenvalueDecompositionOperation);
			else if(_removeEigenvalueButton.get_active())
				_action.SetOperation(rfiStrategy::SpatialCompositionAction::EigenvalueRemovalOperation);
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // SPATIALCOMPOSITIONFRAME_H
