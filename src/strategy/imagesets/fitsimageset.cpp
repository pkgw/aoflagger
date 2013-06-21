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
#include "fitsimageset.h"

#include <sstream>
#include <vector>

#include "../../msio/date.h"
#include "../../msio/fitsfile.h"
#include "../../msio/image2d.h"
#include "../../msio/timefrequencydata.h"

#include "../../util/aologger.h"

namespace rfiStrategy {
	
	FitsImageSet::FitsImageSet(const std::string &file) :
		ImageSet(),
		_file(new FitsFile(file)),
		_currentBaselineIndex(0),
		_frequencyOffset(0.0)
	{
		_file->Open(FitsFile::ReadWriteMode);
	}
	
	FitsImageSet::FitsImageSet(const FitsImageSet& source) :
		ImageSet(),
		_file(source._file),
		_baselines(source._baselines),
		_bandCount(source._bandCount),
		_antennaInfos(source._antennaInfos),
		_bandInfos(source._bandInfos),
		_bandIndexToNumber(source._bandIndexToNumber),
		_currentBaselineIndex(source._currentBaselineIndex),
		_currentBandIndex(source._currentBandIndex),
		_frequencyOffset(source._frequencyOffset),
		_baselineData(source._baselineData)
	{
	}
	
	FitsImageSet::~FitsImageSet()
	{
	}
	
	FitsImageSet *FitsImageSet::Copy()
	{
		return new FitsImageSet(*this);
	}

	void FitsImageSet::Initialize()
	{
		//AOLogger::Debug << "Keyword count: " << _file->GetKeywordCount() << '\n';
		if(_file->HasGroups())
		{
			AOLogger::Debug << "This file has " << _file->GetGroupCount() << " groups with " << _file->GetParameterCount() << " parameters.\n";
			//AOLogger::Debug << "Group size: " << _file->GetGroupSize() << '\n';
			_file->MoveToHDU(1);
			if(_file->GetCurrentHDUType() != FitsFile::ImageHDUType)
				throw FitsIOException("Primary table is not a grouped image");
			long double *parameters = new long double[_file->GetParameterCount()];
			int baselineIndex = _file->GetGroupParameterIndex("BASELINE");
			size_t groupCount = _file->GetGroupCount();
			std::set<std::pair<size_t,size_t> > baselineSet;
			for(size_t g=0;g<groupCount;++g)
			{
				_file->ReadGroupParameters(g, parameters);
				int a1 = ((int) parameters[baselineIndex]) & 255;
				int a2 = ((int) parameters[baselineIndex] >> 8) & 255;
				baselineSet.insert(std::pair<size_t,size_t>(a1,a2));
			}
			delete[] parameters;
			AOLogger::Debug << "Baselines in file: " << baselineSet.size() << '\n';
			for(std::set<std::pair<size_t,size_t> >::const_iterator i=baselineSet.begin();i!=baselineSet.end();++i)
				_baselines.push_back(*i);
			_bandCount = _file->GetCurrentImageSize(5);
		} else {
			// sdfits
			
			_baselines.push_back(std::pair<size_t,size_t>(0, 0));
			_antennaInfos.push_back(AntennaInfo());
			
			// find number of bands
			_file->MoveToHDU(2);
			int ifColumn = _file->GetTableColumnIndex("IF");
			int rowCount = _file->GetRowCount();
			std::set<int> ifSet;
			for(int i=1;i<=rowCount;++i)
			{
				double thisIndex;
				_file->ReadTableCell(i, ifColumn, &thisIndex, 1);
				ifSet.insert((int) round(thisIndex));
			}
			_bandCount = ifSet.size();
			if(_bandCount == 0)
				throw std::runtime_error("Could not find any IF's in this set");
			_bandIndexToNumber.clear();
			AOLogger::Debug << _bandCount << " IF's in set: [" << *ifSet.begin();
			for(std::set<int>::const_iterator i=ifSet.begin(); i!=ifSet.end(); ++i)
			{
				_bandInfos.insert(std::pair<int, BandInfo>(*i, BandInfo()));
				if(_bandIndexToNumber.size()>0)
					AOLogger::Debug << ", " << *i;
				_bandIndexToNumber.push_back(*i);
			}
			AOLogger::Debug << "]\n";
		}
	}

