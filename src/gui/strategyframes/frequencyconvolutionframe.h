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
#ifndef FREQUENCYCONVOLUTIONFRAME_H
#define FREQUENCYCONVOLUTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/frequencyconvolutionaction.h"

#include "../editstrategywindow.h"

class FrequencyConvolutionFrame : public Gtk::Frame {
	public:
		FrequencyConvolutionFrame(rfiStrategy::FrequencyConvolutionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Frequency convolution"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_rectangularKernelButton("Rectangular kernel"),
		_sincKernelButton("Sinc kernel"),
		_totalKernelButton("Total kernel"),
		_convolutionSizeLabel("Convolution size:"),
		_convolutionSizeScale(1, 1024, 1),
		_inSamplesButton("Size in samples"),
		_applyButton(Gtk::Stock::APPLY)
		{
			Gtk::RadioButton::Group kernelGroup;
		
			_rectangularKernelButton.set_group(kernelGroup);
			_box.pack_start(_rectangularKernelButton);
			_sincKernelButton.set_group(kernelGroup);
			_box.pack_start(_sincKernelButton);
			_totalKernelButton.set_group(kernelGroup);
			_box.pack_start(_totalKernelButton);
			
			if(_action.KernelKind() == rfiStrategy::FrequencyConvolutionAction::RectangleKernel)
				_rectangularKernelButton.set_active(true);
			else if(_action.KernelKind() == rfiStrategy::FrequencyConvolutionAction::SincKernel)
				_sincKernelButton.set_active(true);
			else
				_totalKernelButton.set_active(true);

			_box.pack_start(_convolutionSizeLabel);
			
			_box.pack_start(_convolutionSizeScale);
			_convolutionSizeScale.set_value(_action.ConvolutionSize());

			_box.pack_start(_inSamplesButton);
			_inSamplesButton.set_active(_action.InSamples());

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &FrequencyConvolutionFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::FrequencyConvolutionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::RadioButton _rectangularKernelButton, _sincKernelButton, _totalKernelButton;
		Gtk::Label _convolutionSizeLabel;
		Gtk::HScale _convolutionSizeScale;
		Gtk::CheckButton _inSamplesButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			if(_rectangularKernelButton.get_active())
				_action.SetKernelKind(rfiStrategy::FrequencyConvolutionAction::RectangleKernel);
			else if(_sincKernelButton.get_active())
				_action.SetKernelKind(rfiStrategy::FrequencyConvolutionAction::SincKernel);
			else if(_totalKernelButton.get_active())
				_action.SetKernelKind(rfiStrategy::FrequencyConvolutionAction::TotalKernel);
			_action.SetConvolutionSize(_convolutionSizeScale.get_value());
			_action.SetInSamples(_inSamplesButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // FREQUENCYCONVOLUTIONFRAME_H
