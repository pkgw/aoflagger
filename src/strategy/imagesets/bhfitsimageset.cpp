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
#include "bhfitsimageset.h"

#include "../../msio/fitsfile.h"
#include "../../msio/image2d.h"
#include "../../msio/timefrequencydata.h"

#include "../../util/aologger.h"

namespace rfiStrategy {
	
	BHFitsImageSet::BHFitsImageSet(const std::string &file) :
		ImageSet(),
		_file(new FitsFile(file))
	{
		_file->Open(FitsFile::ReadWriteMode);
	}
	
	BHFitsImageSet::BHFitsImageSet(const BHFitsImageSet& source) :
		ImageSet(),
		_file(source._file),
		_baselineData(source._baselineData),
		_timeRanges(source._timeRanges),
		_width(source._width),
		_height(source._height)
	{
	}
	
	BHFitsImageSet::~BHFitsImageSet()
	{
	}
	
	BHFitsImageSet *BHFitsImageSet::Copy()
	{
		return new BHFitsImageSet(*this);
	}

	void BHFitsImageSet::Initialize()
	{
	  _file->MoveToHDU(1);
	  /*for(int i=1;i<=_file->GetKeywordCount();++i)
	    {
	      AOLogger::Debug << _file->GetKeyword(i) << " = " << _file->GetKeywordValue(i) << '\n';
	    }*/
	  if(_file->GetCurrentHDUType() != FitsFile::ImageHDUType)
	    throw std::runtime_error("Error in Bighorns fits files: first HDU was not an image HDU");
	  if(_file->GetCurrentImageDimensionCount() != 2)
	    throw std::runtime_error("Fits image was not two dimensional");
	  _width =_file->GetCurrentImageSize(2), _height = _file->GetCurrentImageSize(1);
	  AOLogger::Debug << "Image of " << _width << " x " << _height << '\n';

	  _timeRanges.clear();
	  size_t keyIndex = 0;
	  bool searchOn;
	  do {
	    searchOn = false;
	    std::ostringstream antKey, termKey;
	    antKey << "ANT";
	    termKey << "TERM";
	    if(keyIndex < 10) {
	      antKey << '0';
	      termKey << '0';
	    }
	    antKey << keyIndex;
	    termKey << keyIndex;
	    std::string antRangeStr, termRangeStr;
	    if(_file->GetKeywordValue(antKey.str(), antRangeStr)) {
	      std::pair<int, int> range = getRangeFromString(antRangeStr);
	      TimeRange timeRange;
	      timeRange.start = range.first;
	      timeRange.end = range.second;
	      timeRange.name = antKey.str();
	      _timeRanges.push_back(timeRange);
	      searchOn = true;
	    }
	    if(_file->GetKeywordValue(termKey.str(), termRangeStr)) {
	      std::pair<int, int> range = getRangeFromString(termRangeStr);
	      TimeRange timeRange;
	      timeRange.start = range.first;
	      timeRange.end = range.second;
	      timeRange.name = termKey.str();
	      _timeRanges.push_back(timeRange);
	      searchOn = true;
	    }
	    ++keyIndex;
	  } while(searchOn);
	  AOLogger::Debug << "This file has " << _timeRanges.size() << " time ranges.\n";
	}

	BaselineData BHFitsImageSet::loadData(const ImageSetIndex &index)
	{
	  const BHFitsImageSetIndex &fitsIndex = static_cast<const BHFitsImageSetIndex&>(index);

	  TimeFrequencyMetaDataPtr metaData(new TimeFrequencyMetaData());
	  TimeFrequencyData data;
	  loadImageData(data, metaData, fitsIndex);
	  return BaselineData(data, metaData, index);
	}

  void BHFitsImageSet::loadImageData(TimeFrequencyData &data, const TimeFrequencyMetaDataPtr &metaData, const BHFitsImageSetIndex &index)
  {
		std::vector<num_t> buffer(_width * _height);
		_file->ReadCurrentImageData(0, &buffer[0], _width * _height);
		
		int
			rangeStart = _timeRanges[index._imageIndex].start, 
			rangeEnd = _timeRanges[index._imageIndex].end;
		Image2DPtr image = Image2D::CreateZeroImagePtr(rangeEnd-rangeStart, _height);

		std::vector<num_t>::const_iterator bufferPtr = buffer.begin() + _height*rangeStart;
		for(int x=rangeStart; x!=rangeEnd; ++x)
		{
			for(int y=0; y!=_height; ++y)
			{
				image->SetValue(x-rangeStart, y, *bufferPtr);
				++bufferPtr;
			}
		}
		data = TimeFrequencyData(TimeFrequencyData::AmplitudePart, SinglePolarisation, image);

		try {
			FitsFile flagFile(flagFilePath());
			flagFile.Open(FitsFile::ReadOnlyMode);
			flagFile.ReadCurrentImageData(0, &buffer[0], _width * _height);
			bufferPtr = buffer.begin() + _height*rangeStart;
			Mask2DPtr mask = Mask2D::CreateUnsetMaskPtr(rangeEnd-rangeStart, _height);
			for(int x=rangeStart; x!=rangeEnd; ++x)
			{
				for(int y=0; y!=_height; ++y)
				{
					mask->SetValue(x-rangeStart, y, *bufferPtr == 1.0);
					++bufferPtr;
				}
			}
			data.SetGlobalMask(mask);
		} catch(std::exception &)
		{
			// Flag file could not be read; probably does not exist. Ignore this, flags will be initialized to false.
		}

		double
			frequencyDelta = _file->GetDoubleKeywordValue("CDELT1"),
			timeDelta = _file->GetDoubleKeywordValue("CDELT2");
		BandInfo band;
		for(int ch=0; ch!=_height; ++ch)
		{
			ChannelInfo channel;
			channel.frequencyHz = ch * frequencyDelta * 1000000.0;
			band.channels.push_back(channel);
		}
		metaData->SetBand(band);

		const int rangeWidth = rangeEnd-rangeStart;
		std::vector<double> observationTimes(rangeWidth);
		for(int t=0; t!=rangeWidth; ++t)
			observationTimes[t] = (t + rangeStart) * timeDelta;
		metaData->SetObservationTimes(observationTimes);

		AntennaInfo antennaInfo;
		antennaInfo.id = 0;
		antennaInfo.name = RangeName(index._imageIndex);
		antennaInfo.diameter = 0.0;
		antennaInfo.mount = "Unknown";
		antennaInfo.station = GetTelescopeName();
		metaData->SetAntenna1(antennaInfo);
		metaData->SetAntenna2(antennaInfo);
  }

