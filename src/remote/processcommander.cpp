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

#include "processcommander.h"

#include <unistd.h> //gethostname
#include <boost/mem_fn.hpp>

#include "observationtimerange.h"
#include "serverconnection.h"

#include "../quality/statisticscollection.h"
#include "../quality/histogramcollection.h"
#include "../quality/statisticsderivator.h"

namespace aoRemote {

ProcessCommander::ProcessCommander(const ClusteredObservation &observation)
: _server(), _observation(observation)
{
	_server.SignalConnectionCreated().connect(boost::bind(&ProcessCommander::onConnectionCreated, this, _1, _2));
}

ProcessCommander::~ProcessCommander()
{
	endIdleConnections();
	for(std::vector<RemoteProcess*>::iterator i=_processes.begin();i!=_processes.end();++i)
	{
		delete *i;
	}
}

void ProcessCommander::endIdleConnections()
{
	ConnectionVector idleConnections = _idleConnections;
	_idleConnections.clear();
	for(ConnectionVector::const_iterator i=idleConnections.begin();i!=idleConnections.end();++i)
	{
		(*i)->StopClient();
	}
}

void ProcessCommander::Run(bool finishConnections)
{
	_errors.clear();
	_finishConnections = finishConnections;
	
	if(!_observation.GetItems().empty() && !_tasks.empty())
	{
		const std::string thisHostName = GetHostName();
		
		// make a list of the involved nodes
		_nodeCommands.Initialize(_observation);
		
		// recycle idle connections
		ConnectionVector list = _idleConnections;
		_idleConnections.clear();
		for(ConnectionVector::iterator i=list.begin();i!=list.end();++i)
		{
			onConnectionAwaitingCommand(*i);
		}
		
		if(_processes.empty())
		{
			//construct a process for each unique node name
			std::vector<std::string> list;
			_nodeCommands.NodeList(list);
			for(std::vector<std::string>::const_iterator i=list.begin();i!=list.end();++i)
			{
				RemoteProcess *process = new RemoteProcess(*i, thisHostName);
				process->SignalFinished() = boost::bind(&ProcessCommander::onProcessFinished, this, _1, _2, _3);
				process->Start();
				_processes.push_back(process);
			}
		}
		
		// We will now start accepting connections. The Run() method will not return until the server
		// stops listening and there are no more io operations pending. With asynchroneous
		// handles, the server and its connections will call onEvent...(). These handles
		// will push new tasks until all tasks in the ProcessCommander are finished.
		_server.Run();
	}
}

void ProcessCommander::continueReadQualityTablesTask(ServerConnectionPtr serverConnection)
{
	const std::string &hostname = serverConnection->Hostname();
	
	boost::mutex::scoped_lock lock(_mutex);
	ClusteredObservationItem item;
	if(_nodeCommands.Pop(hostname, item))
	{
		const std::string &msFilename = item.LocalPath();
		StatisticsCollection *statisticsCollection = new StatisticsCollection();
		HistogramCollection *histogramCollection = new HistogramCollection();
		serverConnection->ReadQualityTables(msFilename, *statisticsCollection, *histogramCollection);
	} else {
		handleIdleConnection(serverConnection);
		
		if(_nodeCommands.Empty())
			onCurrentTaskFinished();
	}
}

void ProcessCommander::continueReadAntennaTablesTask(ServerConnectionPtr serverConnection)
{
	boost::mutex::scoped_lock lock(_mutex);
	
	const std::string &hostname = serverConnection->Hostname();
	std::vector<AntennaInfo> *antennas = new std::vector<AntennaInfo>();
	serverConnection->ReadAntennaTables(_nodeCommands.Top(hostname).LocalPath(),
																			boost::shared_ptr<std::vector<AntennaInfo> >(antennas));
	
	onCurrentTaskFinished();
}

void ProcessCommander::continueReadBandTablesTask(ServerConnectionPtr serverConnection)
{
	const std::string &hostname = serverConnection->Hostname();
	
	boost::mutex::scoped_lock lock(_mutex);
	ClusteredObservationItem item;
	if(_nodeCommands.Pop(hostname, item))
	{
		const std::string &msFilename = item.LocalPath();
		serverConnection->ReadBandTable(msFilename, _bands[item.Index()]);
	} else {
		handleIdleConnection(serverConnection);
		
		if(_nodeCommands.Empty())
			onCurrentTaskFinished();
	}
}

void ProcessCommander::continueReadDataRowsTask(ServerConnectionPtr serverConnection)
{
	const std::string &hostname = serverConnection->Hostname();
	
	boost::mutex::scoped_lock lock(_mutex);
	ClusteredObservationItem item;
	if(_nodeCommands.Pop(hostname, item))
	{
		const std::string &msFilename = item.LocalPath();
		if(_rowCount != 0)
			serverConnection->ReadDataRows(msFilename, _rowStart, _rowCount, _readRowBuffer[item.Index()]);
		else
			serverConnection->ReadDataRows(msFilename, _rowStart, _rowCount, 0);
	} else {
		handleIdleConnection(serverConnection);
		
		if(_nodeCommands.Empty())
			onCurrentTaskFinished();
	}
}

void ProcessCommander::continueWriteDataRowsTask(ServerConnectionPtr serverConnection)
{
	const std::string &hostname = serverConnection->Hostname();
	
	boost::mutex::scoped_lock lock(_mutex);
	ClusteredObservationItem item;
	if(_nodeCommands.Pop(hostname, item))
	{
		const std::string &msFilename = item.LocalPath();
		_observationTimerange->GetTimestepData(item.Index(), _writeRowBuffer[item.Index()]);

		serverConnection->WriteDataRows(msFilename, _observationTimerange->TimeOffsetIndex(), _observationTimerange->TimestepCount(), _writeRowBuffer[item.Index()]);
	} else {
		handleIdleConnection(serverConnection);
		
		if(_nodeCommands.Empty())
			onCurrentTaskFinished();
	}
}

std::string ProcessCommander::GetHostName()
{
	char name[HOST_NAME_MAX];
	if(gethostname(name, HOST_NAME_MAX) == 0)
	{
		return std::string(name);
	} else {
		throw std::runtime_error("Error retrieving hostname");
	}
}

void ProcessCommander::onConnectionCreated(ServerConnectionPtr serverConnection, bool &acceptConnection)
{
	serverConnection->SignalAwaitingCommand().connect(boost::bind(&ProcessCommander::onConnectionAwaitingCommand, this, _1));
	serverConnection->SignalFinishReadQualityTables().connect(boost::bind(&ProcessCommander::onConnectionFinishReadQualityTables, this, _1, _2, _3));
	serverConnection->SignalFinishReadAntennaTables().connect(boost::bind(&ProcessCommander::onConnectionFinishReadAntennaTables, this, _1, _2, _3));
	serverConnection->SignalFinishReadBandTable().connect(boost::bind(&ProcessCommander::onConnectionFinishReadBandTable, this, _1, _2));
	serverConnection->SignalFinishReadDataRows().connect(boost::bind(&ProcessCommander::onConnectionFinishReadDataRows, this, _1, _2, _3));
	serverConnection->SignalError().connect(boost::bind(&ProcessCommander::onError, this, _1, _2));
	acceptConnection = true;
}

void ProcessCommander::onConnectionAwaitingCommand(ServerConnectionPtr serverConnection)
{
	switch(currentTask())
	{
		case ReadQualityTablesTask:
			continueReadQualityTablesTask(serverConnection);
			break;
		case ReadAntennaTablesTask:
			continueReadAntennaTablesTask(serverConnection);
			break;
		case ReadBandTablesTask:
			continueReadBandTablesTask(serverConnection);
			break;
		case ReadDataRowsTask:
			continueReadDataRowsTask(serverConnection);
			break;
		case WriteDataRowsTask:
			continueWriteDataRowsTask(serverConnection);
			break;
		case NoTask:
			handleIdleConnection(serverConnection);
			break;
		default:
			throw std::runtime_error("Unknown task");
	}
}

void ProcessCommander::onConnectionFinishReadQualityTables(ServerConnectionPtr serverConnection, StatisticsCollection &statisticsCollection, HistogramCollection &histogramCollection)
{
	boost::mutex::scoped_lock lock(_mutex);
	if(statisticsCollection.PolarizationCount() == 0)
		throw std::runtime_error("Client sent StatisticsCollection with 0 polarizations.");
	
	// If the collection is still empty, we need to set its polarization count
	if(_statisticsCollection->PolarizationCount() == 0)
		_statisticsCollection->SetPolarizationCount(statisticsCollection.PolarizationCount());
	
	_statisticsCollection->Add(statisticsCollection);
	
	if(!histogramCollection.Empty())
	{
		if(_correctHistograms)
		{
			DefaultStatistics thisStat(statisticsCollection.PolarizationCount());
			statisticsCollection.GetGlobalCrossBaselineStatistics(thisStat);
			DefaultStatistics singlePol = thisStat.ToSinglePolarization();
			double stddev = StatisticsDerivator::GetStatisticAmplitude(QualityTablesFormatter::DStandardDeviationStatistic, singlePol, 0);
			
			std::cout << "Scaling with " << 1.0 / stddev << ".\n";
			histogramCollection.Rescale(1.0 / stddev);
		}
		
		if(_histogramCollection->PolarizationCount() == 0)
			_histogramCollection->SetPolarizationCount(histogramCollection.PolarizationCount());
		
		_histogramCollection->Add(histogramCollection);
	}
	
	delete &statisticsCollection;
	delete &histogramCollection;
}

void ProcessCommander::onConnectionFinishReadAntennaTables(ServerConnectionPtr serverConnection, boost::shared_ptr<std::vector<AntennaInfo> > antennas, size_t polarizationCount)
{
	boost::mutex::scoped_lock lock(_mutex);
	_polarizationCount = polarizationCount;
	_antennas = *antennas;
}

void ProcessCommander::onConnectionFinishReadBandTable(ServerConnectionPtr serverConnection, BandInfo &band)
{
	// Nothing needs to be done.
}

void ProcessCommander::onConnectionFinishReadDataRows(ServerConnectionPtr serverConnection, MSRowDataExt *rowData, size_t totalRows)
{
	const std::string &hostname = serverConnection->Hostname();
	ClusteredObservationItem item;
	_nodeCommands.Current(hostname, item);
	_observationTimerange->SetTimestepData(item.Index(), rowData, _rowCount);
	_observationTimerange->SetTimeOffsetIndex(_rowStart);
	if(_rowsTotal < totalRows)
		_rowsTotal = totalRows;
}

void ProcessCommander::onError(ServerConnectionPtr connection, const std::string &error)
{
	std::stringstream s;
	
	const std::string &hostname = connection->Hostname();
	ClusteredObservationItem item;
	bool knowFile = _nodeCommands.Current(hostname, item);
	s << "On connection with " << hostname;
	if(knowFile)
		s << " to process local file '" << item.LocalPath() << "'";
	s << ", reported error was: " << error;
	boost::mutex::scoped_lock lock(_mutex);
	_errors.push_back(s.str());
}

void ProcessCommander::onProcessFinished(RemoteProcess &process, bool error, int status)
{
	boost::mutex::scoped_lock lock(_mutex);
	
	if(_nodeCommands.RemoveNode(process.ClientHostname()) && _nodeCommands.Empty())
		onCurrentTaskFinished();
	
	if(error)
	{
		std::stringstream s;
		s << "Remote process to " << process.ClientHostname() << " reported an error";
		if(status != 0) s << " (status " << status << ")";
		_errors.push_back(s.str());
	}
}

std::string ProcessCommander::ErrorString() const
{
	if(!_errors.empty())
	{
		std::stringstream s;
		s << _errors.size() << " error(s) occured while querying the nodes or measurement sets in the given observation. This might be caused by a failing node, an unreadable measurement set, or maybe the quality tables are not available. The errors reported are:\n\n";
		size_t count = 0;
		for(std::vector<std::string>::const_iterator i=_errors.begin();i!=_errors.end() && count < 30;++i)
		{
			s << "- " << *i << '\n';
			++count;
		}
		if(_errors.size() > 30)
		{
			s << "... and " << (_errors.size()-30) << " more.\n";
		}
		return s.str();
	} else {
		return std::string();
	}
}

void ProcessCommander::CheckErrors() const
{
	if(!_errors.empty())
	{
		throw std::runtime_error(ErrorString());
	}
}

}
