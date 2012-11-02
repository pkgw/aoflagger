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

#ifndef AOREMOTE__SERVER_CONNECTION_H
#define AOREMOTE__SERVER_CONNECTION_H

#include <string>

#include <boost/asio/ip/tcp.hpp>

#include <boost/enable_shared_from_this.hpp>

#include <sigc++/signal.h>

#include "format.h"

#include "../msio/antennainfo.h"
#include "../msio/msrowdataext.h"

class StatisticsCollection;
class HistogramCollection;

namespace aoRemote {

typedef boost::shared_ptr<class ServerConnection> ServerConnectionPtr;

class ServerConnection : public boost::enable_shared_from_this<ServerConnection>

{
	public:
		static ServerConnectionPtr Create(boost::asio::io_service &ioService)
		{
			return ServerConnectionPtr(new ServerConnection(ioService));
		}
		~ServerConnection();
		
		void StopClient();
		void ReadQualityTables(const std::string &msFilename, class StatisticsCollection &collection, HistogramCollection &histogramCollection);
		void ReadAntennaTables(const std::string &msFilename, boost::shared_ptr<std::vector<AntennaInfo> > antennas);
		void ReadBandTable(const std::string &msFilename, BandInfo &band);
		void ReadDataRows(const std::string &msFilename, size_t rowStart, size_t rowCount, MSRowDataExt *destinationArray);
		void WriteDataRows(const std::string &msFilename, size_t rowStart, size_t rowCount, const MSRowDataExt *rowArray);
		void Start();
		
		boost::asio::ip::tcp::socket &Socket() { return _socket; }
		
		sigc::signal<void, ServerConnectionPtr> &SignalAwaitingCommand() { return _onAwaitingCommand; }
		sigc::signal<void, ServerConnectionPtr, StatisticsCollection&, HistogramCollection&> &SignalFinishReadQualityTables() { return _onFinishReadQualityTables; }
		sigc::signal<void, ServerConnectionPtr, boost::shared_ptr<std::vector<AntennaInfo> >, size_t > &SignalFinishReadAntennaTables() { return _onFinishReadAntennaTables; }
		sigc::signal<void, ServerConnectionPtr, BandInfo&> &SignalFinishReadBandTable() { return _onFinishReadBandTable; }
		sigc::signal<void, ServerConnectionPtr, MSRowDataExt*, size_t> &SignalFinishReadDataRows() { return _onFinishReadDataRows; }
		sigc::signal<void, ServerConnectionPtr, const std::string&> &SignalError() { return _onError; }
		
		const std::string &Hostname() const { return _hostname; }
	private:
		ServerConnection(boost::asio::io_service &ioService);
		boost::asio::ip::tcp::socket _socket;
		std::string _hostname;
		
		sigc::signal<void, ServerConnectionPtr> _onAwaitingCommand;
		sigc::signal<void, ServerConnectionPtr, StatisticsCollection&, HistogramCollection&> _onFinishReadQualityTables;
		sigc::signal<void, ServerConnectionPtr, boost::shared_ptr<std::vector<AntennaInfo> >, size_t > _onFinishReadAntennaTables;
		sigc::signal<void, ServerConnectionPtr, BandInfo&> _onFinishReadBandTable;
		sigc::signal<void, ServerConnectionPtr, MSRowDataExt*, size_t> _onFinishReadDataRows;
		sigc::signal<void, ServerConnectionPtr, const std::string&> _onError;
		
		char *_buffer;
		
		void onReceiveInitialResponse();
		
		void onReceiveQualityTablesResponseHeader();
		void onReceiveQualityTablesResponseData(size_t dataSize);
		
		void onReceiveAntennaTablesResponseHeader();
		void onReceiveAntennaTablesResponseData(size_t dataSize);
		
		void onReceiveBandTableResponseHeader();
		void onReceiveBandTableResponseData(size_t dataSize);
		
		void onReceiveReadDataRowsResponseHeader();
		void onReceiveReadDataRowsResponseData(size_t dataSize);
		
		void onReceiveWriteDataRowsResponseHeader();
		
		void prepareBuffer(size_t size)
		{
			if(_buffer != 0) delete[] _buffer;
			_buffer = new char[size];
		}
		
		void handleError(const GenericReadResponseHeader &header);
		
		StatisticsCollection *_collection;
		HistogramCollection *_histogramCollection;
		boost::shared_ptr<std::vector<AntennaInfo> > _antennas;
		BandInfo *_band;
		MSRowDataExt *_readRowData;
		const MSRowDataExt *_writeRowData;
};
	
}

#endif
