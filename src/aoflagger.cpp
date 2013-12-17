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
#include <string>

#include <libgen.h>

#include "strategy/actions/foreachmsaction.h"
#include "strategy/actions/strategyaction.h"

#include "strategy/algorithms/baselineselector.h"
#include "strategy/algorithms/polarizationstatistics.h"

#include "strategy/plots/antennaflagcountplot.h"
#include "strategy/plots/frequencyflagcountplot.h"
#include "strategy/plots/timeflagcountplot.h"

#include "strategy/control/artifactset.h"
#include "strategy/control/strategyreader.h"

#include "util/aologger.h"
#include "util/parameter.h"
#include "util/progresslistener.h"
#include "util/stopwatch.h"

#include "version.h"

#include <boost/date_time/posix_time/posix_time.hpp>

class ConsoleProgressHandler : public ProgressListener {
	private:
		boost::mutex _mutex;
		
	public:
		
		virtual void OnStartTask(const rfiStrategy::Action &action, size_t taskNo, size_t taskCount, const std::string &description, size_t weight)
		{
			boost::mutex::scoped_lock lock(_mutex);
			ProgressListener::OnStartTask(action, taskNo, taskCount, description, weight);
			
			double totalProgress = TotalProgress();
			std::ostringstream ss;
			ss.width (5);
			ss.precision (1);
			ss.setf (std::ios_base::fixed);
			ss << totalProgress * 100 << "% : ";
			AOLogger::Progress << ss.str ();
			
			for(size_t i=1;i<Depth();++i)
				AOLogger::Progress << "+-";
			
			AOLogger::Progress << description << "... \r";
		}
		
		virtual void OnEndTask(const rfiStrategy::Action &action)
		{
			boost::mutex::scoped_lock lock(_mutex);
			
			ProgressListener::OnEndTask(action);
		}

		virtual void OnProgress(const rfiStrategy::Action &action, size_t i, size_t j)
		{
			ProgressListener::OnProgress(action, i, j);
		}

		virtual void OnException(const rfiStrategy::Action &, std::exception &thrownException) 
		{
			AOLogger::Error <<
				"An exception occured during execution of the strategy!\n"
				"Your set might not be fully flagged. Exception was:\n"
				<< thrownException.what() << '\n';
		}
};

#define RETURN_SUCCESS                0
#define RETURN_CMDLINE_ERROR         10
#define RETURN_STRATEGY_PARSE_ERROR  20
#define RETURN_UNHANDLED_EXCEPTION   30

void checkRelease()
{
#ifndef NDEBUG
		AOLogger::Warn
			<< "This version of the AOFlagger has been compiled as DEBUG version! (NDEBUG was not defined)\n"
			<< "For better performance, recompile it as a RELEASE.\n\n";
#endif
}
void generalInfo()
{
	AOLogger::Info << 
		"AOFlagger " << AOFLAGGER_VERSION_STR << " (" << AOFLAGGER_VERSION_DATE_STR <<
		") command line application\n"
		"This program will execute an RFI strategy as can be created with the RFI gui\n"
		"and executes it on one or several observations.\n\n"
		"Author: André Offringa (offringa@gmail.com)\n\n";
}

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		AOLogger::Init(basename(argv[0]));
		generalInfo();
		AOLogger::Error << "Usage: " << argv[0] << " [options] <obs1> [<obs2> [..]]\n"
		"  -v will produce verbose output\n"
		"  -q : quiet output\n"
		"  -j overrides the number of threads specified in the strategy\n"
		"     (default: one thread for each CPU core)\n"
		"  -strategy specifies a possible customized strategy\n"
		"  -direct-read will perform the slowest IO but will always work.\n"
		"  -indirect-read will reorder the measurement set before starting, which is normally\n"
		"     faster but requires free disk space to reorder the data to.\n"
		"  -memory-read will read the entire measurement set in memory. This is the fastest, but\n"
		"     requires much memory.\n"
		"  -auto-read-mode will select either memory or direct mode based on available memory (default).\n"
		"  -skip-flagged will skip an ms if it has already been processed by AOFlagger according\n"
		"     to its HISTORY table.\n"
		"  -uvw reads uvw values (some exotic strategies require these)\n"
		"  -column <NAME> specify column to flag\n\n"
		"This tool supports at least the Casa measurement set format and the SDFITS format. See\n"
		"documentation for support of other file types.\n";
		
		checkRelease();
		
		return RETURN_CMDLINE_ERROR;
	}
	
