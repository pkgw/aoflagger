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
#ifndef ITERATIONFRAME_H
#define ITERATIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/iterationaction.h"

#include "../editstrategywindow.h"

class IterationFrame : public Gtk::Frame {
	public:
		IterationFrame(rfiStrategy::IterationBlock &iterationBlock, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Iteration"),
		_editStrategyWindow(editStrategyWindow), _iterationBlock(iterationBlock),
		_iterationCountLabel("Iteration count:"),
		_sensitivityStartLabel("Sensitivity start value (moves to 1):"),
		_iterationCountScale(0, 1000, 1),
		_sensitivityStartScale(0, 25.0, 0.25),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_iterationCountLabel);

			_box.pack_start(_iterationCountScale);
			_iterationCountScale.set_value(_iterationBlock.IterationCount());

			_box.pack_start(_sensitivityStartLabel);

			_box.pack_start(_sensitivityStartScale);
			_sensitivityStartScale.set_value(_iterationBlock.SensitivityStart());

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &IterationFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::IterationBlock &_iterationBlock;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _iterationCountLabel, _sensitivityStartLabel;
		Gtk::HScale _iterationCountScale, _sensitivityStartScale;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_iterationBlock.SetIterationCount((size_t) _iterationCountScale.get_value());
			_iterationBlock.SetSensitivityStart(_sensitivityStartScale.get_value());
			_editStrategyWindow.UpdateAction(&_iterationBlock);

		}
};

#endif // ITERATIONFRAME_H