	BaselineData FitsImageSet::loadData(const ImageSetIndex &index)
	{
		const FitsImageSetIndex &fitsIndex = static_cast<const FitsImageSetIndex&>(index);
		_frequencyOffset = 0.0;

		_file->MoveToHDU(1);
		TimeFrequencyMetaDataPtr metaData(new TimeFrequencyMetaData());
		TimeFrequencyData data;
		if(_file->HasGroups())
			data = ReadPrimaryGroupTable(fitsIndex._baselineIndex, fitsIndex._band, 0, *metaData);
		else
			ReadPrimarySingleTable(data, *metaData);
		for(int hduIndex=2;hduIndex <= _file->GetHDUCount();hduIndex++)
		{
			_file->MoveToHDU(hduIndex);
			switch(_file->GetCurrentHDUType())
			{
				case FitsFile::BinaryTableHDUType:
					AOLogger::Debug << "Binary table found.\n";
					ReadTable(data, *metaData, fitsIndex._band);
					break;
				case FitsFile::ASCIITableHDUType:
					AOLogger::Debug << "ASCII table found.\n";
					ReadTable(data, *metaData, fitsIndex._band);
					break;
				case FitsFile::ImageHDUType:
					AOLogger::Debug << "Image found.\n";
					break;
			}
		}
		
		if(_file->HasGroups())
		{
			_currentBaselineIndex = fitsIndex._baselineIndex;
			_currentBandIndex = fitsIndex._band;
			int bandNumber = _bandIndexToNumber[fitsIndex._band];
			metaData->SetBand(_bandInfos[bandNumber]);
			AOLogger::Debug << "Loaded metadata for: " << Date::AipsMJDToString(metaData->ObservationTimes()[0]) << ", band " << bandNumber << " (" << Frequency::ToString(_bandInfos[bandNumber].channels[0].frequencyHz) << " - " << Frequency::ToString(_bandInfos[bandNumber].channels.rbegin()->frequencyHz) << ")\n";

		}
		return BaselineData(data, metaData, index);
	}

