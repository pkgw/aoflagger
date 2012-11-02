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
#ifndef TIMECONVOLUTIONFRAME_H
#define TIMECONVOLUTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../strategy/actions/timeconvolutionaction.h"

#include "../editstrategywindow.h"

class TimeConvolutionFrame : public Gtk::Frame {
	public:
		TimeConvolutionFrame(rfiStrategy::TimeConvolutionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("UV project"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_singleSincOperationButton("Single sinc"),
		_sincOperationButton("Sinc"),
		_projectedSincOperationButton("Projected sinc"),
		_projectedFourierOperationButton("Projected FT"),
		_extrapolatedSincOperationButton("Extrapolated sinc"),
		_iterativeExtrapolatedSincOperationButton("Iterative extr. sinc"),
		_fftSincOperationButton("FFT sinc"),
		_sincSizeLabel("Sinc size: (relative to uv track diameter)"),
		_sincSizeScale(0, 10000, 1),
		_angleLabel("Angle: (degrees)"),
		_angleScale(-180, 180, 1),
		_etaLabel("Eta"),
		_etaScale(0, 1, 0.01),
		_iterationsLabel("Iterations"),
		_iterationsScale(0, 100, 1),
		_channelAveragingSizeLabel("Channel averaging size"),
		_channelAveragingSizeScale(0, 512, 1),
		_sincScaleInSamplesButton("Sinc scale is in time steps"),
		_autoAngleButton("Auto angle"),
		_alwaysRemoveButton("Always remove"),
		_hammingButton("Use hamming window"),
		_applyButton(Gtk::Stock::APPLY)
		{
			Gtk::RadioButton::Group group;

			_box.pack_start(_singleSincOperationButton);
			_singleSincOperationButton.set_group(group);
			_singleSincOperationButton.show();
			
			_box.pack_start(_sincOperationButton);
			_sincOperationButton.set_group(group);
			_sincOperationButton.show();
			
			_box.pack_start(_projectedSincOperationButton);
			_projectedSincOperationButton.set_group(group);
			_projectedSincOperationButton.show();

			_box.pack_start(_projectedFourierOperationButton);
			_projectedFourierOperationButton.set_group(group);
			_projectedFourierOperationButton.show();

			_box.pack_start(_extrapolatedSincOperationButton);
			_extrapolatedSincOperationButton.set_group(group);
			_extrapolatedSincOperationButton.show();

			_box.pack_start(_iterativeExtrapolatedSincOperationButton);
			_iterativeExtrapolatedSincOperationButton.set_group(group);
			_iterativeExtrapolatedSincOperationButton.show();
			
			_box.pack_start(_fftSincOperationButton);
			_fftSincOperationButton.set_group(group);
			_fftSincOperationButton.show();

			switch(action.Operation())
			{
				case rfiStrategy::TimeConvolutionAction::SingleSincOperation:
					_singleSincOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::SincOperation:
					_sincOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::ProjectedSincOperation:
					_projectedSincOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::ProjectedFTOperation:
					_projectedFourierOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::ExtrapolatedSincOperation:
					_extrapolatedSincOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::IterativeExtrapolatedSincOperation:
					_iterativeExtrapolatedSincOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::FFTSincOperation:
					_fftSincOperationButton.set_active(true);
					break;
			}

			_box.pack_start(_sincSizeLabel);

			_box.pack_start(_sincSizeScale);
			_sincSizeScale.set_value(action.SincScale());
			
			_box.pack_start(_angleLabel);

			_box.pack_start(_angleScale);
			_angleScale.set_value(action.DirectionRad()*180.0/M_PI);
			
			_box.pack_start(_etaLabel);

			_box.pack_start(_etaScale);
			_etaScale.set_value(action.EtaParameter());
			
			_box.pack_start(_iterationsLabel);

			_box.pack_start(_iterationsScale);
			_iterationsScale.set_value(action.Iterations());
			
			_box.pack_start(_channelAveragingSizeLabel);

			_box.pack_start(_channelAveragingSizeScale);
			_channelAveragingSizeScale.set_value(action.ChannelAveragingSize());
			
			_box.pack_start(_sincScaleInSamplesButton);
			_sincScaleInSamplesButton.set_active(action.IsSincScaleInSamples());

			_box.pack_start(_autoAngleButton);
			_autoAngleButton.set_active(action.AutoAngle());

			_box.pack_start(_alwaysRemoveButton);
			_alwaysRemoveButton.set_active(action.AlwaysRemove());
			
			_box.pack_start(_hammingButton);
			_hammingButton.set_active(action.UseHammingWindow());

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &TimeConvolutionFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::TimeConvolutionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::RadioButton _singleSincOperationButton, _sincOperationButton, _projectedSincOperationButton, _projectedFourierOperationButton, _extrapolatedSincOperationButton, _iterativeExtrapolatedSincOperationButton, _fftSincOperationButton;
		Gtk::Label _sincSizeLabel;
		Gtk::HScale _sincSizeScale;
		Gtk::Label _angleLabel;
		Gtk::HScale _angleScale;
		Gtk::Label _etaLabel;
		Gtk::HScale _etaScale;
		Gtk::Label _iterationsLabel;
		Gtk::HScale _iterationsScale;
		Gtk::Label _channelAveragingSizeLabel;
		Gtk::HScale _channelAveragingSizeScale;
		Gtk::CheckButton _sincScaleInSamplesButton, _autoAngleButton, _alwaysRemoveButton, _hammingButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetDirectionRad((num_t) _angleScale.get_value()/180.0*M_PI);
			_action.SetSincScale(_sincSizeScale.get_value());
			_action.SetEtaParameter(_etaScale.get_value());
			_action.SetIterations((unsigned) _iterationsScale.get_value());
			_action.SetIsSincScaleInSamples(_sincScaleInSamplesButton.get_active());
			_action.SetAutoAngle(_autoAngleButton.get_active());
			_action.SetAlwaysRemove(_alwaysRemoveButton.get_active());
			_action.SetChannelAveragingSize((unsigned) _channelAveragingSizeScale.get_value());
			_action.SetUseHammingWindow(_hammingButton.get_active());
			if(_singleSincOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::SingleSincOperation);
			else if(_sincOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::SincOperation);
			else if(_projectedSincOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::ProjectedSincOperation);
			else if(_projectedFourierOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::ProjectedFTOperation);
			else if(_extrapolatedSincOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::ExtrapolatedSincOperation);
			else if(_iterativeExtrapolatedSincOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::IterativeExtrapolatedSincOperation);
			else // if(_fftSincOperationButton
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::FFTSincOperation);
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // TIMECONVOLUTIONFRAME_H
