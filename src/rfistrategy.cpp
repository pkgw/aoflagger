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
#include <libgen.h>

#include "strategy/actions/strategyaction.h"

#include "strategy/control/strategyreader.h"
#include "strategy/control/strategywriter.h"

#include "util/aologger.h"
#include "util/parameter.h"

using namespace rfiStrategy;
using namespace std;

int main(int argc, char *argv[])
{
	AOLogger::Init(basename(argv[0]), false);

	cout << 
			"RFI strategy file writer\n"
			"This program will write an RFI strategy to a file, to run it with the\n"
			"rficonsole or the rfigui.\n\n"
			"Author: André Offringa (offringa@astro.rug.nl)\n"
			<< endl;

	Parameter<enum BaselineSelection> baselineSelection;
	Parameter<std::string> dataColumn;
	Parameter<bool> frequencyBasedFlagging;
	Parameter<bool> flagStokes;
	Parameter<size_t> threadCount;
	Parameter<pair<double, double> > kernelSize;
	Parameter<enum PolarisationType> polarisation;
	Parameter<num_t> sensitivity;
	Parameter<pair<size_t, size_t> > windowSize;

	size_t parameterIndex = 1;
	while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
	{
		string flag(argv[parameterIndex]+1);

		if(flag == "b" || flag == "baseline")
		{
			++parameterIndex;
			string baselineStr(argv[parameterIndex]); 
			if(baselineStr == "all") baselineSelection = All;
			else if(baselineStr == "auto") baselineSelection = AutoCorrelations;
			else if(baselineStr == "cross") baselineSelection = CrossCorrelations;
			else throw runtime_error("Unknown baseline selection for -b");
		}
		else if(flag == "c" || flag == "column")
		{
			cerr <<
			"ERROR: flag -" << flag << ":\n"
			"As of June 2011, you can no longer specify the column on which a strategy\n"
			"is applied with rfistrategy: use the -column parameter of rficonsole (or select\n"
			"the proper column when opening the ms in rfigui)\n";
			return 1;
		}
		else if(flag == "ff" || flag == "freq-based-flagging")	{ frequencyBasedFlagging = true;	}
		else if(flag == "fs" || flag == "flag-stokes")	{ flagStokes = true; }
		else if(flag == "j" || flag == "threads") { ++parameterIndex; threadCount = atoi(argv[parameterIndex]); }
		else if(flag == "ks" || flag == "kernel-size")
		{
			++parameterIndex;
			kernelSize = pair<double, double>( atof(argv[parameterIndex]), atof(argv[parameterIndex+1]));
			++parameterIndex;
		}
		else if(flag == "p" || flag == "polarizations")
		{
			++parameterIndex;
			string polStr(argv[parameterIndex]);
			if(polStr == "all") polarisation = DipolePolarisation;
			else if(polStr == "auto") polarisation = AutoDipolePolarisation;
			else if(polStr == "stokesi") polarisation = StokesIPolarisation;
			else throw runtime_error("Unknown polarisation type for -p");
		}
		else if(flag == "s" || flag == "sensitivity") { ++parameterIndex; sensitivity = atof(argv[parameterIndex]); }
		else if(flag == "ws" || flag == "window-size")
		{
			++parameterIndex;
			windowSize = pair<size_t, size_t>( atoi(argv[parameterIndex]), atoi(argv[parameterIndex+1]));
			++parameterIndex;
		}
		else
		{
			cerr << "Incorrect usage; parameter \"" << argv[parameterIndex] << "\" not understood.\nType rfistrategy without parameters for a list of commands." << endl;
			return 1;
		}
		++parameterIndex;
	}
	if((int) parameterIndex > argc-2)
	{
		cerr << "Usage: " << argv[0] << " [options] <profile> <filename>\n\n"
			"Profiles:\n"
			"  fast     Fastest strategy that provides a moderate\n"
			"           result in quality.\n"
			"  average  Best trade-off between speed and quality.\n"
			"  best     Highest quality detection.\n"
			"   NOTE: currently, all profiles perform equally. Since the speed\n"
			"         is currently limited by IO alone, all strategies flag using\n"
			"         the highest accuracy. This might change over time, once the\n"
			"         IO problem has been solved.\n"
			"  pedantic Pedantic detection. Like the 'best' profile,\n"
			"           but will flag all channels completely that are still\n"
			"           deviating from others after flagging. Flags about twice\n"
			"           as much.\n"
			"  pulsar   Like the best strategy, but will not assume\n"
			"           smoothness in time; especially usefull for pulsar\n"
			"           observations.\n"
			"<filename> is the filename to which the strategy is written. This\n"
			"file should have the extension \".rfis\".\n\n"
			"All profiles implement the SumThreshold method. The details of this\n"
			"method are described in the article named \"Post-correlation radio\n"
			"frequency interference classiﬁcation methods\", MNRAS 405 (2010) 155-167.\n"
			"\n"
			"Possible options:\n"
//			"-a -antennae"
			"-b or -baseline <all/auto/cross>\n"
			"  Specify which baselines to process (default: all)\n"
			"-c or -column <DATA/CORRECTED_DATA/...>\n"
			"  Specify which column to use when reading the data (default: DATA)\n"
//			"-cf or -clear-flags\n"
//			"-f  or -freq <channel start>-<channel end>\n"
			"-ff or -freq-based-flagging\n"
			"  Overrides default behaviour of smoothing in both time and frequency:\n"
			"  does not assume time smoothness. Useful e.g. if strong time-dependent\n"
			"  sources are expected (e.g. pulsars). Default: not enabled, except in\n"
			"  \'pulsar\' strategy.\n"
			"-fs or -flag-stokes\n"
			"  Will calculate the stokes I, Q, U and V components from the orthogonal\n"
			"  components (calculated with I=XX + YY, Q=XX - YY, U=XY + YX, V=i*XY - YX),\n"
			"  and use these values for flagging. All polarisations need to be read for this,\n"
			"  thus this option is only useful together with '-polarizations all'.\n"
			"-j or -threads <threadcount>\n"
			"  Set number of threads to use. Each thread will independently process\n"
			"  whole baselines, thus this has implications on both memory usage and\n"
			"  CPU usage. Defaults to 3, also overridable in rficonsole.\n"
			"-ks or -kernel-size <width> <height>\n"
			"  Gaussian kernel size used for smoothing. Floats. \n"
			"  Note that the temporal resolution is temporary decreased before\n"
			"  Gaussian smoothing, which makes the kernel size three times\n"
			"  apparantly larger.\n"
			"  Default: 2.5(x3) time steps x 15.0 channels.\n"
			"-p or -polarizations <all/auto/stokesi>\n"
			"  Specify what polarizations to read and process. Independent of this setting,\n"
			"  the flags of all polarizations will be or-ed together and all polarizations\n"
			"  will be set to that value.\n"
			"-s or -sensitivity <threshold factor>\n"
			"  Set a factor that is applied to each (sum)threshold operation. Higher\n"
			"  values mean higher thresholds, thus less flagged samples. Default: 1.\n"
//			"-t  or -time <time start index>-<time end index>\n"
			"-ws or -window-size <width in timesteps> <height in channels>\n"
			"  Window size used in smoothing. Integers. \n"
			"  Note that the temporal resolution is temporary decreased before\n"
			"  Gaussian smoothing, which makes the window size three times\n"
			"  apparantly larger.\n"
			"  Default: 10(x3) time steps x 40 channels (pulsar strategy: 1 x 40)\n"
			"\nScripts are recommended to use the long option names.\n";
		return 1;
	}

	string profile(argv[parameterIndex]), filename(argv[parameterIndex+1]);

	rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
	if(profile == "fast")
		strategy->LoadFastStrategy(false, false);
	else if(profile == "average" || profile == "default")
		strategy->LoadAverageStrategy(false, false);
	else if(profile == "best")
		strategy->LoadBestStrategy(false, false);
	else if(profile == "pedantic")
		strategy->LoadBestStrategy(true, false);
	else if(profile == "pulsar")
		strategy->LoadBestStrategy(true, true);
	else {
		cerr << "Unknown profile: " << profile << endl;
		return 1;
	}

	if(baselineSelection.IsSet())
		Strategy::SetBaselines(*strategy, baselineSelection);
	if(dataColumn.IsSet())
		Strategy::SetDataColumnName(*strategy, dataColumn);
	if(flagStokes.IsSet())
		Strategy::SetFlagStokes(*strategy, flagStokes.Value());
	if(frequencyBasedFlagging.IsSet() && frequencyBasedFlagging.Value())
		Strategy::SetTransientCompatibility(*strategy);
	if(threadCount.IsSet())
		Strategy::SetThreadCount(*strategy, threadCount);
	if(kernelSize.IsSet())
		Strategy::SetFittingKernelSize(*strategy, kernelSize.Value().first, kernelSize.Value().second);
	if(polarisation.IsSet())
		Strategy::SetPolarisations(*strategy, polarisation);
	if(sensitivity.IsSet())
		Strategy::SetMultiplySensitivity(*strategy, sensitivity);
	if(windowSize.IsSet())
		Strategy::SetFittingWindowSize(*strategy, windowSize.Value().first, windowSize.Value().second);

	rfiStrategy::StrategyWriter writer;
	cout << "Writing strategy..." << endl;
	writer.WriteToFile(*strategy, filename);
	delete strategy;
}
