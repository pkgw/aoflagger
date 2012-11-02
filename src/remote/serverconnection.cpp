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

#include "serverconnection.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>

#include <boost/bind.hpp>

#include "format.h"

#include "../util/autoarray.h"

#include "../quality/statisticscollection.h"
#include "../quality/histogramcollection.h"

namespace aoRemote
{

ServerConnection::ServerConnection(boost::asio::io_service &ioService) :
	_socket(ioService), _buffer(0)
{
}

ServerConnection::~ServerConnection()
{
	if(_buffer != 0)
		delete[] _buffer;
}

void ServerConnection::Start()
{
	InitialBlock initialBlock;
	initialBlock.blockIdentifier = InitialId;
	initialBlock.blockSize = sizeof(initialBlock);
	initialBlock.options = 0;
	initialBlock.protocolVersion = AO_REMOTE_PROTOCOL_VERSION;
	
	boost::asio::write(_socket, boost::asio::buffer(&initialBlock, sizeof(initialBlock)));
	
	prepareBuffer(sizeof(InitialResponseBlock));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(InitialResponseBlock)), boost::bind(&ServerConnection::onReceiveInitialResponse, shared_from_this()));
}

void ServerConnection::onReceiveInitialResponse()
{
	InitialResponseBlock initialResponse = *reinterpret_cast<InitialResponseBlock*>(_buffer);
	enum ErrorCode errorCode = (enum ErrorCode) initialResponse.errorCode;
	if(initialResponse.blockIdentifier != InitialResponseId || initialResponse.blockSize != sizeof(initialResponse))
		throw std::runtime_error("Bad response from client during initial response");
	if(errorCode != NoError)
		throw std::runtime_error(std::string("Error reported by client during initial response: ") + ErrorStr::GetStr(errorCode));
	if(initialResponse.negotiatedProtocolVersion != AO_REMOTE_PROTOCOL_VERSION)
		throw std::runtime_error("Client seems to run different protocol version");
	if(initialResponse.hostNameSize == 0 || initialResponse.hostNameSize > 65536)
		throw std::runtime_error("Client did not send proper hostname");
	
	char hostname[initialResponse.hostNameSize + 1];
	boost::asio::read(_socket, boost::asio::buffer(hostname, initialResponse.hostNameSize));
	hostname[initialResponse.hostNameSize] = 0;
	_hostname = hostname;
	
	_onAwaitingCommand(shared_from_this());
}

void ServerConnection::StopClient()
{
	RequestBlock requestBlock;
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = 0;
	requestBlock.request = StopClientRequest;
	boost::asio::write(_socket, boost::asio::buffer(&requestBlock, sizeof(requestBlock)));
}

void ServerConnection::ReadQualityTables(const std::string &msFilename, StatisticsCollection &collection, HistogramCollection &histogramCollection)
{
	_collection = &collection;
	_histogramCollection = &histogramCollection;
	
	std::stringstream reqBuffer;
	
	RequestBlock requestBlock;
	ReadQualityTablesRequestOptions options;
	
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = sizeof(options.flags) + msFilename.size();
	requestBlock.request = ReadQualityTablesRequest;
	reqBuffer.write(reinterpret_cast<char *>(&requestBlock), sizeof(requestBlock));
	
	options.flags = 0;
	options.msFilename = msFilename;
	reqBuffer.write(reinterpret_cast<char *>(&options.flags), sizeof(options.flags));
	reqBuffer.write(reinterpret_cast<const char *>(options.msFilename.c_str()), options.msFilename.size());
	
	boost::asio::write(_socket, boost::asio::buffer(reqBuffer.str()));
	
	prepareBuffer(sizeof(GenericReadResponseHeader));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(GenericReadResponseHeader)),
		boost::bind(&ServerConnection::onReceiveQualityTablesResponseHeader, shared_from_this()));
}

