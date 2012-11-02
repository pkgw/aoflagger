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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#include "msio/measurementset.h"
#include "msio/image2d.h"
#include "msio/pngfile.h"

#include "util/integerdomain.h"

#include "uvimager.h"

using namespace std;

int main(int argc, char *argv[])
{
	cout << "Checking commandline..." << endl;

	int pindex = 1;
	bool showFlagging = false;
	bool invFlagging = false;
	while(pindex < argc && argv[pindex][0] == '-') {
		string parameter = argv[pindex]+1;
		if(parameter == "flag") { showFlagging = true; }
		else if(parameter == "invflag") { invFlagging = true; }
		else {
			cerr << "Unknown parameter: -" << parameter << endl;
			return -1;
		}
		++pindex;
	}

	string msFilename;
	if(argc-pindex < 1)
	{
		cerr << "Syntax:\n\t" << argv[0] << " [options] <ms>\n"
					"\toptions can be:\n\t-flag create uv-image of the flagging.\n"
					"\t-invflag invert flagging (when -flag specified).\n";
		return -1;
	} else
	{
		msFilename = argv[pindex];

		unsigned width = 2048, height = 2048;
		MeasurementSet ms(msFilename);
		UVImager::ImageKind kind = UVImager::Homogeneous;
		if(showFlagging)
			kind = UVImager::Flagging;
		UVImager imager(width, height, kind);
		if(invFlagging)
			imager.SetInvertFlagging(true);
		imager.Image(ms, 0, IntegerDomain(1, 1, 256));
		ColorMap *map;
		//if(showFlagging)
		//	map = new ColdHotMap();
		//else
		ColdHotMap chMap;
		map = new PosLogMap(chMap);
		PngFile::Save(imager.RealUVImage(), "real.png", *map);
		PngFile::Save(imager.ImaginaryUVImage(), "imaginary.png", *map);
		delete map;
		return 0;
	}
}
