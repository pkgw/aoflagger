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

#include <gtkmm.h>

int main(int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);
	AOQPlotWindow window;
	if(argc>1)
	{
		window.Open(argv[1]);
		kit.run();
	} else {
		std::cout << "Syntax: aoqplot <observation>\n\n"
			"If your observation consists of a single observation, specify a measurement\n"
			"set. To get statistics for a (remote) observation consisting of multiple measurement\n"
			"sets, specify a measurement set specifier instead (generally a .ref, .vds\n"
			".gvds or .gds file).\n\n"
			"aoqplot is part of the AOFlagger software package, written\n"
			"by Andre Offringa (offringa@gmail.com).\n";
	}
	return 0;
}
