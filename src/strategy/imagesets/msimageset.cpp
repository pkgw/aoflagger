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
#include <sstream>
#include <stdexcept>

#include "../algorithms/timefrequencystatistics.h"

#include "msimageset.h"

#include "../../msio/directbaselinereader.h"
#include "../../msio/indirectbaselinereader.h"
#include "../../msio/memorybaselinereader.h"

#include "../../util/aologger.h"

namespace rfiStrategy {

	void MSImageSet::Initialize()
	{
		AOLogger::Debug << "Initializing image set...\n";
		AOLogger::Debug << "Antenna's: " << _set.AntennaCount() << '\n';
		_set.GetBaselines(_baselines);
		AOLogger::Debug << "Unique baselines: " << _baselines.size() << '\n';
		initReader();
		_reader->PartInfo(_maxScanCounts-_scanCountPartOverlap, _timeScanCount, _partCount);
		AOLogger::Debug << "Unique time stamps: " << _timeScanCount << '\n';
		_bandCount = _set.MaxSpectralBandIndex()+1;
		AOLogger::Debug << "Bands: " << _bandCount << '\n';
		AOLogger::Debug << "Number of parts: " << _partCount << '\n';
	}
	
	void MSImageSetIndex::Previous()
	{
		if(_partIndex > 0)
			--_partIndex;
		else {
			_partIndex = static_cast<class MSImageSet&>(imageSet()).PartCount() - 1;

			if(_baselineIndex > 0)
				--_baselineIndex;
			else {
				_baselineIndex = static_cast<class MSImageSet&>(imageSet()).Baselines().size() - 1;
				LargeStepPrevious();
			}
		}
	}
	
	void MSImageSetIndex::Next()
	{
		++_partIndex;
		if(_partIndex >= static_cast<class MSImageSet&>(imageSet()).PartCount() )
		{
			_partIndex = 0;

			++_baselineIndex;
			if( _baselineIndex >= static_cast<class MSImageSet&>(imageSet()).Baselines().size() )
			{
				_baselineIndex = 0;
				LargeStepNext();
			}
		}
	}
	
	void MSImageSetIndex::LargeStepPrevious()
	{
		if(_band > 0)
			--_band;
		else {
			_band = static_cast<class MSImageSet&>(imageSet()).BandCount() - 1;
			_isValid = false;
		}
	}
	
	void MSImageSetIndex::LargeStepNext()
	{
		++_band;
		if(_band >= static_cast<class MSImageSet&>(imageSet()).BandCount())
		{
			_band = 0;
			_isValid = false;
		}
	}

	void MSImageSet::initReader()
	{
		if(_reader == 0 )
		{
			switch(_ioMode)
			{
				case IndirectReadMode: {
					IndirectBaselineReader *indirectReader = new IndirectBaselineReader(_msFile);
					indirectReader->SetReadUVW(_readUVW);
					_reader = BaselineReaderPtr(indirectReader);
				} break;
				case DirectReadMode:
					_reader = BaselineReaderPtr(new DirectBaselineReader(_msFile));
					break;
				case MemoryReadMode:
					_reader = BaselineReaderPtr(new MemoryBaselineReader(_msFile));
					break;
				case AutoReadMode:
					if(MemoryBaselineReader::IsEnoughMemoryAvailable(_msFile))
						_reader = BaselineReaderPtr(new MemoryBaselineReader(_msFile));
					else
						_reader = BaselineReaderPtr(new DirectBaselineReader(_msFile));
					break;
			}
		}
		_reader->SetDataColumnName(_dataColumnName);
		_reader->SetSubtractModel(_subtractModel);
		_reader->SetReadFlags(_readFlags);
		_reader->SetReadData(true);
	}

	size_t MSImageSet::StartIndex(const MSImageSetIndex &index)
	{
		size_t startIndex =
			(_timeScanCount * index._partIndex) / _partCount - LeftBorder(index);
		return startIndex;
	}