void ServerConnection::ReadAntennaTables(const std::string &msFilename, boost::shared_ptr<std::vector<AntennaInfo> > antennas)
{
	_antennas = antennas;
	
	std::stringstream reqBuffer;
	
	RequestBlock requestBlock;
	ReadAntennaTablesRequestOptions options;
	
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = sizeof(options.flags) + msFilename.size();
	requestBlock.request = ReadAntennaTablesRequest;
	reqBuffer.write(reinterpret_cast<char *>(&requestBlock), sizeof(requestBlock));
	
	options.flags = 0;
	options.msFilename = msFilename;
	reqBuffer.write(reinterpret_cast<char *>(&options.flags), sizeof(options.flags));
	reqBuffer.write(reinterpret_cast<const char *>(options.msFilename.c_str()), options.msFilename.size());
	
	boost::asio::write(_socket, boost::asio::buffer(reqBuffer.str()));
	
	std::cout << "Requesting antenna tables from " << Hostname() << "...\n";
	
	prepareBuffer(sizeof(GenericReadResponseHeader));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(GenericReadResponseHeader)),
		boost::bind(&ServerConnection::onReceiveAntennaTablesResponseHeader, shared_from_this()));
}

void ServerConnection::ReadBandTable(const std::string &msFilename, BandInfo &band)
{
	_band = &band;
	
	std::cout << "Requesting band table from " << Hostname() << "...\n";
	std::stringstream reqBuffer;
	
	RequestBlock requestBlock;
	ReadBandTableRequestOptions options;
	
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = sizeof(options.flags) + msFilename.size();
	requestBlock.request = ReadBandTableRequest;
	reqBuffer.write(reinterpret_cast<char *>(&requestBlock), sizeof(requestBlock));
	
	options.flags = 0;
	options.msFilename = msFilename;
	reqBuffer.write(reinterpret_cast<char *>(&options.flags), sizeof(options.flags));
	reqBuffer.write(reinterpret_cast<const char *>(options.msFilename.c_str()), options.msFilename.size());
	
	boost::asio::write(_socket, boost::asio::buffer(reqBuffer.str()));
	
	prepareBuffer(sizeof(GenericReadResponseHeader));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(GenericReadResponseHeader)),
		boost::bind(&ServerConnection::onReceiveBandTableResponseHeader, shared_from_this()));
}

void ServerConnection::ReadDataRows(const std::string &msFilename, size_t rowStart, size_t rowCount, MSRowDataExt *destinationArray)
{
	_readRowData = destinationArray;
	
	std::stringstream reqBuffer;
	
	RequestBlock requestBlock;
	ReadDataRowsRequestOptions options;
	
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = sizeof(options.flags) + msFilename.size() + sizeof(options.startRow) + sizeof(options.rowCount);
	requestBlock.request = ReadDataRowsRequest;
	reqBuffer.write(reinterpret_cast<char *>(&requestBlock), sizeof(requestBlock));
	
	options.flags = 0;
	options.msFilename = msFilename;
	options.startRow = rowStart;
	options.rowCount = rowCount;
	
	reqBuffer.write(reinterpret_cast<char *>(&options.flags), sizeof(options.flags));
	reqBuffer.write(reinterpret_cast<const char *>(options.msFilename.c_str()), options.msFilename.size());
	reqBuffer.write(reinterpret_cast<const char *>(&options.startRow), sizeof(options.startRow));
	reqBuffer.write(reinterpret_cast<const char *>(&options.rowCount), sizeof(options.rowCount));
	
	boost::asio::write(_socket, boost::asio::buffer(reqBuffer.str()));
	
	prepareBuffer(sizeof(GenericReadResponseHeader));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(GenericReadResponseHeader)),
		boost::bind(&ServerConnection::onReceiveReadDataRowsResponseHeader, shared_from_this()));
}

