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

#include "memorybaselinereader.h"
#include "system.h"

#include "../util/aologger.h"
#include "../util/stopwatch.h"

#include <ms/MeasurementSets/MeasurementSet.h>

#include <vector>

using namespace casa;

void MemoryBaselineReader::PerformReadRequests()
{
	readSet();
	
	for(size_t i=0;i!=_readRequests.size();++i)
	{
		const ReadRequest &request = _readRequests[i];
		BaselineID id;
		id.antenna1 = request.antenna1;
		id.antenna2 = request.antenna2;
		id.spw = request.spectralWindow;
		_results.push_back(_baselines.find(id)->second);
	}
	
	_readRequests.clear();
}

void MemoryBaselineReader::readSet()
{
	if(!_isRead)
	{
		Stopwatch watch(true);
		
		initialize();
	
		casa::Table &table = *Table();
		
		ROScalarColumn<int>
			ant1Column(table, casa::MeasurementSet::columnName(MSMainEnums::ANTENNA1)),
			ant2Column(table, casa::MeasurementSet::columnName(MSMainEnums::ANTENNA2)),
			spwColumn(table, casa::MeasurementSet::columnName(MSMainEnums::DATA_DESC_ID));
		ROScalarColumn<double>
			timeColumn(table, casa::MeasurementSet::columnName(MSMainEnums::TIME));
		ROArrayColumn<casa::Complex>
			dataColumn(table, DataColumnName());
		ROArrayColumn<bool>
			flagColumn(table, casa::MeasurementSet::columnName(MSMainEnums::FLAG));
		ROArrayColumn<double>
			uvwColumn(table, casa::MeasurementSet::columnName(MSMainEnums::UVW));
		const std::map<double, size_t>
			&observationTimes = AllObservationTimes();
		
		size_t
			antennaCount = Set().AntennaCount(),
			frequencyCount = FrequencyCount(),
			polarizationCount = PolarizationCount(),
			timeStepCount = observationTimes.size();
			
		if(Set().BandCount() != 1)
			throw std::runtime_error("Can not handle measurement sets with more than 1 band.");
		
		// Initialize the look-up matrix
		// to quickly access the elements (without the map-lookup)
		typedef Result* MatrixElement;
		typedef std::vector<MatrixElement> MatrixRow;
		typedef std::vector<MatrixRow> Matrix;
		Matrix matrix(antennaCount);
		
		AOLogger::Debug << "Claiming memory for memory baseline reader...\n";
		
		BandInfo band = Set().GetBandInfo(0);
		for(size_t a1=0;a1!=antennaCount;++a1)
		{
			matrix[a1].resize(antennaCount);
			for(size_t a2=0;a2!=antennaCount;++a2)
				matrix[a1][a2] = 0;
		}
		
		// The actual reading of the data
		AOLogger::Debug << "Reading the data...\n";
		
		IPosition dataShape = IPosition(2);
		dataShape[0] = polarizationCount;
		dataShape[1] = frequencyCount;
		
		double prevTime = -1.0;
		unsigned rowCount = table.nrow();
		size_t timeIndex = 0, prevTimeIndex = (size_t) (-1);
		casa::Array<casa::Complex> dataArray(dataShape);
		casa::Array<bool> flagArray(dataShape);
		for(unsigned rowIndex = 0;rowIndex < rowCount;++rowIndex)
		{
			double time = timeColumn(rowIndex);
			if(time != prevTime)
			{
				timeIndex = observationTimes.find(time)->second;
				if(timeIndex != prevTimeIndex+1)
				{
					// sanity check failed -- never seen this happen in a ms, but just for sure.
					std::stringstream s;
					s << "Error: time step " << prevTimeIndex << " is followed by time step " << timeIndex;
					throw std::runtime_error(s.str());
				}
				prevTime = time;
				prevTimeIndex = timeIndex;
			}
			
			size_t ant1 = ant1Column(rowIndex);
			size_t ant2 = ant2Column(rowIndex);
			if(ant1 > ant2) std::swap(ant1, ant2);
			
			Result *result = matrix[ant1][ant2];
			if(result == 0)
			{
				result = new Result();
				for(size_t p=0;p!=polarizationCount;++p) {
					result->_realImages.push_back(Image2D::CreateZeroImagePtr(timeStepCount, frequencyCount));
					result->_imaginaryImages.push_back(Image2D::CreateZeroImagePtr(timeStepCount, frequencyCount));
					result->_flags.push_back(Mask2D::CreateSetMaskPtr<true>(timeStepCount, frequencyCount));
				}
				result->_bandInfo = band;
				result->_uvw.resize(timeStepCount);
				matrix[ant1][ant2] = result;
			}
			
			dataColumn.get(rowIndex, dataArray);
			flagColumn.get(rowIndex, flagArray);
			
			Array<double> uvwArray = uvwColumn.get(rowIndex);
			Array<double>::const_iterator uvwPtr = uvwArray.begin();
			UVW uvw;
			uvw.u = *uvwPtr; ++uvwPtr;
			uvw.v = *uvwPtr; ++uvwPtr;
			uvw.w = *uvwPtr;
			result->_uvw[timeIndex] = uvw;
			
			for(size_t p=0;p!=polarizationCount;++p)
			{
				Array<Complex>::const_iterator dataPtr = dataArray.begin();
				Array<bool>::const_iterator flagPtr = flagArray.begin();
			
				Image2D *real = &*result->_realImages[p];
				Image2D *imag = &*result->_imaginaryImages[p];
				Mask2D *mask = &*result->_flags[p];
				const size_t imgStride = real->Stride();
				const size_t mskStride = mask->Stride();
				num_t *realOutPtr = real->ValuePtr(timeIndex, 0);
				num_t *imagOutPtr = imag->ValuePtr(timeIndex, 0);
				bool *flagOutPtr = mask->ValuePtr(timeIndex, 0);
				
				for(size_t i=0;i!=p;++i) {
					++dataPtr;
					++flagPtr;
				}
					
				for(size_t ch=0;ch!=frequencyCount;++ch)
				{
					*realOutPtr = dataPtr->real();
					*imagOutPtr = dataPtr->imag();
					*flagOutPtr = *flagPtr;
					
					realOutPtr += imgStride;
					imagOutPtr += imgStride;
					flagOutPtr += mskStride;
					
					for(size_t i=0;i!=polarizationCount;++i) {
						++dataPtr;
						++flagPtr;
					}
				}
			}
		}
		
		// Store elements in matrix to the baseline map.
		for(size_t a1=0;a1!=antennaCount;++a1)
		{
			for(size_t a2=a1;a2!=antennaCount;++a2)
			{
				if(matrix[a1][a2] != 0)
				{
					BaselineID id;
					id.antenna1 = a1;
					id.antenna2 = a2;
					id.spw = 0;
					_baselines.insert(std::pair<BaselineID, Result>(id, *matrix[a1][a2]));
					delete matrix[a1][a2];
				}
			}
		}
		_areFlagsChanged = false;
		_isRead = true;
		
		AOLogger::Debug << "Reading toke " << watch.ToString() << ".\n";
	}
}

void MemoryBaselineReader::PerformFlagWriteRequests()
{
	readSet();
	
	for(size_t i=0;i!=_writeRequests.size();++i)
	{
		const WriteRequest &request = _writeRequests[i];
		BaselineID id;
		id.antenna1 = request.antenna1;
		id.antenna2 = request.antenna2;
		id.spw = request.spectralWindow;
		Result &result = _baselines[id];
		if(result._flags.size() != request.flags.size())
			throw std::runtime_error("Polarizations do not match");
		for(size_t p=0;p!=result._flags.size();++p)
			result._flags[p] = Mask2D::CreateCopy(request.flags[p]);
	}
	_areFlagsChanged = true;
	
	_writeRequests.clear();
}

void MemoryBaselineReader::writeFlags()
{
	casa::Table &table = *Table();
	
	ROScalarColumn<int>
		ant1Column(table, casa::MeasurementSet::columnName(MSMainEnums::ANTENNA1)),
		ant2Column(table, casa::MeasurementSet::columnName(MSMainEnums::ANTENNA2)),
		spwColumn(table, casa::MeasurementSet::columnName(MSMainEnums::DATA_DESC_ID));
	ROScalarColumn<double>
		timeColumn(table, casa::MeasurementSet::columnName(MSMainEnums::TIME));
	ArrayColumn<bool>
		flagColumn(table, casa::MeasurementSet::columnName(MSMainEnums::FLAG));
	const std::map<double, size_t>
		&observationTimes = AllObservationTimes();
	
	size_t
		frequencyCount = FrequencyCount(),
		polarizationCount = PolarizationCount();
		
	AOLogger::Debug << "Flags have changed, writing them back to the set...\n";
	
	IPosition flagShape = IPosition(2);
	flagShape[0] = polarizationCount;
	flagShape[1] = frequencyCount;
	
	double prevTime = -1.0;
	unsigned rowCount = table.nrow();
	size_t timeIndex = 0;
	casa::Array<bool> flagArray(flagShape);
	for(unsigned rowIndex = 0;rowIndex < rowCount;++rowIndex)
	{
		double time = timeColumn(rowIndex);
		if(time != prevTime)
		{
			timeIndex = observationTimes.find(time)->second;
			prevTime = time;
		}
		
		size_t ant1 = ant1Column(rowIndex);
		size_t ant2 = ant2Column(rowIndex);
		size_t spw = spwColumn(rowIndex);
		if(ant1 > ant2) std::swap(ant1, ant2);
		
		BaselineID baselineID;
		baselineID.antenna1 = ant1;
		baselineID.antenna2 = ant2;
		baselineID.spw = spw;
		Result *result = &_baselines.find(baselineID)->second;
		
		Array<bool>::iterator flagPtr = flagArray.begin();
		
		std::vector<Mask2D*> masks(polarizationCount);
		for(size_t p=0;p!=polarizationCount;++p)
			masks[p] = &*result->_flags[p];
		
		for(size_t ch=0;ch!=frequencyCount;++ch)
		{
			for(size_t p=0;p!=polarizationCount;++p)
			{
				*flagPtr = masks[p]->Value(timeIndex, ch);
				++flagPtr;
			}
		}
		
		flagColumn.put(rowIndex, flagArray);
	}
	
	_areFlagsChanged = false;
}

