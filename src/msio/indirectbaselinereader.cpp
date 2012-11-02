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
#include "indirectbaselinereader.h"

#include <fstream>
#include <set>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include "arraycolumniterator.h"
#include "scalarcolumniterator.h"
#include "timefrequencydata.h"
#include "system.h"

#include "../util/aologger.h"
#include "../util/stopwatch.h"

IndirectBaselineReader::IndirectBaselineReader(const std::string &msFile) : BaselineReader(msFile), _directReader(msFile), _msIsReordered(false), _removeReorderedFiles(false), _reorderedDataFilesHaveChanged(false), _reorderedFlagFilesHaveChanged(false), _maxMemoryUse(1024*1024*1024), _readUVW(false)
{
	AOLogger::Debug << "Total system memory detected: " << System::TotalMemory() << '\n';
	if(System::TotalMemory() < 3l*1024l*1024l*1024l)
	{
		AOLogger::Warn << "Memory available is less than 3 GB: using alternative buffer sizes" << '\n';
		_maxMemoryUse = 128*1024*1024;
	}
}

IndirectBaselineReader::~IndirectBaselineReader()
{
	if(_reorderedDataFilesHaveChanged)
		updateOriginalMSData();
	if(_reorderedFlagFilesHaveChanged)
		updateOriginalMSFlags();
	removeTemporaryFiles();
}

void IndirectBaselineReader::PerformReadRequests()
{
	initialize();

	if(!_msIsReordered) initializeReorderedMS();

	_results.clear();
	AOLogger::Debug << "Performing " << _readRequests.size() << " read requests...\n";
	for(size_t i=0;i<_readRequests.size();++i)
	{
		const ReadRequest request = _readRequests[i];
		_results.push_back(Result());
		const size_t width = AllObservationTimes().size();
		for(size_t p=0;p<PolarizationCount();++p)
		{
			if(ReadData()) {
				_results[i]._realImages.push_back(Image2D::CreateZeroImagePtr(width, FrequencyCount()));
				_results[i]._imaginaryImages.push_back(Image2D::CreateZeroImagePtr(width, FrequencyCount()));
			}
			if(ReadFlags()) {
				// The flags should be initialized to true, as a baseline might
				// miss some time scans that other baselines do have, and these
				// should be flagged.
				_results[i]._flags.push_back(Mask2D::CreateSetMaskPtr<true>(width, FrequencyCount()));
			}
		}
		if(_readUVW)
			_results[i]._uvw = _directReader.ReadUVW(request.antenna1, request.antenna2, request.spectralWindow);
		else {
			_results[i]._uvw.clear();
			for(unsigned j=0;j<width;++j)
			_results[i]._uvw.push_back(UVW(0.0, 0.0, 0.0));
		}

		const std::string dataFilename = DataFilename(request.antenna1, request.antenna2);
		std::ifstream dataFile(dataFilename.c_str(), std::ifstream::binary);

		const std::string flagFilename = FlagFilename(request.antenna1, request.antenna2);
		std::ifstream flagFile(flagFilename.c_str(), std::ifstream::binary);

		const size_t bufferSize = FrequencyCount() * PolarizationCount();
		for(size_t x=0;x<width;++x)
		{
			float dataBuffer[bufferSize*2];
			bool flagBuffer[bufferSize];
			dataFile.read((char *) dataBuffer, bufferSize * sizeof(float) * 2);
			size_t dataBufferPtr = 0;
			flagFile.read((char *) flagBuffer, bufferSize * sizeof(bool));
			size_t flagBufferPtr = 0;
			for(size_t f=0;f<FrequencyCount();++f) {
				for(size_t p=0;p<PolarizationCount();++p)
				{
					_results[i]._realImages[p]->SetValue(x, f, dataBuffer[dataBufferPtr]);
					++dataBufferPtr;
					_results[i]._imaginaryImages[p]->SetValue(x, f, dataBuffer[dataBufferPtr]);
					++dataBufferPtr;
					_results[i]._flags[p]->SetValue(x, f, flagBuffer[flagBufferPtr]);
					++flagBufferPtr;
				}
			}
		}
	}
	AOLogger::Debug << "Done reading.\n";

	_readRequests.clear();
}

void IndirectBaselineReader::PerformFlagWriteRequests()
{
	for(size_t i=0;i!=_writeRequests.size();++i)
	{
		const WriteRequest request = _writeRequests[i];
		performFlagWriteTask(request.flags, request.antenna1, request.antenna2 /* TODO, request.spectralWindow, request.startIndex, request.endIndex, request.leftBorder, request.rightBorder */);
	}
	_writeRequests.clear();
}