	size_t MSImageSet::EndIndex(const MSImageSetIndex &index)
	{
		size_t endIndex =
			(_timeScanCount * (index._partIndex+1)) / _partCount + RightBorder(index);
		return endIndex;
	}

	size_t MSImageSet::LeftBorder(const MSImageSetIndex &index)
	{
		if(index._partIndex > 0)
			return _scanCountPartOverlap/2;
		else
			return 0;
	}

	size_t MSImageSet::RightBorder(const MSImageSetIndex &index)
	{
		if(index._partIndex + 1 < _partCount)
			return _scanCountPartOverlap/2 + _scanCountPartOverlap%2;
		else
			return 0;
	}

	class TimeFrequencyData *MSImageSet::LoadData(const ImageSetIndex &index)
	{
		const MSImageSetIndex &msIndex = static_cast<const MSImageSetIndex&>(index);
		initReader();
		size_t a1 = _baselines[msIndex._baselineIndex].first;
		size_t a2 = _baselines[msIndex._baselineIndex].second;
		size_t
			startIndex = StartIndex(msIndex),
			endIndex = EndIndex(msIndex);
		AOLogger::Debug << "Loading baseline " << a1 << "x" << a2 << ", t=" << startIndex << "-" << endIndex << '\n';
		_reader->AddReadRequest(a1, a2, msIndex._band, startIndex, endIndex);
		_reader->PerformReadRequests();
		std::vector<UVW> uvw;
		TimeFrequencyData data = _reader->GetNextResult(uvw);
		return new TimeFrequencyData(data);
	}

	TimeFrequencyMetaDataCPtr MSImageSet::createMetaData(const ImageSetIndex &index, std::vector<UVW> &uvw)
	{
		const MSImageSetIndex &msIndex = static_cast<const MSImageSetIndex&>(index);
		TimeFrequencyMetaData *metaData = new TimeFrequencyMetaData();
		metaData->SetAntenna1(_set.GetAntennaInfo(GetAntenna1(msIndex)));
		metaData->SetAntenna2(_set.GetAntennaInfo(GetAntenna2(msIndex)));
		metaData->SetBand(_set.GetBandInfo(msIndex._band));
		metaData->SetField(_set.GetFieldInfo(msIndex._field));
		metaData->SetObservationTimes(ObservationTimesVector(msIndex));
		if(_reader != 0)
		{
			metaData->SetUVW(uvw);
		}
		return TimeFrequencyMetaDataCPtr(metaData);
	}

	std::string MSImageSetIndex::Description() const
	{
		std::stringstream sstream;
		size_t
			antenna1 = static_cast<class MSImageSet&>(imageSet()).Baselines()[_baselineIndex].first,
			antenna2 = static_cast<class MSImageSet&>(imageSet()).Baselines()[_baselineIndex].second;
		AntennaInfo info1 = static_cast<class MSImageSet&>(imageSet()).GetAntennaInfo(antenna1);
		AntennaInfo info2 = static_cast<class MSImageSet&>(imageSet()).GetAntennaInfo(antenna2);
		BandInfo bandInfo = static_cast<class MSImageSet&>(imageSet()).GetBandInfo(_band);
		double bandStart = round(bandInfo.channels.front().frequencyHz/100000.0)/10.0;
		double bandEnd = round(bandInfo.channels.back().frequencyHz/100000.0)/10.0;
		sstream
			<< info1.station << ' ' << info1.name << " x " << info2.station << ' ' << info2.name;
		if(static_cast<class MSImageSet&>(imageSet()).BandCount() > 1)
		{
			sstream
				<< ", spect window " << _band << " (" << bandStart
				<< "MHz -" << bandEnd << "MHz)";
		}
		return sstream.str();
	}

	size_t MSImageSet::FindBaselineIndex(size_t a1, size_t a2)
	{
		size_t index = 0;
		for(std::vector<std::pair<size_t,size_t> >::const_iterator i=_baselines.begin();
			i != _baselines.end() ; ++i)
		{
			if((i->first == a1 && i->second == a2) || (i->first == a2 && i->second == a1))
			{
				return index;
			}
			++index;
		}
		throw BadUsageException("Baseline not found");
	}

