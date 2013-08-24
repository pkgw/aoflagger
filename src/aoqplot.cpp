/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include "gui/quality/aoqplotwindow.h"

#include <gtkmm/main.h>
#include <gtkmm/filechooserdialog.h>

int main(int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);
	AOQPlotWindow window;
	bool wantHelp = false;
	for(int i=1;i<argc;++i)
	{
		if(argv[i][0]=='-') wantHelp = true;
	}
	if(wantHelp)
	{
		std::cout << "Syntax: aoqplot [<observation>]\n\n"
			"If your observation consists of a single observation, specify a measurement\n"
			"set. To get statistics for a (remote) observation consisting of multiple measurement\n"
			"sets, specify a measurement set specifier instead (generally a .ref, .vds\n"
			".gvds or .gds file).\n\n"
			"aoqplot is part of the AOFlagger software package, written\n"
			"by André Offringa (offringa@gmail.com).\n";
	} 
	else {
		if(argc>1)
		{
			std::vector<std::string> files;
			for(int i=1; i!=argc; ++i)
				files.push_back(argv[i]);
			window.Open(files);
		} else {
			Gtk::FileChooserDialog fileDialog(window, "Open observation set");
			
			fileDialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			fileDialog.add_button("Open", Gtk::RESPONSE_OK);
			
			Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
			filter->set_name("Observation sets (*.{vds,gds,ref,MS})");
			filter->add_pattern("*.vds");
			filter->add_pattern("*.gds");
			filter->add_pattern("*.gvds");
			filter->add_pattern("*.ref");
			filter->add_pattern("*.MS");
			fileDialog.add_filter(filter);
			
			if(fileDialog.run() == Gtk::RESPONSE_OK)
			{
				window.Open(fileDialog.get_filename());
			}
			else return 0;
		}
		kit.run();
	}
	return 0;
}
