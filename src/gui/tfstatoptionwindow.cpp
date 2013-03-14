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

#include <gtkmm/messagedialog.h>

#include "../strategy/imagesets/timefrequencystatimageset.h"

#include "rfiguiwindow.h"
#include "tfstatoptionwindow.h"

TFStatOptionWindow::TFStatOptionWindow(RFIGuiWindow &rfiGuiWindow, const std::string &filename) :
	Gtk::Window(),
	_rfiGuiWindow(rfiGuiWindow),
	_filename(filename),
	_openButton("Open"),
	_modeFrame("What to read"),
	_rfiPercentangeButton("RFI percentages"),
	_totalAmplitudeButton("Total amplitudes"),
	_rfiAmplitudeButton("RFI amplitudes"),
	_nonRfiAmplitudeButton("Non-RFI amplitudes")
{
	set_title("Options for opening a tf stat file");

	initModeButtons();
	
	_openButton.signal_clicked().connect(sigc::mem_fun(*this, &TFStatOptionWindow::onOpen));
	_bottomButtonBox.pack_start(_openButton);
	_openButton.show();

	_topVBox.pack_start(_bottomButtonBox);
	_bottomButtonBox.show();

	add(_topVBox);
	_topVBox.show();
}

void TFStatOptionWindow::initModeButtons()
{
	Gtk::RadioButton::Group group;

	_rfiPercentangeButton.set_group(group);
	_modeBox.pack_start(_rfiPercentangeButton);
	_rfiPercentangeButton.set_active(true);
	_rfiPercentangeButton.show();
	
	_totalAmplitudeButton.set_group(group);
	_modeBox.pack_start(_totalAmplitudeButton);
	_totalAmplitudeButton.show();
	
	_rfiAmplitudeButton.set_group(group);
	_modeBox.pack_start(_rfiAmplitudeButton);
	_rfiAmplitudeButton.show();
	
	_nonRfiAmplitudeButton.set_group(group);
	_modeBox.pack_start(_nonRfiAmplitudeButton);
	_nonRfiAmplitudeButton.show();

	_modeFrame.add(_modeBox);
	_modeBox.show();

	_topVBox.pack_start(_modeFrame);
	_modeFrame.show();
}

void TFStatOptionWindow::onOpen()
{
	std::cout << "Opening " << _filename << std::endl;
	try
	{
		rfiStrategy::TimeFrequencyStatImageSet *imageSet =
			new rfiStrategy::TimeFrequencyStatImageSet(_filename);
		if(_rfiPercentangeButton.get_active())
			imageSet->SetMode(rfiStrategy::TimeFrequencyStatImageSet::RFIPercentages);
		else if(_totalAmplitudeButton.get_active())
			imageSet->SetMode(rfiStrategy::TimeFrequencyStatImageSet::TotalAmplitude);
		else if(_rfiAmplitudeButton.get_active())
			imageSet->SetMode(rfiStrategy::TimeFrequencyStatImageSet::RFIAmplitude);
		else if(_nonRfiAmplitudeButton.get_active())
			imageSet->SetMode(rfiStrategy::TimeFrequencyStatImageSet::NonRFIAmplitude);
		imageSet->Initialize();
	
		_rfiGuiWindow.SetImageSet(imageSet);
	} catch(std::exception &e)
	{
		Gtk::MessageDialog dialog(*this, e.what(), false, Gtk::MESSAGE_ERROR);
		dialog.run();
	}
	hide();
}