void IndirectBaselineReader::initializeReorderedMS()
{
	boost::filesystem::path path("ms-aoinfo.txt");
	bool reorderRequired = true;
	
	if(boost::filesystem::exists(path))
	{
		std::ifstream str(path.string().c_str());
		std::string name;
		std::getline(str, name);
		if(boost::filesystem::equivalent(boost::filesystem::path(name), Set().Location()))
		{
			AOLogger::Debug << "Measurement set has already been reordered; using old temporary files.\n";
			reorderRequired = false;
			_msIsReordered = true;
			_removeReorderedFiles = false;
			_reorderedDataFilesHaveChanged = false;
			_reorderedFlagFilesHaveChanged = false;
		}
	}
	if(reorderRequired)
	{
		reorderMS();
		std::ofstream str(path.string().c_str());
		str << Set().Location() << '\n';
	}
}

void IndirectBaselineReader::reorderMS()
{
	casa::Table &table = *Table();

	casa::ROScalarColumn<double> timeColumn(*Table(), "TIME");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");

	int rowCount = table.nrow();
	if(rowCount == 0)
		throw std::runtime_error("Measurement set is empty (zero rows)");

	casa::ROArrayColumn<casa::Complex> *dataColumn = new casa::ROArrayColumn<casa::Complex>(table, DataColumnName());

	std::vector<std::pair<size_t,size_t> > baselines;
	Set().GetBaselines(baselines);
	size_t
		antennaCount = Set().AntennaCount(),
		frequencyCount = FrequencyCount(),
		polarizationCount = PolarizationCount();

	size_t bufferSize = _maxMemoryUse / (baselines.size() * frequencyCount * polarizationCount * (sizeof(float) * 2 + sizeof(bool)));

	std::vector<std::vector<float *> > dataBuffers;
	std::vector<std::vector<bool *> > flagBuffers;
	std::vector<std::vector<std::ofstream *> > dataFiles;
	std::vector<std::vector<std::ofstream *> > flagFiles;
	for(size_t i=0;i<antennaCount;++i) {
		dataBuffers.push_back(std::vector<float *>());
		dataFiles.push_back(std::vector<std::ofstream *>());
		flagBuffers.push_back(std::vector<bool *>());
		flagFiles.push_back(std::vector<std::ofstream *>());
		for(size_t j=0;j<antennaCount;++j) {
			dataBuffers[i].push_back(0);
			dataFiles[i].push_back(0);
			flagBuffers[i].push_back(0);
			flagFiles[i].push_back(0);
		}
	}

	AOLogger::Debug << "Requesting " << (sizeof(float)*2+sizeof(bool)) << " x " << baselines.size() << " x " << bufferSize << " x " << polarizationCount << " x " << frequencyCount << " bytes of data\n";
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		const std::string dataFilename = DataFilename(i->first, i->second);
		const std::string flagFilename = FlagFilename(i->first, i->second);
		dataBuffers[i->first][i->second] = new float[2 * bufferSize * frequencyCount * polarizationCount];
		dataFiles[i->first][i->second] = new std::ofstream(dataFilename.c_str(), std::ofstream::binary);
		if(dataFiles[i->first][i->second]->fail())
			throw std::runtime_error("Error: failed to open temporary data files for writing! Check access rights and free disk space.");
		flagBuffers[i->first][i->second] = new bool[bufferSize * frequencyCount * polarizationCount];
		flagFiles[i->first][i->second] = new std::ofstream(flagFilename.c_str(), std::ofstream::binary);
		if(flagFiles[i->first][i->second]->fail())
			throw std::runtime_error("Error: failed to open temporary data files for writing! Check access rights and free disk space.");
	}

	size_t
		prevTimeIndex = (size_t) (-1),
		timeIndex = 0,
		currentBufferBlockPtr = (size_t) (-1);
	double prevTime = -1.0;

	AOLogger::Debug << 'R';
	AOLogger::Debug.Flush();
	for(int rowIndex = 0;rowIndex < rowCount;++rowIndex)
	{
		double time = timeColumn(rowIndex);
		if(time != prevTime)
		{
			timeIndex = AllObservationTimes().find(time)->second;
			if(timeIndex != prevTimeIndex+1)
			{
				std::stringstream s;
				s << "Error: time step " << prevTimeIndex << " is followed by time step " << timeIndex;
				throw std::runtime_error(s.str());
			}
			prevTime = time;
			prevTimeIndex = timeIndex;
			++currentBufferBlockPtr;

			if(currentBufferBlockPtr >= bufferSize)
			{
				// buffer is full, flush
				AOLogger::Debug << 'W';
				AOLogger::Debug.Flush();
				for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
				{
					const size_t sampleCount = bufferSize * frequencyCount * polarizationCount;
					float *dataBuffer = dataBuffers[i->first][i->second];
					bool *flagBuffer = flagBuffers[i->first][i->second];
					std::ofstream *dataFile = dataFiles[i->first][i->second];
					std::ofstream *flagFile = flagFiles[i->first][i->second];
					dataFile->write(reinterpret_cast<char*>(dataBuffer), sampleCount * 2 * sizeof(float));
					if(dataFile->fail())
						throw std::runtime_error("Error: failed to write temporary data files! Check access rights and free disk space.");
					flagFile->write(reinterpret_cast<char*>(flagBuffer), sampleCount * sizeof(bool));
					if(flagFile->fail())
						throw std::runtime_error("Error: failed to write temporary flag files! Check access rights and free disk space.");
				}

				AOLogger::Debug << 'R';
				AOLogger::Debug.Flush();
				currentBufferBlockPtr = 0;
			}
		}
		
		casa::Array<casa::Complex> data = (*dataColumn)(rowIndex);
		casa::Array<casa::Complex>::const_iterator dataI=data.begin();
		casa::Array<bool> flag = flagColumn(rowIndex);
		casa::Array<bool>::const_iterator flagI=flag.begin();
		size_t dataBufferPtr = currentBufferBlockPtr*2*frequencyCount*polarizationCount;
		float *dataBuffer = dataBuffers[antenna1Column(rowIndex)][antenna2Column(rowIndex)];
		size_t flagBufferPtr = currentBufferBlockPtr*frequencyCount*polarizationCount;
		bool *flagBuffer = flagBuffers[antenna1Column(rowIndex)][antenna2Column(rowIndex)];
		for(size_t f=0;f<frequencyCount;++f) {
			for(size_t p=0;p<polarizationCount;++p) {
				dataBuffer[dataBufferPtr] = dataI->real();
				++dataBufferPtr;
				dataBuffer[dataBufferPtr] = dataI->imag();
				++dataI;
				++dataBufferPtr;

				flagBuffer[flagBufferPtr] = *flagI;
				++flagBufferPtr;
				++flagI;
			}
		}
	}
	
	// flush half-full buffer
	AOLogger::Debug << "W";
	AOLogger::Debug.Flush();
	++currentBufferBlockPtr; // Since we have finished reading the last timestep
	                         // the buffer contains one more
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		const size_t sampleCount = currentBufferBlockPtr * frequencyCount * polarizationCount;
		float *dataBuffer = dataBuffers[i->first][i->second];
		bool *flagBuffer = flagBuffers[i->first][i->second];
		std::ofstream *dataFile = dataFiles[i->first][i->second];
		std::ofstream *flagFile = flagFiles[i->first][i->second];
		dataFile->write(reinterpret_cast<char*>(dataBuffer), sampleCount * 2 * sizeof(float));
		if(dataFile->fail())
			throw std::runtime_error("Error: failed to write temporary data files! Check access rights and free disk space.");
		flagFile->write(reinterpret_cast<char*>(flagBuffer), sampleCount * sizeof(bool));
		if(flagFile->fail())
			throw std::runtime_error("Error: failed to write temporary flag files! Check access rights and free disk space.");
	}

	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		delete[] dataBuffers[i->first][i->second];
		delete[] flagBuffers[i->first][i->second];
		delete dataFiles[i->first][i->second];
		delete flagFiles[i->first][i->second];
	}

	delete dataColumn;

	clearTableCaches();

	AOLogger::Debug << "Done reordering data set\n";
	_msIsReordered = true;
	_removeReorderedFiles = true;
	_reorderedDataFilesHaveChanged = false;
	_reorderedFlagFilesHaveChanged = false;
}

