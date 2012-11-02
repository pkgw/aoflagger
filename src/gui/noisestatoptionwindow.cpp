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

#include "../strategy/imagesets/noisestatimageset.h"

#include "mswindow.h"
#include "noisestatoptionwindow.h"

NoiseStatOptionWindow::NoiseStatOptionWindow(MSWindow &msWindow, const std::string &filename) :
	Gtk::Window(),
	_msWindow(msWindow),
	_filename(filename),
	_openButton("Open"),
	_modeFrame("What to read"),
	_meanButton("Mean"),
	_stdDevButton("Stddev"),
	_varianceButton("Variance"),
	_varianceOfVarianceButton("Variance of variance")
{
	set_title("Options for opening a noise stat file");

	initModeButtons();
	
	_openButton.signal_clicked().connect(sigc::mem_fun(*this, &NoiseStatOptionWindow::onOpen));
	_bottomButtonBox.pack_start(_openButton);

	_topVBox.pack_start(_bottomButtonBox);

	add(_topVBox);
	_topVBox.show_all();
}

void NoiseStatOptionWindow::initModeButtons()
{
	Gtk::RadioButton::Group group;

	_meanButton.set_group(group);
	_modeBox.pack_start(_meanButton);
	
	_stdDevButton.set_group(group);
	_modeBox.pack_start(_stdDevButton);
	_stdDevButton.set_active(true);
	
	_varianceButton.set_group(group);
	_modeBox.pack_start(_varianceButton);
	
	_varianceOfVarianceButton.set_group(group);
	_modeBox.pack_start(_varianceOfVarianceButton);
	
	_modeFrame.add(_modeBox);

	_topVBox.pack_start(_modeFrame);
}

void NoiseStatOptionWindow::onOpen()
{
	std::cout << "Opening " << _filename << std::endl;
	try
	{
		rfiStrategy::NoiseStatImageSet *imageSet =
			new rfiStrategy::NoiseStatImageSet(_filename);
		if(_meanButton.get_active())
			imageSet->SetMode(rfiStrategy::NoiseStatImageSet::Mean);
		else if(_stdDevButton.get_active())
			imageSet->SetMode(rfiStrategy::NoiseStatImageSet::StdDev);
		else if(_varianceButton.get_active())
			imageSet->SetMode(rfiStrategy::NoiseStatImageSet::Variance);
		else if(_varianceOfVarianceButton.get_active())
			imageSet->SetMode(rfiStrategy::NoiseStatImageSet::VarianceOfVariance);
		imageSet->Initialize();
	
		_msWindow.SetImageSet(imageSet);
	} catch(std::exception &e)
	{
		Gtk::MessageDialog dialog(*this, e.what(), false, Gtk::MESSAGE_ERROR);
		dialog.run();
	}
	hide();
}

