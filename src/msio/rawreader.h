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
#ifndef RAWREADER_H
#define RAWREADER_H

#include <cstring>
#include <fstream>
#include <string>

#include "../util/aologger.h"

class RawReader
{
	public:
		explicit RawReader(const std::string &filename) :
		_blockHeaderSize(512),
		_blockFooterSize(8),
		_beamCount(1),
		_subbandCount(1),
		_channelsPerSubbandCount(1),
		_timestepsPerBlockCount(12208),
		_filename(filename),
		_useNetworkOrder(true),
		_outputStream(0),
		_timeStepsInWriteBlock(0),
		_block(*this)
		{
			AOLogger::Debug << "RawReader for " << filename << " constructed.\n";
		}
		
		~RawReader()
		{
			if(_outputStream != 0)
				FinishWrite();
		}
		
		size_t TimestepCount()
		{
			std::ifstream stream(_filename.c_str());
			stream.seekg(0, std::ios_base::end);
			std::streampos fileSize = stream.tellg();
			unsigned long blockSize = BlockSize();
			return (fileSize / blockSize) * _timestepsPerBlockCount;
		}
		
		void Read(size_t startIndex, size_t endIndex, float *dest)
		{
			AOLogger::Debug << "Reading " << startIndex << " to " << endIndex << " (total: " << TimestepCount() << ") with " << _beamCount << " beams. \n";
			
			std::ifstream stream(_filename.c_str());
			
			size_t startBlock = startIndex / _timestepsPerBlockCount;
			stream.seekg(startBlock * BlockSize(), std::ios_base::beg);
			
			const size_t rowsPerTimestep =  _beamCount * _subbandCount * _channelsPerSubbandCount;
			const size_t bytesPerTimestep = sizeof(float) * rowsPerTimestep;
			
			RawBlock block(*this);
			block.read(stream);
			
			size_t startTimestepInFirstBlock = startIndex - startBlock * _timestepsPerBlockCount;
			
			size_t totalTimestepsInFirstBlock = (_timestepsPerBlockCount - startTimestepInFirstBlock);
			if(totalTimestepsInFirstBlock > endIndex - startIndex)
				totalTimestepsInFirstBlock = endIndex - startIndex;
			
			memcpy(dest, block.SamplePtr(startTimestepInFirstBlock), bytesPerTimestep * totalTimestepsInFirstBlock);
			
			size_t samplesCopied = rowsPerTimestep * totalTimestepsInFirstBlock;
			
			size_t currentIndex = (startBlock + 1) * _timestepsPerBlockCount;
			while(currentIndex < endIndex)
			{
				AOLogger::Debug << currentIndex << ", stream=" << stream.tellg() << "\n";
				block.read(stream);
				if(currentIndex + _timestepsPerBlockCount > endIndex)
				{
					// Whole block won't fit
					memcpy(&dest[samplesCopied], block.SamplePtr(), bytesPerTimestep * (endIndex - currentIndex));
					samplesCopied += rowsPerTimestep * (endIndex - currentIndex);
					currentIndex += (endIndex - currentIndex);
				}
				else {
					// Block fits
					memcpy(&dest[samplesCopied], block.SamplePtr(), bytesPerTimestep * _timestepsPerBlockCount);
					samplesCopied += rowsPerTimestep * _timestepsPerBlockCount;
					currentIndex += _timestepsPerBlockCount;
				}
			}
			AOLogger::Debug << "Done reading.\n";
		}
		
		void StartWrite()
		{
			_outputStream = new std::ofstream(_filename.c_str());
			_block = RawBlock(*this);
		}
		
		void FinishWrite()
		{
			if(_timeStepsInWriteBlock != 0)
			{
				for(size_t i=_timeStepsInWriteBlock;i<_timestepsPerBlockCount;++i)
				{
					float *currentSamplePtr = _block.SamplePtr(i);
					for(size_t j=0;j<_beamCount * _subbandCount * _channelsPerSubbandCount;++j)
					{
						currentSamplePtr[j] = 0.0;
					}
				}
				_block.write(*_outputStream);
			}
			delete _outputStream;
			_outputStream = 0;
		}
		
		void Write(float *data, size_t timestepCount)
		{
			float *currentSamplePtr = _block.SamplePtr(_timeStepsInWriteBlock);
			size_t writeCount;
			
			// Fill the current block
			size_t writeTimesteps = _timestepsPerBlockCount - _timeStepsInWriteBlock;
			if(writeTimesteps > timestepCount)
				writeTimesteps = timestepCount;
			writeCount = _beamCount * _subbandCount * _channelsPerSubbandCount * writeTimesteps;
			memcpy(currentSamplePtr, data, sizeof(float) * writeCount);
			timestepCount -= writeTimesteps;
			data += writeTimesteps;
			_timeStepsInWriteBlock += writeTimesteps;
			
			// Has the block been filled?
			if(_timeStepsInWriteBlock == _timestepsPerBlockCount)
			{
				_block.write(*_outputStream);
				_timeStepsInWriteBlock = 0;
				
				// Write whole blocks until no whole block of data is available
				writeCount = _beamCount * _subbandCount * _channelsPerSubbandCount * _timestepsPerBlockCount;
				while(timestepCount >= _timestepsPerBlockCount)
				{
					memcpy(_block.SamplePtr(), data, sizeof(float) * writeCount);
					_block.write(*_outputStream);
					timestepCount -= _timestepsPerBlockCount;
					data += _timestepsPerBlockCount;
				}
				
				// Fill the last block as far as available
				writeCount = _beamCount * _subbandCount * _channelsPerSubbandCount * timestepCount;
				memcpy(_block.SamplePtr(_timeStepsInWriteBlock), data, sizeof(float) * writeCount);
				_timeStepsInWriteBlock += timestepCount;
			}
		}
		