	TimeFrequencyData FitsImageSet::ReadPrimaryGroupTable(size_t baselineIndex, int band, int stokes, TimeFrequencyMetaData &metaData)
	{
		if(!_file->HasGroups() || _file->GetCurrentHDUType() != FitsFile::ImageHDUType)
			throw FitsIOException("Primary table is not a grouped image");

		std::vector<double> observationTimes;
		std::vector<UVW> uvws;

		//int keywordCount = _file->GetKeywordCount();
		//for(int i=1;i<=keywordCount;++i)
		//	AOLogger::Debug << "Keyword " << i << ": " << _file->GetKeyword(i) << "=" << _file->GetKeywordValue(i) << " ("  << _file->GetKeywordComment(i) << ")\n";

		std::vector<long double> parameters(_file->GetParameterCount());
		int baseline = _baselines[baselineIndex].first + (_baselines[baselineIndex].second<<8);
		int baselineColumn = _file->GetGroupParameterIndex("BASELINE");
		size_t
			complexCount = _file->GetCurrentImageSize(2),
			stokesStep = complexCount,
			stokesCount = _file->GetCurrentImageSize(3),
			frequencyStep = stokesCount*complexCount,
			frequencyCount = _file->GetCurrentImageSize(4),
			bandStep = frequencyStep*frequencyCount;
		std::vector<std::vector<long double> > valuesR(frequencyCount);
		std::vector<std::vector<long double> > valuesI(frequencyCount);
		std::vector<long double> data(_file->GetImageSize());
		size_t groupCount = _file->GetGroupCount();
		bool hasDate2 = _file->HasGroupParameter("DATE", 2);
		int date2Index = 0, date1Index = _file->GetGroupParameterIndex("DATE");
		if(hasDate2)
		{
			date2Index = _file->GetGroupParameterIndex("DATE", 2);
		}
		int uuIndex, vvIndex, wwIndex;
		if(_file->HasGroupParameter("UU"))
		{
			uuIndex = _file->GetGroupParameterIndex("UU");
			vvIndex = _file->GetGroupParameterIndex("VV");
			wwIndex = _file->GetGroupParameterIndex("WW");
		} else {
			uuIndex = _file->GetGroupParameterIndex("UU---SIN");
			vvIndex = _file->GetGroupParameterIndex("VV---SIN");
			wwIndex = _file->GetGroupParameterIndex("WW---SIN");
		}
		size_t match = 0;
		double frequencyFactor = 1.0;
		if(_frequencyOffset != 0.0)
			frequencyFactor = _frequencyOffset;
		for(size_t g=0;g<groupCount;++g)
		{
			_file->ReadGroupParameters(g, &parameters[0]);
			if(parameters[baselineColumn] == baseline)
			{
				double date;
				if(hasDate2)
					date = parameters[date1Index] + parameters[date2Index];
				else
					date = parameters[date1Index];
				UVW uvw;
				uvw.u = parameters[uuIndex] * frequencyFactor;
				uvw.v = parameters[vvIndex] * frequencyFactor;
				uvw.w = parameters[wwIndex] * frequencyFactor;

				_file->ReadGroupData(g, &data[0]);
				for(size_t f=0;f<frequencyCount;++f)
				{
					size_t index = stokes*stokesStep + frequencyStep*f + bandStep*band;
					long double r = data[index];
					long double i = data[index + 1];
					valuesR[f].push_back(r);
					valuesI[f].push_back(i);
				}
				observationTimes.push_back(Date::JDToAipsMJD(date));
				uvws.push_back(uvw);
				++match;
			}
		}
		AOLogger::Debug << match << " rows in table matched baseline.\n";
		data.clear();
		parameters.clear();

		AOLogger::Debug << "Image is " << valuesR[0].size() << " x " << frequencyCount << '\n';
		if(valuesR[0].size() == 0)
			throw BadUsageException("Baseline not found!");
		Image2DPtr
			real = Image2D::CreateUnsetImagePtr(valuesR[0].size(), frequencyCount),
			imaginary = Image2D::CreateUnsetImagePtr(valuesR[0].size(), frequencyCount);
		for(size_t i=0;i<valuesR[0].size();++i)
		{
			for(size_t f=0;f<frequencyCount;++f)
			{
				real->SetValue(i, f, valuesR[f][i]);
				imaginary->SetValue(i, f, valuesI[f][i]);
			}
		}
		
		metaData.SetUVW(uvws);
		metaData.SetObservationTimes(observationTimes);
		return TimeFrequencyData(StokesIPolarisation, real, imaginary);
	}

	void FitsImageSet::ReadPrimarySingleTable(TimeFrequencyData &data, TimeFrequencyMetaData &metaData)
	{
		//int keywordCount = _file->GetKeywordCount();
		//for(int i=1;i<=keywordCount;++i)
		//	AOLogger::Debug << "Keyword " << i << ": " << _file->GetKeyword(i) << "=" << _file->GetKeywordValue(i) << " ("  << _file->GetKeywordComment(i) << ")\n";
	}
	
