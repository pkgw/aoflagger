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
#ifndef HIGHPASSFILTERFRAME_H
#define HIGHPASSFILTERFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/highpassfilteraction.h"

#include "../editstrategywindow.h"

class HighPassFilterFrame : public Gtk::Frame {
	public:
		HighPassFilterFrame(rfiStrategy::HighPassFilterAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Sliding window fit"), _editStrategyWindow(editStrategyWindow), _action(action),
		_hWindowSizeScale(0.0, 400.0, 1.0),
		_vWindowSizeScale(0.0, 400.0, 1.0),
		_hKernelSigmaScale(0.1, 1000.0, 0.1),
		_vKernelSigmaScale(0.1, 1000.0, 0.1),
		_hWindowSizeLabel("Horizontal sliding window size:", Gtk::ALIGN_START),
		_vWindowSizeLabel("Vertical sliding window size:", Gtk::ALIGN_START),
		_hKernelSigmaLabel("Horizontal kernel sigma:", Gtk::ALIGN_START),
		_vKernelSigmaLabel("Vertical kernel sigma:", Gtk::ALIGN_START),
		_modeContaminatedButton("Store result (i.e. high-pass filtered) in contaminated"),
		_modeRevisedButton("Store residual (i.e. low-pass filtered) in revised"),
		_applyButton(Gtk::Stock::APPLY)
		{
			initScales();
			
			_box.pack_start(_modeContaminatedButton);
			_box.pack_start(_modeRevisedButton);
			Gtk::RadioButtonGroup group;
			_modeContaminatedButton.set_group(group);
			_modeRevisedButton.set_group(group);
			if(_action.Mode() == rfiStrategy::HighPassFilterAction::StoreContaminated)
				_modeContaminatedButton.set_active(true);
			else
				_modeRevisedButton.set_active(true);
		
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &HighPassFilterFrame::onApplyClicked));
			_box.pack_start(_applyButton);
		
			add(_box);
			_box.show_all();
		}

	private:
		void initScales()
		{
			_box.pack_start(_hWindowSizeLabel);
			_hWindowSizeScale.set_value(_action.WindowWidth());
			_box.pack_start(_hWindowSizeScale);
		
			_box.pack_start(_vWindowSizeLabel);
			_vWindowSizeScale.set_value(_action.WindowHeight());
			_box.pack_start(_vWindowSizeScale);
		
			_box.pack_start(_hKernelSigmaLabel);
			_hKernelSigmaScale.set_value(sqrt(_action.HKernelSigmaSq()));
			_box.pack_start(_hKernelSigmaScale);
		
			_box.pack_start(_vKernelSigmaLabel);
			_vKernelSigmaScale.set_value(sqrt(_action.VKernelSigmaSq()));
			_box.pack_start(_vKernelSigmaScale);
		}

		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::HighPassFilterAction &_action;

		Gtk::VBox _box;
		Gtk::HScale
			_hWindowSizeScale, _vWindowSizeScale,
			_hKernelSigmaScale, _vKernelSigmaScale;
		Gtk::Label
			_hWindowSizeLabel, _vWindowSizeLabel,
			_hKernelSigmaLabel, _vKernelSigmaLabel;
		Gtk::RadioButton _modeContaminatedButton, _modeRevisedButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetWindowWidth((unsigned) _hWindowSizeScale.get_value());
			_action.SetWindowHeight((unsigned) _vWindowSizeScale.get_value());
			_action.SetHKernelSigmaSq(_hKernelSigmaScale.get_value()*_hKernelSigmaScale.get_value());
			_action.SetVKernelSigmaSq(_vKernelSigmaScale.get_value()*_vKernelSigmaScale.get_value());
			if(_modeContaminatedButton.get_active())
				_action.SetMode(rfiStrategy::HighPassFilterAction::StoreContaminated);
			else
				_action.SetMode(rfiStrategy::HighPassFilterAction::StoreRevised);

			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif
