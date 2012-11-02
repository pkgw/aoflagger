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
#ifndef RESAMPLINGFRAME_H
#define RESAMPLINGFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/resamplingaction.h"

#include "../editstrategywindow.h"

class ResamplingFrame : public Gtk::Frame {
	public:
		ResamplingFrame(rfiStrategy::ResamplingAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Resampling"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_averagingButton("Average"),
		_nnButton("Nearest neighbour"),
		_sizeXLabel("Size x:"),
		_sizeYLabel("Size y"),
		_sizeXScale(1, 1024, 1),
		_sizeYScale(1, 1024, 1),
		_applyButton(Gtk::Stock::APPLY)
		{
			Gtk::RadioButton::Group group;
			_averagingButton.set_group(group);
			_box.pack_start(_averagingButton);
			
			_nnButton.set_group(group);
			_box.pack_start(_nnButton);
			if(_action.Operation() == rfiStrategy::ResamplingAction::Average)
				_averagingButton.set_active(true);
			else
				_nnButton.set_active(true);
			
			_box.pack_start(_sizeXLabel);

			_box.pack_start(_sizeXScale);
			_sizeXScale.set_value(_action.SizeX());

			_box.pack_start(_sizeYLabel);

			_box.pack_start(_sizeYScale);
			_sizeYScale.set_value(_action.SizeY());

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &ResamplingFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::ResamplingAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::RadioButton _averagingButton, _nnButton;
		Gtk::Label _sizeXLabel, _sizeYLabel;
		Gtk::HScale _sizeXScale, _sizeYScale;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			if(_averagingButton.get_active())
				_action.SetOperation(rfiStrategy::ResamplingAction::Average);
			else
				_action.SetOperation(rfiStrategy::ResamplingAction::NearestNeighbour);
			_action.SetSizeX((unsigned) _sizeXScale.get_value());
			_action.SetSizeY((unsigned)_sizeYScale.get_value());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // RESAMPLINGFRAME_H