	void FitsImageSet::ReadTable(TimeFrequencyData &data, TimeFrequencyMetaData &metaData, size_t bandIndex)
	{
		//AOLogger::Debug << "Row count: " << _file->GetRowCount() << '\n';
		//AOLogger::Debug << "Column count: " << _file->GetColumnCount() << '\n';
		//for(int i= 1;i <= _file->GetColumnCount(); ++i)
		//{
		//	AOLogger::Debug << "Column type " << i << ": " << _file->GetColumnType(i) << '\n';
		//}
		std::string extName = _file->GetKeywordValue("EXTNAME");
		//for(int i=1;i<=_file->GetKeywordCount();++i)
		//	AOLogger::Debug << "Keyword " << i << ": " << _file->GetKeyword(i) << "=" << _file->GetKeywordValue(i) << " ("  << _file->GetKeywordComment(i) << ")\n";
		if(extName == "AIPS AN")
			ReadAntennaTable(metaData);
		else if(extName == "AIPS FQ")
			ReadFrequencyTable(data, metaData);
		else if(extName == "AIPS CL")
			ReadCalibrationTable();
		else if(extName == "SINGLE DISH")
			ReadSingleDishTable(data, metaData, bandIndex);
	}
	
	void FitsImageSet::ReadAntennaTable(TimeFrequencyMetaData &metaData)
	{
		AOLogger::Debug << "Found antenna table\n";
		_frequencyOffset = _file->GetDoubleKeywordValue("FREQ");
		for(std::map<int, BandInfo>::iterator i=_bandInfos.begin();i!=_bandInfos.end();++i)
		{
			for(std::vector<ChannelInfo>::iterator j=i->second.channels.begin();j!=i->second.channels.end();++j) {
				j->frequencyHz += _frequencyOffset;
			}
		}
		std::vector<UVW> uvws(metaData.UVW());
		for(std::vector<UVW>::iterator i=uvws.begin();i!=uvws.end();++i)
		{
			i->u = i->u * _frequencyOffset;
			i->v = i->v * _frequencyOffset;
			i->w = i->w * _frequencyOffset;
		}
		metaData.SetUVW(uvws);
		_antennaInfos.clear();
		for(int i=1;i<=_file->GetRowCount();++i)
		{
			AntennaInfo info;
			char name[9];
			long double pos[3];
			_file->ReadTableCell(i, 1, name);
			_file->ReadTableCell(i, 2, pos, 3);
			info.name = name;
			info.position.x = pos[0];
			info.position.y = pos[1];
			info.position.z = pos[2];
			_antennaInfos.push_back(info);
		}
	}

	void FitsImageSet::ReadFrequencyTable(TimeFrequencyData &data, TimeFrequencyMetaData &metaData)
	{
		AOLogger::Debug << "Found frequency table\n";
		const size_t numberIfs = _file->GetIntKeywordValue("NO_IF");
		AOLogger::Debug << "Number of ifs: " << numberIfs << '\n';
		_bandInfos.clear();
		BandInfo bandInfo;
		for(int i=1;i<=_file->GetRowCount();++i)
		{
			long double freqSel;
			std::vector<long double> ifFreq(numberIfs), chWidth(numberIfs), totalBandwidth(numberIfs), sideband(numberIfs);
			_file->ReadTableCell(i, 1, &freqSel, 1);
			_file->ReadTableCell(i, 2, &ifFreq[0], numberIfs);
			_file->ReadTableCell(i, 3, &chWidth[0], numberIfs);
			_file->ReadTableCell(i, 4, &totalBandwidth[0], numberIfs);
			_file->ReadTableCell(i, 5, &sideband[0], numberIfs);
			for(size_t b=0;b<numberIfs;++b)
			{
				for(size_t channel=0;channel<data.ImageHeight();++channel)
				{
					ChannelInfo channelInfo;
					channelInfo.channelWidthHz = chWidth[b];
					channelInfo.effectiveBandWidthHz = chWidth[b];
					channelInfo.frequencyHz = _frequencyOffset + ifFreq[b] + (chWidth[b] * channel);
					channelInfo.frequencyIndex = channel;
					channelInfo.resolutionHz = chWidth[b];
					bandInfo.channels.push_back(channelInfo);
				}

				bandInfo.windowIndex = b;
				_bandInfos.insert(std::pair<int, BandInfo>(b, bandInfo));
			}
		}
	}

