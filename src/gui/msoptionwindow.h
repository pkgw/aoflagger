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
#ifndef MSOPTIONWINDOW_H
#define MSOPTIONWINDOW_H

#include <string>

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class MSOptionWindow : public Gtk::Window {
	public:
		MSOptionWindow(class MSWindow &msWindow, const std::string &filename);
		~MSOptionWindow();
		void onOpen();
	private:
		void initDataTypeButtons();
		void initPolarisationButtons();
		void initPartitioningButtons();

		class MSWindow &_msWindow;
		const std::string _filename;

		Gtk::HButtonBox _bottomButtonBox;
		Gtk::VBox _leftVBox;
		Gtk::HBox _topHBox;
		Gtk::Button _openButton;
		Gtk::Frame _dataKindFrame, _polarisationFrame, _partitioningFrame;
		Gtk::VBox _dataKindBox, _polarisationBox, _partitioningBox;
		Gtk::HBox _otherColumnBox;
		Gtk::RadioButton _observedDataButton, _correctedDataButton, _modelDataButton, _residualDataButton, _otherColumnButton;
		Gtk::Entry _otherColumnEntry;
		Gtk::RadioButton _allDipolePolarisationButton, _autoDipolePolarisationButton, _stokesIPolarisationButton;
		Gtk::RadioButton _noPartitioningButton, _max2500ScansButton, _max10000ScansButton, _max25000ScansButton,
			_max100000ScansButton;
		Gtk::RadioButton _directReadButton, _indirectReadButton, _memoryReadButton;
		Gtk::CheckButton _readUVWButton;
};

#endif