void IndirectBaselineReader::removeTemporaryFiles()
{
	if(_msIsReordered && _removeReorderedFiles)
	{
		boost::filesystem::remove("ms-aoinfo.txt");
		std::vector<std::pair<size_t,size_t> > baselines;
		Set().GetBaselines(baselines);
		for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
		{
			boost::filesystem::remove(DataFilename(i->first, i->second));
			boost::filesystem::remove(FlagFilename(i->first, i->second));
		}
		AOLogger::Debug << "Temporary files removed.\n";
	}
	_msIsReordered = false;
	_removeReorderedFiles = false;
	_reorderedDataFilesHaveChanged = false;
	_reorderedFlagFilesHaveChanged = false;
}

void IndirectBaselineReader::PerformDataWriteTask(std::vector<Image2DCPtr> _realImages, std::vector<Image2DCPtr> _imaginaryImages, int antenna1, int antenna2, int /*spectralWindow*/)
{
	initialize();

	AOLogger::Debug << "Performing data write task with indirect baseline reader...\n";

	const size_t polarizationCount = PolarizationCount();
	
	if(_realImages.size() != polarizationCount || _imaginaryImages.size() != polarizationCount)
		throw std::runtime_error("PerformDataWriteTask: input format did not match number of polarizations in measurement set");
	
	for(size_t i=1;i<_realImages.size();++i)
	{
		if(_realImages[0]->Width() != _realImages[i]->Width() || _realImages[0]->Height() != _realImages[i]->Height() || _realImages[0]->Width() != _imaginaryImages[i]->Width() || _realImages[0]->Height() != _imaginaryImages[i]->Height())
		throw std::runtime_error("PerformDataWriteTask: width and/or height of input images did not match");
	}
	
	if(!_msIsReordered) initializeReorderedMS();
	
	const size_t width = _realImages[0]->Width();
	const size_t bufferSize = FrequencyCount() * PolarizationCount();
	
	const std::string dataFilename = DataFilename(antenna1, antenna2);
	std::ofstream dataFile(dataFilename.c_str(), std::ofstream::binary | std::ofstream::trunc);
		
	for(size_t x=0;x<width;++x)
	{
		float dataBuffer[bufferSize*2];
		
		size_t dataBufferPtr = 0;
		for(size_t f=0;f<FrequencyCount();++f) {
			for(size_t p=0;p<PolarizationCount();++p)
			{
				dataBuffer[dataBufferPtr] = _realImages[p]->Value(x, f);
				++dataBufferPtr;
				dataBuffer[dataBufferPtr] = _imaginaryImages[p]->Value(x, f);
				++dataBufferPtr;
			}
		}

		dataFile.write(reinterpret_cast<char*>(dataBuffer), bufferSize * sizeof(float) * 2);
		if(dataFile.bad())
			throw std::runtime_error("Error: failed to update temporary data files! Check access rights and free disk space.");
	}
	
	_reorderedDataFilesHaveChanged = true;
	
	AOLogger::Debug << "Done writing.\n";
}