  std::pair<int, int> BHFitsImageSet::getRangeFromString(const std::string &rangeStr)
  {
    std::pair<int, int> value;
    size_t partA = rangeStr.find(' ');
    value.first = atoi(rangeStr.substr(0, partA).c_str());
    size_t partB = rangeStr.find('-');
    if(rangeStr[partB+1] == ' ')
      ++partB;
    value.second = atoi(rangeStr.substr(partB+1).c_str());
    return value;
  }
  
	std::string BHFitsImageSet::flagFilePath() const
	{
    std::string flagFilePath = _file->Filename();
    if(flagFilePath.size() > 7) {
      flagFilePath = flagFilePath.substr(0, flagFilePath.size()-7);
    }
    flagFilePath += "_flag.fits";
		return flagFilePath;
	}

  void BHFitsImageSet::AddWriteFlagsTask(const ImageSetIndex &index, std::vector<Mask2DCPtr> &flags)
  {
		if(flags.size() != 1)
			throw std::runtime_error("BHFitsImageSet::AddWriteFlagsTask() called with multiple flags");
		std::string flagFilename = flagFilePath();
		AOLogger::Debug << "Writing to " << flagFilename << '\n';
		FitsFile flagFile(flagFilename);
		bool newFile = true;
		std::vector<num_t> buffer(_width * _height);
		try {
			flagFile.Open(FitsFile::ReadWriteMode);
			newFile = false;
		} catch(std::exception &) {
			AOLogger::Debug << "File did not exist yet, creating new.\n";
			flagFile.Create();
			flagFile.AppendImageHUD(FitsFile::Float32ImageType, _height, _width);
		}

		// This must be outside the try { } block, so that exceptions
		// don't result in creating a new file.
		if(!newFile) {
			flagFile.ReadCurrentImageData(0, &buffer[0], _width * _height);
		}

		const BHFitsImageSetIndex &bhIndex(static_cast<const BHFitsImageSetIndex&>(index));
		int
			rangeStart = _timeRanges[bhIndex._imageIndex].start, 
			rangeEnd = _timeRanges[bhIndex._imageIndex].end;
		std::vector<num_t>::iterator bufferPtr = buffer.begin() + _height*rangeStart;
		for(int x=rangeStart; x!=rangeEnd; ++x)
		{
		for(int y=0; y!=_height; ++y)
			{
				*bufferPtr = flags[0]->Value(x-rangeStart, y) ? 1.0 : 0.0;
				++bufferPtr;
			}
		}

		flagFile.WriteImage(0, &buffer[0], _width * _height);
	}

	void BHFitsImageSet::PerformWriteFlagsTask()
	{
	  // Nothing to do; already written
	}
	
	
	void BHFitsImageSetIndex::Previous()
	{
	  if(_imageIndex > 0)
			--_imageIndex;
		else {
			_imageIndex = static_cast<class BHFitsImageSet&>(imageSet()).ImageCount() - 1;
			LargeStepPrevious();
		}
	}
	
	void BHFitsImageSetIndex::Next()
	{
		++_imageIndex;
		if( _imageIndex >= static_cast<class BHFitsImageSet&>(imageSet()).ImageCount() )
		{
		  _imageIndex = 0;
		  LargeStepNext();
		}
	}

	void BHFitsImageSetIndex::LargeStepPrevious()
	{
	  _isValid = false;
	}
	
	void BHFitsImageSetIndex::LargeStepNext()
	{
	  _isValid = false;
	}

	std::string BHFitsImageSetIndex::Description() const {
	  std::ostringstream str;
	  str << "Time range " << 
	    static_cast<BHFitsImageSet&>(imageSet()).RangeName(_imageIndex);
	  return str.str();
	}

	std::string BHFitsImageSet::File()
	{
	  return _file->Filename();
	}
}
