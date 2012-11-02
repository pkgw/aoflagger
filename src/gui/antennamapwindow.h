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
#ifndef ANTENNAMAPWINDOW_H
#define ANTENNAMAPWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/window.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>

#include "antennamap/antennamapwidget.h"

#include "numinputdialog.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AntennaMapWindow : public Gtk::Window {
	public:
		AntennaMapWindow() : _antennaMapWidget(), _set(0)
		{
			set_default_size(500, 400);

			add(_vBox);
			
			Gtk::Menu::MenuList& menulist = _menuFile.items();
			
			menulist.push_back( Gtk::Menu_Helpers::MenuElem("Open _antennas in ms", sigc::mem_fun(*this, &AntennaMapWindow::onOpenAntennasInMS) ) );
			menulist.push_back( Gtk::Menu_Helpers::MenuElem("Open _antenna file", sigc::mem_fun(*this, &AntennaMapWindow::onOpenAntennaFile) ) );
			menulist.push_back( Gtk::Menu_Helpers::MenuElem("Open antenna _time file", sigc::mem_fun(*this, &AntennaMapWindow::onOpenAntennaTimeFile) ) );
			menulist.push_back( Gtk::Menu_Helpers::MenuElem("Open _spectrum file", sigc::mem_fun(*this, &AntennaMapWindow::onOpenSpectrumFile) ) );
			
			_vBox.pack_start(_menuBar, Gtk::PACK_SHRINK);
			_menuBar.items().push_back(Gtk::Menu_Helpers::MenuElem("_File", _menuFile));
			
			_vBox.pack_end(_antennaMapWidget);

			_vBox.show_all();

		  Glib::signal_timeout().connect(sigc::mem_fun(*this, &AntennaMapWindow::onTimeout), 1000/25);
		}
		
		~AntennaMapWindow()
		{
			if(_set != 0)
				delete _set;
		}
		
		void SetMeasurementSet(class MeasurementSet &set)
		{
			_antennaMapWidget.SetMeasurementSet(set);
		}
		
	private:
		antennaMap::AntennaMapWidget _antennaMapWidget;
		Gtk::MenuBar _menuBar;
		Gtk::VBox _vBox;
		MeasurementSet *_set;

		Gtk::Menu _menuFile;
		
		bool chooseFile(std::string &file, const std::string &windowTitle, bool folder=false)
		{
			Gtk::FileChooserDialog *dialog;
			if(folder)
				dialog = new Gtk::FileChooserDialog(windowTitle, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
			else
				dialog = new Gtk::FileChooserDialog(windowTitle);
			dialog->set_transient_for(*this);

			//Add response buttons the the dialog:
			dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			dialog->add_button("Open", Gtk::RESPONSE_OK);

			int result = dialog->run();

			if(result == Gtk::RESPONSE_OK)
			{
				file = dialog->get_filename();
				delete dialog;
				return true;
			} else {
				delete dialog;
				return false;
			}
		}
		
		void onOpenAntennasInMS()
		{
			std::string file;
			if(chooseFile(file, "Select antenna measurement set", true))
			{
				if(_set != 0)
					delete _set;
				_set = new MeasurementSet(file);
				_antennaMapWidget.SetMeasurementSet(*_set);
			}
			_antennaMapWidget.Update();
		}
		
		void onOpenAntennaFile()
		{
			std::string file;
			if(chooseFile(file, "Select an antenna time file"))
			{
				_antennaMapWidget.SetValuesFromAntennaFile(file);
			}
		}
		
		void onOpenAntennaTimeFile()
		{
			std::string file;
			if(chooseFile(file, "Select an antenna time file"))
			{
				NumInputDialog numInputDialog("Movie speed", "Time steps per frame:", 60);
				int result = numInputDialog.run();
				if(result == Gtk::RESPONSE_OK)
				{
					_antennaMapWidget.SetMovieTimeStepsPerFrame(numInputDialog.Value());
					_antennaMapWidget.Map().OpenStatFile(file);
				}
			}
		}
		
		void onOpenSpectrumFile()
		{
			std::string file;
			if(chooseFile(file, "Select a spectrum file"))
			{
				_antennaMapWidget.SetValuesFromSpectrumFile(file);
			}
		}
		
		bool onTimeout()
		{
			_antennaMapWidget.Update();
			return true;
		}
};
#endif