void IndirectBaselineReader::performFlagWriteTask(std::vector<Mask2DCPtr> flags, int antenna1, int antenna2)
{
	initialize();

	const size_t polarizationCount = PolarizationCount();
	
	if(flags.size() != polarizationCount)
		throw std::runtime_error("PerformDataWriteTask: input format did not match number of polarizations in measurement set");
	
	for(size_t i=1;i<flags.size();++i)
	{
		if(flags[0]->Width() != flags[i]->Width() || flags[0]->Height() != flags[i]->Height())
		throw std::runtime_error("PerformDataWriteTask: width and/or height of input images did not match");
	}
	
	if(!_msIsReordered) initializeReorderedMS();
	
	const size_t width = flags[0]->Width();
	const size_t bufferSize = FrequencyCount() * PolarizationCount();
	
	const std::string flagFilename = FlagFilename(antenna1, antenna2);
	std::ofstream flagFile(flagFilename.c_str(), std::ofstream::binary | std::ofstream::trunc);
	
	for(size_t x=0;x<width;++x)
	{
		bool flagBuffer[bufferSize];
		
		size_t flagBufferPtr = 0;
		for(size_t f=0;f<FrequencyCount();++f) {
			for(size_t p=0;p<PolarizationCount();++p)
			{
				flagBuffer[flagBufferPtr] = flags[p]->Value(x, f);
				++flagBufferPtr;
			}
		}

		flagFile.write(reinterpret_cast<char*>(flagBuffer), bufferSize * sizeof(bool));
		if(flagFile.bad())
			throw std::runtime_error("Error: failed to update temporary flag files! Check access rights and free disk space.");
	}
	
	_reorderedFlagFilesHaveChanged = true;
}