	void MSImageSet::WriteFlags(const ImageSetIndex &index, TimeFrequencyData &data)
	{
		ImageSet::AddWriteFlagsTask(index, data);
		_reader->PerformFlagWriteRequests();
	}

	void MSImageSet::AddReadRequest(const ImageSetIndex &index)
	{
		BaselineData newRequest(index);
		_baselineData.push_back(newRequest);
	}
	
	void MSImageSet::PerformReadRequests()
	{
		for(std::vector<BaselineData>::iterator i=_baselineData.begin();i!=_baselineData.end();++i)
		{
			MSImageSetIndex &index = static_cast<MSImageSetIndex&>(i->Index());
			_reader->AddReadRequest(GetAntenna1(index), GetAntenna2(index), index._band, StartIndex(index), EndIndex(index));
		}
		
		_reader->PerformReadRequests();
		
		for(std::vector<BaselineData>::iterator i=_baselineData.begin();i!=_baselineData.end();++i)
		{
			if(!i->Data().IsEmpty())
				throw std::runtime_error("ReadRequest() called, but a previous read request was not completely processed by calling GetNextRequested().");
			std::vector<UVW> uvw;
			TimeFrequencyData data = _reader->GetNextResult(uvw);
			i->SetData(data);
			TimeFrequencyMetaDataCPtr metaData = createMetaData(i->Index(), uvw);
			i->SetMetaData(metaData);
		}
	}
	
	BaselineData *MSImageSet::GetNextRequested()
	{
		BaselineData top = _baselineData.front();
		_baselineData.erase(_baselineData.begin());
		if(top.Data().IsEmpty())
			throw std::runtime_error("Calling GetNextRequested(), but requests were not read with LoadRequests.");
		return new BaselineData(top);
	}
	
	void MSImageSet::AddWriteFlagsTask(const ImageSetIndex &index, std::vector<Mask2DCPtr> &flags)
	{
		const MSImageSetIndex &msIndex = static_cast<const MSImageSetIndex&>(index);
		initReader();
		size_t a1 = _baselines[msIndex._baselineIndex].first;
		size_t a2 = _baselines[msIndex._baselineIndex].second;
		size_t b = msIndex._band;
		size_t
			startIndex = StartIndex(msIndex),
			endIndex = EndIndex(msIndex);

		/*double ratio = 0.0;
		for(std::vector<Mask2DCPtr>::const_iterator i=flags.begin();i!=flags.end();++i)
		{
			ratio += ((double) (*i)->GetCount<true>() / ((*i)->Width() * (*i)->Height() * flags.size()));
		}*/
			
		std::vector<Mask2DCPtr> allFlags;
		if(flags.size() > _reader->PolarizationCount())
			throw std::runtime_error("Trying to write more polarizations to image set than available");
		else if(flags.size() < _reader->PolarizationCount())
		{
			if(flags.size() == 1)
				for(size_t i=0;i<_reader->PolarizationCount();++i)
					allFlags.push_back(flags[0]);
			else
				throw std::runtime_error("Incorrect number of polarizations in write action");
		}
		else allFlags = flags;
		
		//const AntennaInfo
		//	a1Info = GetAntennaInfo(a1),
		//	a2Info = GetAntennaInfo(a2);
		//AOLogger::Info << "Baseline " << a1Info.name << " x " << a2Info.name << " has " << TimeFrequencyStatistics::FormatRatio(ratio) << " of bad data.\n";
	
		_reader->AddWriteTask(allFlags, a1, a2, b, startIndex, endIndex, LeftBorder(msIndex), RightBorder(msIndex));
	}
	
	void MSImageSet::PerformWriteFlagsTask()
	{
		_reader->PerformFlagWriteRequests();
	}

}