	void FitsImageSet::ReadCalibrationTable()
	{
		AOLogger::Debug << "Found calibration table with " << _file->GetRowCount() << " rows.\n";
	}
	
	void FitsImageSet::ReadSingleDishTable(TimeFrequencyData &data, TimeFrequencyMetaData &metaData, size_t ifIndex)
	{
		const int rowCount = _file->GetRowCount();
		AOLogger::Debug << "Found single dish table with " << rowCount << " rows.\n";
		const int
			timeColumn = _file->GetTableColumnIndex("TIME"),
			dateObsColumn = _file->GetTableColumnIndex("DATE-OBS"),
			dataColumn = _file->GetTableColumnIndex("DATA"),
			flagColumn = _file->GetTableColumnIndex("FLAGGED"),
			freqValColumn = _file->GetTableColumnIndex("CRVAL1"),
			freqRefPixColumn = _file->GetTableColumnIndex("CRPIX1"),
			freqDeltaColumn = _file->GetTableColumnIndex("CDELT1"),
			freqResColumn = _file->GetTableColumnIndex("FREQRES"),
			freqBandwidthColumn = _file->GetTableColumnIndex("BANDWID"),
			ifColumn = _file->GetTableColumnIndex("IF");
		//int
		//	beamColumn = 0;
		//const bool hasBeamColumn = _file->HasTableColumn("BEAM", beamColumn);
		const int
			freqCount = _file->GetTableDimensionSize(dataColumn, 0),
			polarizationCount = _file->GetTableDimensionSize(dataColumn, 1),
			raCount = _file->GetTableDimensionSize(dataColumn, 2),
			decCount = _file->GetTableDimensionSize(dataColumn, 3);
			
		const std::string telescopeName = _file->GetKeywordValue("TELESCOP");
		_antennaInfos[0].name = telescopeName;
			
		const int totalSize = _file->GetTableColumnArraySize(dataColumn);
		AOLogger::Debug << "Shape of data cells: " << freqCount << " channels x " << polarizationCount << " pols x " << raCount << " RAs x " << decCount << " decs" << "=" << totalSize << '\n';
		std::vector<long double> cellData(totalSize);
		bool *flagData = new bool[totalSize];
		std::vector<Image2DPtr> images(polarizationCount);
		std::vector<Mask2DPtr> masks(polarizationCount);
		for(int i=0;i<polarizationCount;++i)
		{
			images[i] = Image2D::CreateZeroImagePtr(rowCount, freqCount);
			masks[i] = Mask2D::CreateSetMaskPtr<true>(rowCount, freqCount);
		}
		std::vector<double> observationTimes(rowCount);
		bool hasBand = false;
		const int requestedIFNumber = _bandIndexToNumber[ifIndex];
		size_t timeIndex = 0;
		for(int row=1;row<=rowCount;++row)
		{
			long double time, date, ifNumber;
			_file->ReadTableCell(row, ifColumn, &ifNumber, 1);
			
			if(ifNumber == requestedIFNumber)
			{
				_file->ReadTableCell(row, timeColumn, &time, 1);
				_file->ReadTableCell(row, dateObsColumn, &date, 1);
				_file->ReadTableCell(row, dataColumn, &cellData[0], totalSize);
				_file->ReadTableCell(row, flagColumn, &flagData[0], totalSize);
			
				observationTimes[timeIndex] = time;
				
				if(!hasBand)
				{
					long double freqVal = 0.0, freqRefPix = 0.0, freqDelta = 0.0, freqRes = 0.0, freqBandwidth = 0.0;
					_file->ReadTableCell(row, freqValColumn, &freqVal, 1);
					_file->ReadTableCell(row, freqRefPixColumn, &freqRefPix, 1);
					_file->ReadTableCell(row, freqDeltaColumn, &freqDelta, 1);
					_file->ReadTableCell(row, freqResColumn, &freqRes, 1);
					_file->ReadTableCell(row, freqBandwidthColumn, &freqBandwidth, 1);
					if(freqBandwidth > 0.0)
					{
						AOLogger::Debug << "Frequency info: " <<freqVal << " Hz at index " << freqRefPix << ", delta " << freqDelta << "\n";
						AOLogger::Debug << "Frequency res: " <<freqRes << " with bandwidth " << freqBandwidth << " Hz\n";
						BandInfo bandInfo;
						bandInfo.windowIndex = ifNumber;
						for(int i=0;i<freqCount;++i)
						{
							ChannelInfo c;
							c.frequencyIndex = i;
							c.frequencyHz = ((double) i-freqRefPix)*freqDelta + freqVal;
							bandInfo.channels.push_back(c);
						}
						_bandInfos[ifNumber] = bandInfo;
						metaData.SetBand(bandInfo);
						hasBand = true;
					}
				}

				long double *dataPtr = &cellData[0];
				bool *flagPtr = flagData;
				for(int p=0;p<polarizationCount;++p)
				{
					for(int f=0;f<freqCount;++f)
					{
						images[p]->SetValue(timeIndex, f, *dataPtr);
						masks[p]->SetValue(timeIndex, f, *flagPtr);
						++dataPtr;
						++flagPtr;
					}
				}
				++timeIndex;
			}
		}
		delete[] flagData;
		if(timeIndex == 0)
		{
			throw std::runtime_error("Couldn't find any rows in the fits image set for the requested IF");
		}
		for(int p=0;p<polarizationCount;++p)
		{
			images[p]->SetTrim(0, 0, timeIndex, images[p]->Height());
			masks[p] = masks[p]->Trim(0, 0, timeIndex, images[p]->Height());
		}
		observationTimes.resize(timeIndex);
		metaData.SetObservationTimes(observationTimes);
		if(polarizationCount == 1)
		{
			data = TimeFrequencyData(TimeFrequencyData::AmplitudePart, StokesIPolarisation, images[0]);
			data.SetGlobalMask(masks[0]);
		} else if(polarizationCount == 2)
		{
			data = TimeFrequencyData(TimeFrequencyData::AmplitudePart, AutoDipolePolarisation, images[0], images[1]);
			data.SetIndividualPolarisationMasks(masks[0], masks[1]);
		}
		else throw std::runtime_error("Don't know how to convert polarizations in file");
	}
	
