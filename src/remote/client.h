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

#ifndef AOREMOTE__CLIENT_H
#define AOREMOTE__CLIENT_H

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "format.h"

namespace aoRemote {

class Client
{
	public:
		Client();
		
		void Run(const std::string &serverHost);
		
		static unsigned PORT() { return 1892; }
		
	private:
		boost::asio::io_service _ioService;
		boost::asio::ip::tcp::socket _socket;
		
		void writeGenericReadException(const std::exception &e);
		void writeGenericReadException(const std::string &s);
		void writeGenericReadError(enum ErrorCode code);
		void writeDataResponse(std::ostringstream &buffer);
		
		std::string readStr(unsigned size);
		
		void handleReadQualityTables(unsigned dataSize);
		void handleReadAntennaTables(unsigned dataSize);
		void handleReadBandTable(unsigned dataSize);
		void handleReadDataRows(unsigned dataSize);
		void handleWriteDataRows(unsigned dataSize);
};
	
}

#endif
