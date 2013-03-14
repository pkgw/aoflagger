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
#ifndef RSPREADER_H
#define RSPREADER_H

#include <fstream>

#include "../util/aologger.h"

#include "timefrequencydata.h"
#include "timefrequencymetadata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class RSPReader {
	public:
		explicit RSPReader(const std::string &rawFile) : _rawFile(rawFile), _clockSpeed(200000000)
		{
		}
		
		~RSPReader()
		{
		}
		
		std::pair<TimeFrequencyData,TimeFrequencyMetaDataPtr> ReadChannelBeamlet(unsigned long timestepStart, unsigned long timestepEnd, unsigned beamletCount, unsigned beamletIndex);
		
		std::pair<TimeFrequencyData,TimeFrequencyMetaDataPtr> ReadSingleBeamlet(unsigned long timestepStart, unsigned long timestepEnd, unsigned beamletCount, unsigned beamletIndex);
		
		std::pair<TimeFrequencyData,TimeFrequencyMetaDataPtr> ReadAllBeamlets(unsigned long timestepStart, unsigned long timestepEnd, unsigned beamletCount);
		
		void ReadForStatistics(unsigned beamletCount);
		
		const std::string &File() const { return _rawFile; }
		
		unsigned long TimeStepCount(size_t beamletCount) const;
	private:
		static const unsigned char BitReverseTable256[256];

		static void readNetworkOrder(std::ifstream &stream, char *buffer, unsigned length)
		{
			std::vector<char> swappedBuffer(length);
			stream.read(&swappedBuffer[0], length);
			for(unsigned i=0,j=length-1;i<length;++i,--j)
				buffer[j] = swappedBuffer[i];
		}
		static unsigned char reverse(unsigned char c)
		{
			return BitReverseTable256[c];
		}
		static unsigned short reverse(unsigned short c)
		{
			return
				BitReverseTable256[c >> 8] |
				(BitReverseTable256[c &0xFF]<<8);
		}
		static signed short reverse(signed short c)
		{
			return
				BitReverseTable256[c >> 8] |
				(BitReverseTable256[c &0xFF]<<8);
		}
		static unsigned int reverse(unsigned int c)
		{
			return
				BitReverseTable256[c >> 24] |
				(BitReverseTable256[(c >> 16)&0xFF]<<8) |
				(BitReverseTable256[(c >> 8)&0xFF]<<16) |
				(BitReverseTable256[c & 0xFF]<<24);
		}
		static signed short toShort(unsigned char c1, unsigned char c2)
		{
			return (c2<<8) | c1;
		}
		static unsigned short toUShort(unsigned char c1, unsigned char c2)
		{
			return (c1<<8) | c2;
		}
		static unsigned int toUInt(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
		{
			return (c1<<24) | (c2<<16) | (c3<<8) | c4;
		}
		
		const std::string _rawFile;
		const unsigned long _clockSpeed;
		static const unsigned long STATION_INTEGRATION_STEPS;

		struct RCPTransportHeader {
			unsigned char versionAndHeaderLength : 8;
			unsigned char typeOfService : 8;
			unsigned short totalLength : 16;
			unsigned short identification : 16;
			unsigned int flagsAndFragmentOffset : 8;
			unsigned int ttl : 8;
			unsigned int protocol : 8;
			unsigned int headerCrc : 16;
			unsigned int sourceIP : 32;
			unsigned int destinationIP : 32;
			unsigned int udpSourcePort : 16;
			unsigned int udpDestPort : 16;
			unsigned int udpLength : 16;
			unsigned int udpChecksum : 16;
			
			void Print() const
			{
				AOLogger::Debug
					<< "versionAndHeaderLength=" << versionAndHeaderLength << "\n"
					<< "typeOfService=" << typeOfService << "\n"
					<< "totalLength=" << totalLength << "\n"
					<< "identification=" << identification << "\n"
					<< "flagsAndFragmentOffset=" << flagsAndFragmentOffset << "\n"
					<< "ttl=" << ttl << "\n"
					<< "protocol=" << protocol << "\n"
					<< "headerCrc=" << headerCrc << "\n"
					<< "sourceIP=" << sourceIP << "\n"
					<< "destinationIP=" << destinationIP << "\n"
					<< "udpSourcePort=" << udpSourcePort << "\n"
					<< "udpDestPort=" << udpDestPort << "\n"
					<< "udpLength=" << udpLength << "\n"
					<< "udpChecksum=" << udpChecksum << "\n";
			}
		};

		struct RCPApplicationHeader {
			unsigned char versionId;
			unsigned char sourceInfo;
			unsigned short configurationId;
			unsigned short stationId;
			unsigned char nofBeamlets;
			unsigned char nofBlocks;
			unsigned int timestamp;
			unsigned int blockSequenceNumber;
			
			static const unsigned int SIZE;
			
			void Read(std::ifstream &stream)
			{
				unsigned char buffer[16];
				stream.read(reinterpret_cast<char*>(buffer), 16);
				versionId = buffer[0];
				sourceInfo = buffer[1];
				configurationId = toUShort(buffer[3], buffer[2]);
				stationId = toUShort(buffer[5], buffer[4]);
				nofBeamlets = buffer[6];
				nofBlocks = buffer[7];
				timestamp = toUInt(buffer[11], buffer[10], buffer[9], buffer[8]);
				blockSequenceNumber = toUInt(buffer[15], buffer[14], buffer[13], buffer[12]);
			}
			
			void Print() const
			{
				AOLogger::Debug
					<< "versionId=" << (unsigned int) versionId << "\n"
					<< "sourceInfo=" << (unsigned int) sourceInfo << "\n"
					<< "configurationId=" << (unsigned int) configurationId << "\n"
					<< "stationId=" << (unsigned int) stationId << "\n"
					<< "nofBeamlets=" << (unsigned int) nofBeamlets << "\n"
					<< "nofBlocks=" << (unsigned int) nofBlocks << "\n"
					<< "timestamp=" << (unsigned int) timestamp << "\n"
					<< "blockSequenceNumber=" << (unsigned int) blockSequenceNumber << "\n";
			}
		};
		struct RCPBeamletData {
			signed short xr, xi;
			signed short yr, yi;
			
			static const unsigned int SIZE;
			
			void Read(std::ifstream &stream)
			{
				unsigned char buffer[8];
				stream.read(reinterpret_cast<char*>(buffer), 8);
				xr = toShort(buffer[6], buffer[7]);
				xi = toShort(buffer[4], buffer[5]);
				yr = toShort(buffer[2], buffer[3]);
				yi = toShort(buffer[0], buffer[1]);
			}
			
			void Print()
			{
				AOLogger::Debug << "x=" << xr;
				if(xi > 0) AOLogger::Debug << "+" << xi << "i";
				else AOLogger::Debug << "-" << (-xi) << "i";
				AOLogger::Debug << ",y=" << yr;
				if(yi > 0) AOLogger::Debug << "+" << yi << "i\n";
				else AOLogger::Debug << "-" << (-yi) << "i\n";
			}
		};
		struct BeamletStatistics {
			BeamletStatistics() : totalCount(0) {
				for(unsigned i=0;i<16;++i)
					bitUseCount[i] = 0;
			}
			void Print()
			{
				for(unsigned bit=0;bit<16;++bit)
				{
					AOLogger::Info
						<< "Bit " << bit << " times required: "
						<< bitUseCount[bit] << " ("
						<< (100.0 * (double) bitUseCount[bit] / (double) totalCount) << "%)"
						<< '\n';
				}
			}
			unsigned long totalCount;
			unsigned long bitUseCount[16];
		};
};

#endif // RSPREADER_H
