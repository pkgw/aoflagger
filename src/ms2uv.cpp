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

//#include <iostream>

//#include "timefrequencyimager.h"
//#include "methodtester.h"

#include <iostream>
#include <cstdlib>
#include <sstream>

#include "msio/measurementset.h"
#include "msio/image2d.h"
#include "msio/pngfile.h"
#include "msio/timefrequencyimager.h"

#include "rfi/localfitmethod.h"
#include "rfi/methoditerator.h"
#include "rfi/mitigationtester.h"
#include "rfi/thresholdtools.h"
#include "rfi/tiledimage.h"

#include "types.h"

#include "rfi/svdmitigater.h" //needs to be last

using namespace std;

int main(int argc, char *argv[])
{
	cout << "Checking commandline..." << endl;

	int pindex = 1;
	bool showFlagging = true, winsorizedStretch = false, useColor = true;
	while(pindex < argc && argv[pindex][0] == '-') {
		string parameter = argv[pindex]+1;
		if(parameter == "nf") { showFlagging = false; }
		else if(parameter == "wins") { winsorizedStretch = true; }
		else if(parameter == "bw") { useColor = false; }
		else {
			cerr << "Unknown parameter: -" << parameter << endl;
			return -1;
		}
		++pindex;
	}


	string msFilename, outputFile;
	unsigned antenna1, antenna2, band, pol;
	if(argc-pindex < 6)
	{
		cerr << "Syntax:\n\t" << argv[0] << " [options] <ms> <antenna1> <antenna2> <band> <polarisationIndex> <outputfile>\n"
					"\toptions can be:\n\t-nf don't show flagging.\n\t-wins stretch the contrast with a Winsorized approach\n\t-bw do not use colors for representing the data\n";
		return -1;
	} else
	{
		msFilename = argv[pindex];
		antenna1 = atoi(argv[pindex+1]);
		antenna2 = atoi(argv[pindex+2]);
		band = atoi(argv[pindex+3]);
		pol = atoi(argv[pindex+4]);
		outputFile = argv[pindex+5];

		MeasurementSet set(msFilename);
		TimeFrequencyImager imager(set);
		imager.SetReadFlags(true);
		cout << "Reading measurement set (a1=" << antenna1 << ", a2=" << antenna2 << ", band=" << band << ")..." << endl;
		imager.Image(antenna1, antenna2, band);
		Image2DCPtr image;
		Mask2DCPtr flag;
		cout << "Saving png file for polarisation " << pol << "..." << endl;
		switch(pol) {
			case 0: image = imager.RealXX(); flag = imager.FlagXX(); break; 
			case 1: image = imager.RealXY(); flag = imager.FlagXY(); break; 
			case 2: image = imager.RealYX(); flag = imager.FlagYX(); break; 
			case 3: image = imager.RealYY(); flag = imager.FlagYY(); break; 
		}
		long double mean, stddev;
		if(!showFlagging) {
			flag = Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
 		}
		ThresholdTools::MeanAndStdDev(image, flag, mean, stddev);
		cout << "Mean: " << mean << ", stddev: " << stddev << endl;
		ThresholdTools::WinsorizedMeanAndStdDev(image, flag, mean, stddev);
		cout << "W-Mean: " << mean << ", w-stddev: " << stddev << endl;
		long double max = ThresholdTools::MaxValue(image, flag);
		long double min = ThresholdTools::MinValue(image, flag);
		cout << "Min: " << min << ", max: " << max << endl;
		unsigned count = flag->GetCount<true>();
		cout << "Flagged count: " << count << " (" <<
            round(1000.0 * count / (flag->Width()*flag->Height())) / 10.0 << "%)" << endl;
		
		MethodIterator::SaveFlaggingToPng(image, flag, outputFile, &imager.BandInfo(), showFlagging, winsorizedStretch, useColor);
		return 0;
	}
}