	void FitsImageSet::AddWriteFlagsTask(const ImageSetIndex &index, std::vector<Mask2DCPtr> &flags)
	{
		if(_file->HasGroups())
			throw BadUsageException("Not implemented for grouped fits files");
		else
			saveSingleDishFlags(flags, static_cast<const FitsImageSetIndex&>(index)._band);
	}

	void FitsImageSet::PerformWriteFlagsTask()
	{
		if(_file->HasGroups())
			throw BadUsageException("Not implemented for grouped fits files");
		else {
			// Nothing to be done; Add..Task already wrote the flags.
		}
	}
	
	void FitsImageSet::saveSingleDishFlags(std::vector<Mask2DCPtr> &flags, size_t ifIndex)
	{
		_file->Close();
		_file->Open(FitsFile::ReadWriteMode);
		_file->MoveToHDU(2);
		AOLogger::Debug << "Writing single dish table for band " << ifIndex << " with " << _file->GetRowCount() << " rows.\n";
		const int
			dataColumn = _file->GetTableColumnIndex("DATA"),
			flagColumn = _file->GetTableColumnIndex("FLAGGED"),
			ifColumn = _file->GetTableColumnIndex("IF");
		const int
			freqCount = _file->GetTableDimensionSize(dataColumn, 0),
			polarizationCount = _file->GetTableDimensionSize(dataColumn, 1);
			
		const int totalSize = _file->GetTableColumnArraySize(dataColumn);
		const int rowCount = _file->GetRowCount();
		std::vector<double> cellData(totalSize);
		bool *flagData = new bool[totalSize];
		std::vector<Mask2DCPtr> storedFlags = flags;
		if(flags.size()==1)
		{
			while(storedFlags.size() < (unsigned) polarizationCount) storedFlags.push_back(flags[0]);
		}
		if(storedFlags.size() != (unsigned) polarizationCount)
		{
			std::stringstream s;
			s << "saveSingleDishFlags() : mismatch in polarization count: the given vector contains " << flags.size() << " polarizations, the number of polarizations in the file is " << polarizationCount;
			throw std::runtime_error(s.str());
		}
		for(std::vector<Mask2DCPtr>::const_iterator i=storedFlags.begin();i!=storedFlags.end();++i)
		{
			if((*i)->Height() != (unsigned) freqCount)
				throw std::runtime_error("Frequency count in given mask does not match with the file");
		}
		size_t timeIndex = 0;
		int specifiedIFNumber = _bandIndexToNumber[ifIndex];
		for(int row=1;row<=rowCount;++row)
		{
			long double ifNumber;
			_file->ReadTableCell(row, ifColumn, &ifNumber, 1);
			
			if(ifNumber == specifiedIFNumber)
			{
				AOLogger::Debug << row << "\n";
				_file->ReadTableCell(row, dataColumn, &cellData[0], totalSize);
				double *dataPtr = &cellData[0];
				bool *flagPtr = flagData;
				
				for(int p=0;p<polarizationCount;++p)
				{
					for(int f=0;f<freqCount;++f)
					{
						if(storedFlags[p]->Value(timeIndex, f))
						{
							*flagPtr = true;
							*dataPtr = 1e20;
						} else {
							*flagPtr = false;
						}
						++dataPtr;
						++flagPtr;
					}
				}
				
				_file->WriteTableCell(row, dataColumn, &cellData[0], totalSize);
				_file->WriteTableCell(row, flagColumn, flagData, totalSize);
				++timeIndex;
			}
		}
		delete[] flagData;
	}
	
