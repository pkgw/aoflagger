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

#include <iostream>

#include <gtkmm/messagedialog.h>

#include "../msio/timefrequencyimager.h"

#include "../strategy/imagesets/msimageset.h"
#include "../strategy/imagesets/rspimageset.h"

#include "rfiguiwindow.h"
#include "rawoptionwindow.h"

RawOptionWindow::RawOptionWindow(RFIGuiWindow &rfiGuiWindow, const std::string &filename) :
	Gtk::Window(),
	_rfiGuiWindow(rfiGuiWindow),
	_filename(filename),
	_openButton("Open"),
	_modeFrame("Columns to read"),
	_allBeamletsButton("All beamlets"), _singleBeamletButton("Single beamlet"), _channelBeamletButton("Single beamlet with channels"),
	_statisticsButton("Read whole set for statistics"),
	_beamletsInSetLabel("Beamlets in raw file:"), _timeBlockSizeLabel("Time block size for reading 128*2^n; n="),
	_beamletsInSetScale(1, 62, 1), _timeBlockSizeScale(1, 10, 1)
{
	set_title("Options for opening a raw file");

	initModeButtons();
	
	_topVBox.pack_start(_beamletsInSetLabel);
	_beamletsInSetLabel.show();
	
	_beamletsInSetScale.set_value(5);
	_topVBox.pack_start(_beamletsInSetScale);
	_beamletsInSetScale.show();

	_topVBox.pack_start(_timeBlockSizeLabel);
	_timeBlockSizeLabel.show();
	
	_timeBlockSizeScale.set_value(4);
	_topVBox.pack_start(_timeBlockSizeScale);
	_timeBlockSizeScale.show();

	_openButton.signal_clicked().connect(sigc::mem_fun(*this, &RawOptionWindow::onOpen));
	_bottomButtonBox.pack_start(_openButton);
	_openButton.show();

	_topVBox.pack_start(_bottomButtonBox);
	_bottomButtonBox.show();

	add(_topVBox);
	_topVBox.show();
}

void RawOptionWindow::initModeButtons()
{
	Gtk::RadioButton::Group group;

	_allBeamletsButton.set_group(group);
	_modeBox.pack_start(_allBeamletsButton);
	_allBeamletsButton.show();
	
	_singleBeamletButton.set_group(group);
	_modeBox.pack_start(_singleBeamletButton);
	_singleBeamletButton.show();
	
	_channelBeamletButton.set_group(group);
	_modeBox.pack_start(_channelBeamletButton);
	_channelBeamletButton.set_active(true);
	_channelBeamletButton.show();
	
	_statisticsButton.set_group(group);
	_modeBox.pack_start(_statisticsButton);
	_statisticsButton.show();

	_modeFrame.add(_modeBox);
	_modeBox.show();

	_topVBox.pack_start(_modeFrame);
	_modeFrame.show();
}

void RawOptionWindow::onOpen()
{
	std::cout << "Opening " << _filename << std::endl;
	try
	{
		rfiStrategy::RSPImageSet *imageSet = new rfiStrategy::RSPImageSet(_filename);
		imageSet->SetBeamletsInSet((unsigned int) _beamletsInSetScale.get_value());
		imageSet->SetTimeBlockSize(128ul << (unsigned long) _timeBlockSizeScale.get_value());
		if(_allBeamletsButton.get_active())
			imageSet->SetMode(rfiStrategy::RSPImageSet::AllBeamletsMode);
		else if(_singleBeamletButton.get_active())
			imageSet->SetMode(rfiStrategy::RSPImageSet::SingleBeamletMode);
		else if(_channelBeamletButton.get_active())
			imageSet->SetMode(rfiStrategy::RSPImageSet::BeamletChannelMode);
		else
			imageSet->SetMode(rfiStrategy::RSPImageSet::StatisticsMode);
		imageSet->Initialize();
	
		_rfiGuiWindow.SetImageSet(imageSet);
	} catch(std::exception &e)
	{
		Gtk::MessageDialog dialog(*this, e.what(), false, Gtk::MESSAGE_ERROR);
		dialog.run();
	}
	hide();
}