void ServerConnection::WriteDataRows(const std::string &msFilename, size_t rowStart, size_t rowCount, const MSRowDataExt *rowArray)
{
	_writeRowData = rowArray;
	
	std::stringstream reqBuffer;
	
	RequestBlock requestBlock;
	WriteDataRowsRequestOptions options;
	
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = sizeof(options.flags) + msFilename.size() + sizeof(options.startRow) + sizeof(options.rowCount) + sizeof(options.dataSize);
	requestBlock.request = WriteDataRowsRequest;
	reqBuffer.write(reinterpret_cast<char *>(&requestBlock), sizeof(requestBlock));
	
	std::ostringstream dataBuffer;
	// Serialize the rows
	for(size_t rowIndex=0; rowIndex != rowCount; ++rowIndex) {
		rowArray[rowIndex].Serialize(dataBuffer);
	}
	std::string dataBufferStr = dataBuffer.str();

	options.flags = 0;
	options.msFilename = msFilename;
	options.startRow = rowStart;
	options.rowCount = rowCount;
	options.dataSize = dataBufferStr.size();
	
	reqBuffer.write(reinterpret_cast<char *>(&options.flags), sizeof(options.flags));
	reqBuffer.write(reinterpret_cast<const char *>(options.msFilename.c_str()), options.msFilename.size());
	reqBuffer.write(reinterpret_cast<const char *>(&options.startRow), sizeof(options.startRow));
	reqBuffer.write(reinterpret_cast<const char *>(&options.rowCount), sizeof(options.rowCount));
	reqBuffer.write(reinterpret_cast<const char *>(&options.dataSize), sizeof(options.dataSize));
	
	boost::asio::write(_socket, boost::asio::buffer(reqBuffer.str()));
	boost::asio::write(_socket, boost::asio::buffer(dataBufferStr));
	
	prepareBuffer(sizeof(GenericReadResponseHeader));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(GenericReadResponseHeader)),
		boost::bind(&ServerConnection::onReceiveWriteDataRowsResponseHeader, shared_from_this()));
}

void ServerConnection::handleError(const GenericReadResponseHeader &header)
{
	std::stringstream s;
	s << "Client reported \"" << ErrorStr::GetStr(header.errorCode) << '\"';
	if(header.dataSize > 0)
	{
		char message[header.dataSize+1];
		boost::asio::read(_socket, boost::asio::buffer(message, header.dataSize));
		message[header.dataSize] = 0;
		s << " (detailed info: " << message << ')';
	}
	_onError(shared_from_this(), s.str());
}

void ServerConnection::onReceiveQualityTablesResponseHeader()
{
	GenericReadResponseHeader responseHeader = *reinterpret_cast<GenericReadResponseHeader*>(_buffer);
	if(responseHeader.blockIdentifier != GenericReadResponseHeaderId || responseHeader.blockSize != sizeof(responseHeader))
	{
		_onError(shared_from_this(), "Bad response from client upon read quality tables request");
		StopClient();
	}
	else if(responseHeader.errorCode != NoError)
	{
		handleError(responseHeader);
		_onAwaitingCommand(shared_from_this());
	}
	else {
		prepareBuffer(responseHeader.dataSize);
		boost::asio::async_read(_socket, boost::asio::buffer(_buffer, responseHeader.dataSize),
			boost::bind(&ServerConnection::onReceiveQualityTablesResponseData, shared_from_this(), responseHeader.dataSize));
	}
}

void ServerConnection::onReceiveQualityTablesResponseData(size_t dataSize)
{
	std::istringstream stream;
	if(stream.rdbuf()->pubsetbuf(_buffer, dataSize) == 0)
		throw std::runtime_error("Could not set string buffer");
	
	std::cout << "Received quality table of size " << dataSize << "." << std::endl;
	_collection->Unserialize(stream);
	if(stream.tellg() != (std::streampos) dataSize)
	{
		size_t histogramTablesSize = dataSize - stream.tellg();
		std::cout << "Processing histogram tables of size " << histogramTablesSize << "." << std::endl;
		_histogramCollection->Unserialize(stream);
	}

	_onFinishReadQualityTables(shared_from_this(), *_collection, *_histogramCollection);
	_onAwaitingCommand(shared_from_this());
}

void ServerConnection::onReceiveAntennaTablesResponseHeader()
{
	GenericReadResponseHeader responseHeader = *reinterpret_cast<GenericReadResponseHeader*>(_buffer);
	if(responseHeader.blockIdentifier != GenericReadResponseHeaderId || responseHeader.blockSize != sizeof(responseHeader))
	{
		_onError(shared_from_this(), "Bad response from client upon read antenna tables request");
		StopClient();
	}
	else if(responseHeader.errorCode != NoError)
	{
		handleError(responseHeader);
		_onAwaitingCommand(shared_from_this());
	}
	else {
		prepareBuffer(responseHeader.dataSize);
		boost::asio::async_read(_socket, boost::asio::buffer(_buffer, responseHeader.dataSize),
			boost::bind(&ServerConnection::onReceiveAntennaTablesResponseData, shared_from_this(), responseHeader.dataSize));
	}
}