#ifdef HAS_LOFARSTMAN
	register_lofarstman();
#endif // HAS_LOFARSTMAN
	
	Parameter<size_t> threadCount;
	Parameter<BaselineIOMode> readMode;
	Parameter<bool> readUVW;
	Parameter<std::string> strategyFile;
	Parameter<bool> logVerbose;
	Parameter<bool> logQuiet;
	Parameter<bool> skipFlagged;
	Parameter<std::string> dataColumn;

	size_t parameterIndex = 1;
	while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
	{
		std::string flag(argv[parameterIndex]+1);
		
		// If "--" was used, strip another dash
		if(!flag.empty() && flag[0] == '-')
			flag = flag.substr(1);
		
		if(flag=="j" && parameterIndex < (size_t) (argc-1))
		{
			threadCount = atoi(argv[parameterIndex+1]);
			parameterIndex+=2;
		}
		else if(flag=="v")
		{
			logVerbose = true;
			++parameterIndex;
		}
		else if(flag == "version")
		{
			AOLogger::Init(basename(argv[0]));
			AOLogger::Info << "AOFlagger " << AOFLAGGER_VERSION_STR << " (" << AOFLAGGER_VERSION_DATE_STR << ")\n";
			return 0;
		}
		else if(flag=="q")
		{
			logQuiet = true;
			++parameterIndex;
		}
		else if(flag=="direct-read")
		{
			readMode = DirectReadMode;
			++parameterIndex;
		}
		else if(flag=="indirect-read")
		{
			readMode = IndirectReadMode;
			++parameterIndex;
		}
		else if(flag=="memory-read")
		{
			readMode = MemoryReadMode;
			++parameterIndex;
		}
		else if(flag=="auto-read-mode")
		{
			readMode = AutoReadMode;
			++parameterIndex;
		}
		else if(flag=="strategy")
		{
			strategyFile = argv[parameterIndex+1];
			parameterIndex+=2;
		}
		else if(flag=="skip-flagged")
		{
			skipFlagged = true;
			++parameterIndex;
		}
		else if(flag=="uvw")
		{
			readUVW = true;
			++parameterIndex;
		}
		else if(flag == "column")
		{
			std::string columnStr(argv[parameterIndex+1]);
			parameterIndex+=2;
			dataColumn = columnStr; 
		}
		else
		{
			AOLogger::Init(basename(argv[0]));
			AOLogger::Error << "Incorrect usage; parameter \"" << argv[parameterIndex] << "\" not understood.\n";
			return 1;
		}
	}

	try {
		AOLogger::Init(basename(argv[0]), false, logVerbose.Value(false), logQuiet.Value(false));

		if(!threadCount.IsSet())
			threadCount = sysconf(_SC_NPROCESSORS_ONLN);
		AOLogger::Debug << "Number of threads: " << threadCount.Value() << "\n";

		Stopwatch watch(true);

		boost::mutex ioMutex;
		
		rfiStrategy::ForEachMSAction *fomAction = new rfiStrategy::ForEachMSAction();
		if(readMode.IsSet())
			fomAction->SetIOMode(readMode);
		if(readUVW.IsSet())
			fomAction->SetReadUVW(readUVW);
		if(dataColumn.IsSet())
			fomAction->SetDataColumnName(dataColumn);
		std::stringstream commandLineStr;
		commandLineStr << argv[0];
		for(int i=1;i<argc;++i)
		{
			commandLineStr << " \"" << argv[i] << '\"';
		}
		fomAction->SetCommandLineForHistory(commandLineStr.str());
		if(skipFlagged.IsSet())
			fomAction->SetSkipIfAlreadyProcessed(skipFlagged);
		for(int i=parameterIndex;i<argc;++i)
		{
			AOLogger::Debug << "Adding '" << argv[i] << "'\n";
			fomAction->Filenames().push_back(argv[i]);
		}
		
		if(!strategyFile.IsSet())
		{
			fomAction->SetLoadOptimizedStrategy(true);
			fomAction->Add(new rfiStrategy::Strategy()); // This helps the progress reader to determine progress
			if(threadCount.IsSet())
				fomAction->SetLoadStrategyThreadCount(threadCount);
		} else {
			fomAction->SetLoadOptimizedStrategy(false);
			rfiStrategy::StrategyReader reader;
			rfiStrategy::Strategy *subStrategy;
			try {
				AOLogger::Debug << "Opening strategy file '" << strategyFile.Value() << "'\n";
				subStrategy = reader.CreateStrategyFromFile(strategyFile);
				AOLogger::Debug << "Strategy parsed succesfully.\n";
			} catch(std::exception &e)
			{
				AOLogger::Error <<
					"ERROR: Reading strategy file \"" << strategyFile.Value() << "\" failed! This\n"
					"might be caused by a change in the file format of the strategy file after you\n"
					"created the strategy file, as it is still rapidly changing.\n"
					"Try recreating the file with rfistrategy.\n"
					"\nThe thrown exception was:\n" << e.what() << "\n";
				return RETURN_STRATEGY_PARSE_ERROR;
			}
			fomAction->Add(subStrategy);
			if(threadCount.IsSet())
				rfiStrategy::Strategy::SetThreadCount(*subStrategy, threadCount);
		}
		
		rfiStrategy::Strategy overallStrategy;
		overallStrategy.Add(fomAction);

		rfiStrategy::ArtifactSet artifacts(&ioMutex);
		artifacts.SetAntennaFlagCountPlot(new AntennaFlagCountPlot());
		artifacts.SetFrequencyFlagCountPlot(new FrequencyFlagCountPlot());
		artifacts.SetTimeFlagCountPlot(new TimeFlagCountPlot());
		artifacts.SetPolarizationStatistics(new PolarizationStatistics());
		artifacts.SetBaselineSelectionInfo(new rfiStrategy::BaselineSelector());
		
		ConsoleProgressHandler progress;

		AOLogger::Info << "Starting strategy on " << to_simple_string(boost::posix_time::microsec_clock::local_time()) << '\n';
		
		overallStrategy.InitializeAll();
		overallStrategy.StartPerformThread(artifacts, progress);
		rfiStrategy::ArtifactSet *set = overallStrategy.JoinThread();
		overallStrategy.FinishAll();

		AOLogger::Progress << '\n'; /* finally new line after \rs */

		set->AntennaFlagCountPlot()->Report();
		set->FrequencyFlagCountPlot()->Report();
		set->PolarizationStatistics()->Report();

		delete set->AntennaFlagCountPlot();
		delete set->FrequencyFlagCountPlot();
		delete set->TimeFlagCountPlot();
		delete set->PolarizationStatistics();
		delete set->BaselineSelectionInfo();

		delete set;

		AOLogger::Debug << "Time: " << watch.ToString() << "\n";
		
		return RETURN_SUCCESS;
	} catch(std::exception &exception)
	{
		std::cerr
			<< "An unhandled exception occured: " << exception.what() << '\n'
			<< "If you think this is a bug, please contact offringa@astro.rug.nl\n";
		return RETURN_UNHANDLED_EXCEPTION;
	}
}