template<bool UpdateData, bool UpdateFlags>
void IndirectBaselineReader::updateOriginalMS()
{
	casa::Table &table = *Table();

	casa::ROScalarColumn<double> timeColumn(*Table(), "TIME");
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");
	casa::ArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ArrayColumn<casa::Complex> *dataColumn = new casa::ArrayColumn<casa::Complex>(table, DataColumnName());

	int rowCount = table.nrow();

	std::vector<std::pair<size_t,size_t> > baselines;
	Set().GetBaselines(baselines);
	size_t
		antennaCount = Set().AntennaCount(),
		frequencyCount = FrequencyCount(),
		polarizationCount = PolarizationCount();

	// Initialize data buffer and files for reading
	size_t bufferSize = _maxMemoryUse / (baselines.size() * frequencyCount * polarizationCount * (sizeof(float) * 2 + sizeof(bool)));
	std::vector<std::vector<float *> > dataBuffers;
	std::vector<std::vector<std::ifstream *> > dataFiles;
	std::vector<std::vector<bool *> > flagBuffers;
	std::vector<std::vector<std::ifstream *> > flagFiles;
	for(size_t i=0;i<antennaCount;++i) {
		if(UpdateData)
		{
			dataBuffers.push_back(std::vector<float *>());
			dataFiles.push_back(std::vector<std::ifstream *>());
			for(size_t j=0;j<antennaCount;++j) {
				dataBuffers[i].push_back(0);
				dataFiles[i].push_back(0);
			}
		}
		if(UpdateFlags)
		{
			flagBuffers.push_back(std::vector<bool *>());
			flagFiles.push_back(std::vector<std::ifstream *>());
			for(size_t j=0;j<antennaCount;++j) {
				flagBuffers[i].push_back(0);
				flagFiles[i].push_back(0);
			}
		}
	}

	AOLogger::Debug << "Requesting " << (sizeof(float)*2+sizeof(bool)) << " x " << baselines.size() << " x " << bufferSize << " x " << polarizationCount << " x " << frequencyCount << " bytes of data\n";
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		if(UpdateData)
		{
			const std::string dataFilename = DataFilename(i->first, i->second);
			dataBuffers[i->first][i->second] = new float[2 * bufferSize * frequencyCount * polarizationCount];
			dataFiles[i->first][i->second] = new std::ifstream(dataFilename.c_str(), std::ifstream::binary);
		}
		if(UpdateFlags)
		{
			const std::string flagFilename = FlagFilename(i->first, i->second);
			flagBuffers[i->first][i->second] = new bool[bufferSize * frequencyCount * polarizationCount];
			flagFiles[i->first][i->second] = new std::ifstream(flagFilename.c_str(), std::ifstream::binary);
		}
	}

	// read first chunk
	AOLogger::Debug << 'R';
	AOLogger::Debug.Flush();
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		const size_t sampleCount = bufferSize * frequencyCount * polarizationCount;
		if(UpdateData)
		{
			float *dataBuffer = dataBuffers[i->first][i->second];
			std::ifstream *dataFile = dataFiles[i->first][i->second];
			if(dataFile->fail())
				throw std::runtime_error("Error: failed to read temporary data files!");
			dataFile->read(reinterpret_cast<char*>(dataBuffer), sampleCount * 2 * sizeof(float));
		}
		if(UpdateFlags)
		{
			bool *flagBuffer = flagBuffers[i->first][i->second];
			std::ifstream *flagFile = flagFiles[i->first][i->second];
			if(flagFile->fail())
				throw std::runtime_error("Error: failed to read temporary data files!");
			flagFile->read(reinterpret_cast<char*>(flagBuffer), sampleCount * sizeof(bool));
		}
	}
	
	AOLogger::Debug << 'W';
	AOLogger::Debug.Flush();
	
	size_t
		prevTimeIndex = (size_t) (-1),
		timeIndex = 0,
		currentBufferBlockPtr = (size_t) (-1);
	double prevTime = -1.0;
	
	// This loop writes the chunks back to the MS and then reads the next chunks from the reordered file.
	for(int rowIndex = 0;rowIndex < rowCount;++rowIndex)
	{
		// We only read entire time steps at a time, so as long as time does not change, don't check if buffer is empty
		double time = timeColumn(rowIndex);
		if(time != prevTime)
		{
			// This row has a different time value, so search it up in the index table and do sanity check
			timeIndex = AllObservationTimes().find(time)->second;
			if(timeIndex != prevTimeIndex+1)
			{
				std::stringstream s;
				s << "Error: time step " << prevTimeIndex << " is followed by time step " << timeIndex;
				throw std::runtime_error(s.str());
			}
			prevTime = time;
			prevTimeIndex = timeIndex;
			++currentBufferBlockPtr;

			if(currentBufferBlockPtr >= bufferSize)
			{
				// entire buffer was written to MS, read next chunk from reordered files
				AOLogger::Debug << 'R';
				AOLogger::Debug.Flush();
				for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
				{
					const size_t sampleCount = bufferSize * frequencyCount * polarizationCount;
					if(UpdateData)
					{
						float *dataBuffer = dataBuffers[i->first][i->second];
						std::ifstream *dataFile = dataFiles[i->first][i->second];
						if(dataFile->fail())
							throw std::runtime_error("Error: failed to read temporary data files!");
						dataFile->read(reinterpret_cast<char*>(dataBuffer), sampleCount * 2 * sizeof(float));
					}
					if(UpdateFlags)
					{
						bool *flagBuffer = flagBuffers[i->first][i->second];
						std::ifstream *flagFile = flagFiles[i->first][i->second];
						if(flagFile->fail())
							throw std::runtime_error("Error: failed to read temporary flag files!");
						flagFile->read(reinterpret_cast<char*>(flagBuffer), sampleCount * sizeof(bool));
					}
				}

				AOLogger::Debug << 'W';
				AOLogger::Debug.Flush();
				currentBufferBlockPtr = 0;
			}
		}
		
		// Write the current row
		if(UpdateData)
		{
			casa::Array<casa::Complex> data = (*dataColumn)(rowIndex);
			casa::Array<casa::Complex>::iterator dataIter=data.begin();
			size_t dataBufferPtr = currentBufferBlockPtr*2*frequencyCount*polarizationCount;
			const float *dataBuffer = dataBuffers[antenna1Column(rowIndex)][antenna2Column(rowIndex)];
			for(size_t f=0;f<frequencyCount;++f) {
				for(size_t p=0;p<polarizationCount;++p) {
					(*dataIter) = casa::Complex(dataBuffer[dataBufferPtr], dataBuffer[dataBufferPtr+1]);
					++dataIter;
					dataBufferPtr += 2;
				}
			}
			dataColumn->basePut(rowIndex, data);
		}
		if(UpdateFlags)
		{
			casa::Array<bool> flagArray = flagColumn(rowIndex);
			casa::Array<bool>::iterator flagIter=flagArray.begin();
			size_t flagBufferPtr = currentBufferBlockPtr*frequencyCount*polarizationCount;
			const bool *flagBuffer = flagBuffers[antenna1Column(rowIndex)][antenna2Column(rowIndex)];
			for(size_t f=0;f<frequencyCount;++f) {
				for(size_t p=0;p<polarizationCount;++p) {
					(*flagIter) = flagBuffer[flagBufferPtr];
					++flagIter;
					++flagBufferPtr;
				}
			}
			flagColumn.basePut(rowIndex, flagArray);
		}
	}
	
	AOLogger::Debug << "\nFreeing the data\n";
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		if(UpdateData)
		{
			delete[] dataBuffers[i->first][i->second];
			delete dataFiles[i->first][i->second];
		}
		if(UpdateFlags)
		{
			delete[] flagBuffers[i->first][i->second];
			delete flagFiles[i->first][i->second];
		}
	}

	delete dataColumn;

	clearTableCaches();

	if(UpdateData)
		AOLogger::Debug << "Done updating measurement set data\n";
	if(UpdateFlags)
		AOLogger::Debug << "Done updating measurement set flags\n";
}

void IndirectBaselineReader::updateOriginalMSData()
{
	AOLogger::Debug << "Data was changed, need to update the original MS...\n";
	updateOriginalMS<true, false>();
	_reorderedDataFilesHaveChanged = false;
}

void IndirectBaselineReader::updateOriginalMSFlags()
{
	Stopwatch watch(true);
	AOLogger::Debug << "Flags were changed, need to update the original MS...\n";
	updateOriginalMS<false, true>();
	_reorderedFlagFilesHaveChanged = false;
	AOLogger::Debug << "Storing flags toke: " << watch.ToString() << '\n';
}
