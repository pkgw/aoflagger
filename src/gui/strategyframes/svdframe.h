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
#ifndef SVDFRAME_H
#define SVDFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/svdaction.h"

#include "../editstrategywindow.h"

class SVDFrame : public Gtk::Frame {
	public:
		SVDFrame(rfiStrategy::SVDAction &svdAction, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Singular value decomposition"),
		_editStrategyWindow(editStrategyWindow), _svdAction(svdAction),
		_singularValueCountLabel("Singular value count:"),
		_singularValueCountScale(0, 100, 1),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_singularValueCountLabel);
			_singularValueCountLabel.show();

			_box.pack_start(_singularValueCountScale);
			_singularValueCountScale.set_value(_svdAction.SingularValueCount());
			_singularValueCountScale.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &SVDFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::SVDAction &_svdAction;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _singularValueCountLabel;
		Gtk::HScale _singularValueCountScale;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_svdAction.SetSingularValueCount((size_t) _singularValueCountScale.get_value());
			_editStrategyWindow.UpdateAction(&_svdAction);
		}
};

#endif // SVDFRAME_H
