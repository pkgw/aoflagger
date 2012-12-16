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

#include "strategy/actions/foreachmsaction.h"
#include "strategy/actions/strategyaction.h"

#include "strategy/algorithms/baselineselector.h"
#include "strategy/algorithms/polarizationstatistics.h"

#include "strategy/plots/antennaflagcountplot.h"
#include "strategy/plots/frequencyflagcountplot.h"
#include "strategy/plots/timeflagcountplot.h"

#include "strategy/control/artifactset.h"
#include "strategy/control/defaultstrategy.h"
#include "strategy/control/strategyreader.h"

#include "util/aologger.h"
#include "util/parameter.h"
#include "util/progresslistener.h"
#include "util/stopwatch.h"

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
			
			AOLogger::Progress << round(totalProgress*1000.0)/10.0 << "% : ";
			
			for(size_t i=1;i<Depth();++i)
				AOLogger::Progress << "+-";
			
			AOLogger::Progress << description << "... \n";
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
			AOLogger::Error << thrownException.what() << '\n';
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
			<< "This version of RFI console has been compiled as DEBUG version! (NDEBUG was not defined)\n"
			<< "For better performance, recompile it as a RELEASE.\n\n";
#endif
}

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		AOLogger::Init(basename(argv[0]));
		AOLogger::Error << "Usage: " << argv[0] << " [options] <ms1> [<ms2> [..]]\n"
		"  -v will produce verbose output\n"
		"  -j overrides the number of threads specified in the strategy\n"
		"  -strategy specifies a possible customized strategy\n"
		"  -indirect-read will reorder the measurement set before starting, which is normally faster\n"
		"  -memory-read will read the entire measurement set in memory. This is the fastest, but requires large memory.\n"
		"  -direct-read will perform the slowest IO but will always work.\n"
		"  -auto-read-mode will select either memory or direct mode based on available memory (default).\n"
		"  -skip-flagged will skip an ms if it has already been processed by RFI console according\n"
		"   to its HISTORY table.\n"
		"  -uvw reads uvw values (some strategies require them)\n"
		"  -column <NAME> specify column to flag\n";
		
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
	Parameter<bool> skipFlagged;
	Parameter<std::string> dataColumn;

	size_t parameterIndex = 1;
	while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
	{
		std::string flag(argv[parameterIndex]+1);
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
			string columnStr(argv[parameterIndex+1]);
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
		AOLogger::Init(basename(argv[0]), false, logVerbose.Value(false));
		AOLogger::Info << 
			"RFI strategy console runner\n"
			"This program will execute an RFI strategy as can be created with the RFI gui\n"
			"or a console program called rfistrategy, and executes it on one or several .MS\n"
			"directories.\n\n"
			"Author: André Offringa (offringa@astro.rug.nl)\n\n";
			
		checkRelease();

		if(!threadCount.IsSet())
			threadCount = sysconf(_SC_NPROCESSORS_ONLN);
		AOLogger::Debug << "Number of threads: " << threadCount.Value() << "\n";

		Stopwatch watch(true);

		boost::mutex ioMutex;
		
		rfiStrategy::Strategy *subStrategy;
		if(!strategyFile.IsSet())
		{
			subStrategy = new rfiStrategy::Strategy();
			rfiStrategy::DefaultStrategy::LoadFullStrategy(*subStrategy, rfiStrategy::DefaultStrategy::GENERIC_TELESCOPE, rfiStrategy::DefaultStrategy::FLAG_NONE);
		} else {
			rfiStrategy::StrategyReader reader;
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
		}
		if(threadCount.IsSet())
			rfiStrategy::Strategy::SetThreadCount(*subStrategy, threadCount);
			
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
		fomAction->Add(subStrategy);
		
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