bool MemoryBaselineReader::IsEnoughMemoryAvailable(const std::string &filename)
{
	casa::MeasurementSet ms(filename);
	
	MSSpectralWindow spwTable = ms.spectralWindow();
	if(spwTable.nrow() != 1) throw std::runtime_error("Set should have exactly one spectral window");
	
	ROScalarColumn<int> numChanCol(spwTable, MSSpectralWindow::columnName(MSSpectralWindowEnums::NUM_CHAN));
	size_t channelCount = numChanCol.get(0);
	if(channelCount == 0) throw std::runtime_error("No channels in set");
	if(ms.nrow() == 0) throw std::runtime_error("Table has no rows (no data)");
	
	typedef float num_t;
	typedef std::complex<num_t> complex_t;
	ROScalarColumn<int> ant1Column(ms, ms.columnName(MSMainEnums::ANTENNA1));
	ROScalarColumn<int> ant2Column(ms, ms.columnName(MSMainEnums::ANTENNA2));
	ROArrayColumn<complex_t> dataColumn(ms, ms.columnName(MSMainEnums::DATA));
	
	IPosition dataShape = dataColumn.shape(0);
	unsigned polarizationCount = dataShape[0];
	
	uint64_t size =
		(uint64_t) polarizationCount * (uint64_t) channelCount *
		(uint64_t) ms.nrow() * (uint64_t) (sizeof(num_t) * 2 + sizeof(bool));
		
	uint64_t totalMem = System::TotalMemory();
	
	if(size * 2 >= totalMem)
	{
		AOLogger::Warn
			<< (size/1000000) << " MB required, but " << (totalMem/1000000) << " MB available.\n"
			"Because this is not at least twice as much, direct read mode (slower!) will be used.\n";
		return false;
	} else {
		AOLogger::Debug
			<< (size/1000000) << " MB required, " << (totalMem/1000000)
			<< " MB available: will use memory read mode.\n";
		return true;
	}
}