	void FitsImageSetIndex::Previous()
	{
		if(_baselineIndex > 0)
			--_baselineIndex;
		else {
			_baselineIndex = static_cast<class FitsImageSet&>(imageSet()).Baselines().size() - 1;
			LargeStepPrevious();
		}
	}
	
	void FitsImageSetIndex::Next()
	{
		++_baselineIndex;
		if( _baselineIndex >= static_cast<class FitsImageSet&>(imageSet()).Baselines().size() )
		{
			_baselineIndex = 0;
			LargeStepNext();
		}
	}

	void FitsImageSetIndex::LargeStepPrevious()
	{
		if(_band > 0)
			--_band;
		else {
			_band = static_cast<class FitsImageSet&>(imageSet()).BandCount() - 1;
			_isValid = false;
		}
	}
	
	void FitsImageSetIndex::LargeStepNext()
	{
		++_band;
		if(_band >= static_cast<class FitsImageSet&>(imageSet()).BandCount())
		{
			_band = 0;
			_isValid = false;
		}
	}

	std::string FitsImageSetIndex::Description() const {
		FitsImageSet &set = static_cast<class FitsImageSet&>(imageSet());
		int a1 = set.Baselines()[_baselineIndex].first;
		int a2 = set.Baselines()[_baselineIndex].second;
		AntennaInfo info1 = set.GetAntennaInfo(a1);
		AntennaInfo info2 = set.GetAntennaInfo(a2);
		std::stringstream s;
		s << "fits correlation " << info1.name << " x " << info2.name << ", band " << _band;
		return s.str();
	}

	std::string FitsImageSet::File()
	{
		return _file->Filename();
	}
	
	std::string FitsImageSet::ReadTelescopeName()
	{
		for(int hduIndex=2; hduIndex <= _file->GetHDUCount(); hduIndex++)
		{
			_file->MoveToHDU(hduIndex);
			std::string extName = _file->GetKeywordValue("EXTNAME");
			if(extName == "SINGLE DISH")
				return _file->GetKeywordValue("TELESCOP");
		}
		return "";
	}
}