		const std::string &Filename() const { return _filename; }
		
		size_t BlockSize() const
		{
			return _blockHeaderSize + _blockFooterSize +
				_beamCount * _subbandCount * _channelsPerSubbandCount * _timestepsPerBlockCount * sizeof(float);
		}
		
		void SetSubbandCount(unsigned count) { _subbandCount = count; }
		void SetChannelCount(unsigned count) { _channelsPerSubbandCount = count; }
		void SetBeamCount(unsigned count) { _beamCount = count; }
		void SetTimestepsPerBlockCount(unsigned count) { _timestepsPerBlockCount = count; }
		void SetBlockHeaderSize(unsigned size) { _blockHeaderSize = size; }
		void SetBlockFooterSize(unsigned size) { _blockFooterSize = size; }
		
	private:
		unsigned _blockHeaderSize;
		unsigned _blockFooterSize;
		unsigned _beamCount;
		unsigned _subbandCount;
		unsigned _channelsPerSubbandCount;
		unsigned _timestepsPerBlockCount;
		const std::string _filename;
		bool _useNetworkOrder;
		
		std::ofstream *_outputStream;
		size_t _timeStepsInWriteBlock;
		
		void readBlock();

		class RawBlock
		{
			public:
				RawBlock(RawReader &reader) :
				_reader(reader)
				{
					initialize();
				}
				
				~RawBlock()
				{
					deinitialize();
				}
				
				void operator=(const RawBlock &source)
				{
					deinitialize();
					initialize();
				}
				
				void initialize()
				{
					_header = new unsigned char[_reader._blockHeaderSize];
					_data = new float[_reader._beamCount * _reader._subbandCount * _reader._channelsPerSubbandCount * _reader._timestepsPerBlockCount];
					_postFix = new unsigned char[_reader._blockFooterSize];
				}
				
				void deinitialize()
				{
					delete[] _header;
					delete[] _data;
					delete[] _postFix;
				}
				
				void read(std::istream &stream)
				{
					size_t length = _reader._beamCount * _reader._subbandCount * _reader._channelsPerSubbandCount * _reader._timestepsPerBlockCount;
					stream.read(reinterpret_cast<char*>(_header), _reader._blockHeaderSize);
					stream.read(reinterpret_cast<char*>(_data), length * sizeof(float));
					stream.read(reinterpret_cast<char*>(_postFix), _reader._blockFooterSize);
					
					if(_reader._useNetworkOrder)
					{
						for(size_t i=0;i<length;++i)
						{
							_data[i] = swapfloat(_data[i]);
						}
					}
				}
				
				void write(std::ostream &stream)
				{
					size_t length = _reader._beamCount * _reader._subbandCount * _reader._channelsPerSubbandCount * _reader._timestepsPerBlockCount;
					if(_reader._useNetworkOrder)
					{
						for(size_t i=0;i<length;++i)
						{
							_data[i] = swapfloat(_data[i]);
						}
					}
					
					stream.write(reinterpret_cast<char*>(_header), _reader._blockHeaderSize);
					stream.write(reinterpret_cast<char*>(_data), length * sizeof(float));
					stream.write(reinterpret_cast<char*>(_postFix), _reader._blockFooterSize);
					
					if(_reader._useNetworkOrder)
					{
						for(size_t i=0;i<length;++i)
						{
							_data[i] = swapfloat(_data[i]);
						}
					}
				}
				
				float *SamplePtr()
				{
					return _data;
				}
				
				float *SamplePtr(size_t sampleIndex)
				{
					size_t dataIndex =
						sampleIndex * _reader._beamCount * _reader._channelsPerSubbandCount * _reader._subbandCount;
					return &_data[dataIndex];
				}
				
				float swapfloat(float input)
				{
					union { char valueChar[4]; float valueFloat; } a, b;
					a.valueFloat = input;
					b.valueChar[3] = a.valueChar[0];
					b.valueChar[2] = a.valueChar[1];
					b.valueChar[1] = a.valueChar[2];
					b.valueChar[0] = a.valueChar[3];
					return b.valueFloat;
				}
				
			private:
				RawReader &_reader;
				unsigned char *_header;
				float *_data;
				unsigned char *_postFix;
		};
		
		RawBlock _block;
};

#endif