void ServerConnection::onReceiveBandTableResponseHeader()
{
	GenericReadResponseHeader responseHeader = *reinterpret_cast<GenericReadResponseHeader*>(_buffer);
	if(responseHeader.blockIdentifier != GenericReadResponseHeaderId || responseHeader.blockSize != sizeof(responseHeader))
	{
		_onError(shared_from_this(), "Bad response from client upon read band table request");
		StopClient();
	}
	else if(responseHeader.errorCode != NoError)
	{
		handleError(responseHeader);
		_onAwaitingCommand(shared_from_this());
	}
	else {
		prepareBuffer(responseHeader.dataSize);
		boost::asio::async_read(_socket, boost::asio::buffer(_buffer, responseHeader.dataSize),
			boost::bind(&ServerConnection::onReceiveBandTableResponseData, shared_from_this(), responseHeader.dataSize));
	}
}

void ServerConnection::onReceiveAntennaTablesResponseData(size_t dataSize)
{
	std::istringstream stream;
	if(stream.rdbuf()->pubsetbuf(_buffer, dataSize) == 0)
		throw std::runtime_error("Could not set string buffer");
	
	std::cout << "Received antenna table of size " << dataSize << "." << std::endl;
	size_t polarizationCount = Serializable::UnserializeUInt32(stream);
	size_t count = Serializable::UnserializeUInt32(stream);
	for(size_t i=0;i<count;++i)
	{
		_antennas->push_back(AntennaInfo());
		_antennas->rbegin()->Unserialize(stream);
	}

	_onFinishReadAntennaTables(shared_from_this(), _antennas, polarizationCount);
	_onAwaitingCommand(shared_from_this());
}

void ServerConnection::onReceiveBandTableResponseData(size_t dataSize)
{
	std::istringstream stream;
	if(stream.rdbuf()->pubsetbuf(_buffer, dataSize) == 0)
		throw std::runtime_error("Could not set string buffer");
	
	_band->Unserialize(stream);

	_onFinishReadBandTable(shared_from_this(), *_band);
	_onAwaitingCommand(shared_from_this());
}

void ServerConnection::onReceiveReadDataRowsResponseHeader()
{
	GenericReadResponseHeader responseHeader = *reinterpret_cast<GenericReadResponseHeader*>(_buffer);
	if(responseHeader.blockIdentifier != GenericReadResponseHeaderId || responseHeader.blockSize != sizeof(responseHeader))
	{
		_onError(shared_from_this(), "Bad response from client upon read data rows request");
		StopClient();
	}
	else if(responseHeader.errorCode != NoError)
	{
		handleError(responseHeader);
		_onAwaitingCommand(shared_from_this());
	}
	else {
		prepareBuffer(responseHeader.dataSize);
		boost::asio::async_read(_socket, boost::asio::buffer(_buffer, responseHeader.dataSize),
			boost::bind(&ServerConnection::onReceiveReadDataRowsResponseData, shared_from_this(), responseHeader.dataSize));
	}
}

void ServerConnection::onReceiveReadDataRowsResponseData(size_t dataSize)
{
	std::istringstream stream;
	if(stream.rdbuf()->pubsetbuf(_buffer, dataSize) == 0)
		throw std::runtime_error("Could not set string buffer");
	
	size_t rowsSent = Serializable::UnserializeUInt64(stream);
	size_t rowsTotal = 0;
	if(rowsSent == 0)
		rowsTotal = Serializable::UnserializeUInt64(stream);
	for(size_t i=0;i<rowsSent;++i)
		_readRowData[i].Unserialize(stream);

	_onFinishReadDataRows(shared_from_this(), _readRowData, rowsTotal);
	_onAwaitingCommand(shared_from_this());
}

void ServerConnection::onReceiveWriteDataRowsResponseHeader()
{
	GenericReadResponseHeader responseHeader = *reinterpret_cast<GenericReadResponseHeader*>(_buffer);
	if(responseHeader.blockIdentifier != GenericReadResponseHeaderId || responseHeader.blockSize != sizeof(responseHeader))
	{
		_onError(shared_from_this(), "Bad response from client upon write data rows request");
		StopClient();
	}
	else if(responseHeader.errorCode != NoError)
	{
		handleError(responseHeader);
		_onAwaitingCommand(shared_from_this());
	}
	else if(responseHeader.dataSize != 0) {
		_onError(shared_from_this(), "Client sent unexpected data during write rows action");
		StopClient();
	} else {
		_onAwaitingCommand(shared_from_this());
	}
}

}

