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
#include "timefrequencyimager.h"
#include "timefrequencydata.h"

#include <set>
#include <stdexcept>

#include <ms/MeasurementSets/MeasurementSet.h>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableIter.h>

#include "../util/aologger.h"

#include "arraycolumniterator.h"
#include "scalarcolumniterator.h"

TimeFrequencyImager::TimeFrequencyImager(MeasurementSet &measurementSet)
	: _readData(true), _readFlags(false),
		_readXX(true), _readXY(true), _readYX(true), _readYY(true), _readStokesI(false), _readStokesIDirectly(false),
		_measurementSet(&measurementSet), _dataKind(CorrectedData), _sortedTable(0)
{
}

TimeFrequencyImager::~TimeFrequencyImager()
{
	ClearImages();
	if(_sortedTable != 0)
		delete _sortedTable;
}

void TimeFrequencyImager::ClearImages()
{
	_realXX.reset();
	_imaginaryXX.reset();
	_realXY.reset();
	_imaginaryXY.reset();
	_realYX.reset();
	_imaginaryYX.reset();
	_realYY.reset();
	_imaginaryYY.reset();
	_realStokesI.reset();
	_imaginaryStokesI.reset();
	_flagXX.reset();
	_flagXY.reset();
	_flagYX.reset();
	_flagYY.reset();
	_flagCombined.reset();
}

void TimeFrequencyImager::setObservationTimes(MeasurementSet &set, std::map<double,size_t> &observationTimes)
{
	observationTimes.clear();
	const std::set<double> &times = set.GetObservationTimesSet();
	unsigned index = 0;
	for(std::set<double>::const_iterator i=times.begin();i!=times.end();++i)
	{
		observationTimes.insert(std::pair<double,size_t>(*i, index));
		++index; 
	}
}

void TimeFrequencyImager::Image(size_t antenna1Select, size_t antenna2Select, size_t spectralWindowSelect)
{
	_antenna1Select = antenna1Select;
	_antenna2Select	= antenna2Select;
	_spectralWindowSelect = spectralWindowSelect;

	_bandInfo = _measurementSet->GetBandInfo(spectralWindowSelect);

	setObservationTimes(*_measurementSet, _observationTimes);
	image(antenna1Select, antenna2Select, spectralWindowSelect, 0, _observationTimes.size());
}

void TimeFrequencyImager::Image(size_t antenna1Select, size_t antenna2Select, size_t spectralWindowSelect, size_t startIndex, size_t endIndex)
{
	_antenna1Select = antenna1Select;
	_antenna2Select	= antenna2Select;
	_spectralWindowSelect = spectralWindowSelect;

	_bandInfo = _measurementSet->GetBandInfo(spectralWindowSelect);

	setObservationTimes(*_measurementSet, _observationTimes);
	image(antenna1Select, antenna2Select, spectralWindowSelect, startIndex, endIndex);
}

void TimeFrequencyImager::image(size_t antenna1Select, size_t antenna2Select, size_t spectralWindowSelect, size_t startIndex, size_t endIndex)
{
	size_t timeCount = _observationTimes.size();
	int frequencyCount = _measurementSet->FrequencyCount();

	initializePolarizations();
	checkPolarizations();

	if(_sortedTable == 0)
	{
		casa::Table *rawTable = _measurementSet->OpenTable();
		casa::Block<casa::String> names(4);
		names[0] = "DATA_DESC_ID";
		names[1] = "ANTENNA1";
		names[2] = "ANTENNA2";
		names[3] = "TIME";
		_sortedTable = new casa::Table(rawTable->sort(names));
		delete rawTable;
	}

	casa::Block<casa::String> selectionNames(3);
	selectionNames[0] = "DATA_DESC_ID";
	selectionNames[1] = "ANTENNA1";
	selectionNames[2] = "ANTENNA2";
	casa::TableIterator iter(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
	while(!iter.pastEnd())
	{
		casa::Table table = iter.table();
		casa::ROScalarColumn<int> antenna1(table, "ANTENNA1"); 
		casa::ROScalarColumn<int> antenna2(table, "ANTENNA2");
		casa::ROScalarColumn<int> windowColumn(table, "DATA_DESC_ID");
		if(table.nrow() > 0 && windowColumn(0) == (int) spectralWindowSelect && antenna1(0) == (int) antenna1Select && antenna2(0) == (int) antenna2Select)
		{
			break;
		} else {
			iter.next();
		}
	}
	if(iter.pastEnd())
	{
		throw std::runtime_error("Baseline not found");
	}

	casa::Table table = iter.table();

	if(startIndex > timeCount)
	{
		std::cerr << "Warning: startIndex > timeCount" << std::endl;
	}
	if(endIndex > timeCount)
	{
		endIndex = timeCount;
		std::cerr << "Warning: endIndex > timeCount" << std::endl;
	}

	size_t width = endIndex-startIndex;
	if(width == 0 || frequencyCount == 0)
		return;

	ClearImages();

	if(_readData) {
		if(_realXX==0 && _readXX)
		{
			_realXX = Image2D::CreateZeroImagePtr(width, frequencyCount);
			_imaginaryXX = Image2D::CreateZeroImagePtr(width, frequencyCount);
		}
		if(_realXY == 0 && _readXY)
		{
			_realXY = Image2D::CreateZeroImagePtr(width, frequencyCount);
			_imaginaryXY = Image2D::CreateZeroImagePtr(width, frequencyCount);
		}
		if(_realYX == 0 && _readYX)
		{
			_realYX = Image2D::CreateZeroImagePtr(width, frequencyCount);
			_imaginaryYX = Image2D::CreateZeroImagePtr(width, frequencyCount);
		}
		if(_realYY == 0 && _readYY)
		{
			_realYY = Image2D::CreateZeroImagePtr(width, frequencyCount);
			_imaginaryYY = Image2D::CreateZeroImagePtr(width, frequencyCount);
		}
		if(_realStokesI == 0 && _readStokesI)
		{
			_realStokesI = Image2D::CreateZeroImagePtr(width, frequencyCount);
			_imaginaryStokesI = Image2D::CreateZeroImagePtr(width, frequencyCount);
		}
	}
	if(_readFlags) {
		// The flags should be initialized to true, as this baseline might
		// miss some time scans that other baselines do have, and these
		// should be flagged.
		if(_flagXX==0 && _readXX)
			_flagXX = Mask2D::CreateSetMaskPtr<true>(width, frequencyCount);
		if(_flagXY==0 && _readXY)
			_flagXY = Mask2D::CreateSetMaskPtr<true>(width, frequencyCount);
		if(_flagYX==0 && _readYX)
			_flagYX = Mask2D::CreateSetMaskPtr<true>(width, frequencyCount);
		if(_flagYY==0 && _readYY)
			_flagYY = Mask2D::CreateSetMaskPtr<true>(width, frequencyCount);
		if(_flagCombined==0 && _readStokesI)
			_flagCombined = Mask2D::CreateSetMaskPtr<true>(width, frequencyCount);
	}
	_uvw.resize(width);

	casa::ROScalarColumn<int> antenna1(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2(table, "ANTENNA2");
	casa::ROScalarColumn<int> windowColumn(table, "DATA_DESC_ID");
	casa::ROScalarColumn<double> timeColumn(table, "TIME");
	casa::ROArrayColumn<float> weightColumn(table, "WEIGHT");
	casa::ROArrayColumn<double> uvwColumn(table, "UVW");

	casa::ROArrayColumn<casa::Complex> *dataColumn = 0;
	if(_readData)
		dataColumn = CreateDataColumn(_dataKind, table);

	ROArrayColumnIterator<casa::Complex> *modelIter;
	if(_dataKind == ResidualData) {
		casa::ROArrayColumn<casa::Complex> *modelColumn;
		modelColumn = new casa::ROArrayColumn<casa::Complex>(table, "MODEL_DATA");
		modelIter = new ROArrayColumnIterator<casa::Complex>(ROArrayColumnIterator<casa::Complex>::First(*modelColumn));
	} else {
		modelIter = 0;
	}
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");

	ScalarColumnIterator<int> antenna1Iter = ScalarColumnIterator<int>::First(antenna1);
	ScalarColumnIterator<int> antenna2Iter = ScalarColumnIterator<int>::First(antenna2);
	ScalarColumnIterator<int> windowIter = ScalarColumnIterator<int>::First(windowColumn);
	ScalarColumnIterator<double> timeIter = ScalarColumnIterator<double>::First(timeColumn);
	ROArrayColumnIterator<double> uvwIter = ROArrayColumnIterator<double>::First(uvwColumn);
	ROArrayColumnIterator<float> weightIter = ROArrayColumnIterator<float>::First(weightColumn);
	ROArrayColumnIterator<casa::Complex> dataIter = 
		ROArrayColumnIterator<casa::Complex>::First(*dataColumn);
	ROArrayColumnIterator<bool> flagIter = 
		ROArrayColumnIterator<bool>::First(flagColumn);

	for(size_t i=0;i<table.nrow();++i) {
		double time = *timeIter;
		size_t timeIndex = _observationTimes.find(time)->second;
		bool timeIsSelected = timeIndex>=startIndex && timeIndex<endIndex;
		if(_readData && timeIsSelected) {
			if(_dataKind == WeightData)
				ReadWeights(timeIndex-startIndex, frequencyCount, *weightIter);
			else if(modelIter == 0)
				ReadTimeData(timeIndex-startIndex, frequencyCount, *dataIter, 0);
			else {
				const casa::Array<casa::Complex> model = **modelIter; 
				ReadTimeData(timeIndex-startIndex, frequencyCount, *dataIter, &model);
			}
		}
		if(_readFlags && timeIsSelected) {
			const casa::Array<bool> flag = *flagIter;
			ReadTimeFlags(timeIndex-startIndex, frequencyCount, flag);
		}
		if(timeIsSelected) {
			casa::Array<double> arr = *uvwIter;
			casa::Array<double>::const_iterator i = arr.begin();
			_uvw[timeIndex-startIndex].u = *i;
			++i;
			_uvw[timeIndex-startIndex].v = *i;
			++i;
			_uvw[timeIndex-startIndex].w = *i;
		}

		if(_readData)
		{
			++dataIter;
			if(modelIter != 0)
				++(*modelIter);
		}
		if(_readFlags)
		{
			++flagIter;
		}

		++weightIter;
		++antenna1Iter;
		++antenna2Iter;
		++timeIter;
		++uvwIter;
		++windowIter;
	}
	if(dataColumn != 0)
		delete dataColumn;
}

casa::ROArrayColumn<casa::Complex> *TimeFrequencyImager::CreateDataColumn(DataKind kind, casa::Table &table)
{
	switch(kind) {
		case ObservedData:
		default:
		return new casa::ROArrayColumn<casa::Complex>(table, "DATA");
		case CorrectedData:
		case ResidualData:
		return new casa::ROArrayColumn<casa::Complex>(table, "CORRECTED_DATA");
		case ModelData:
		return new casa::ROArrayColumn<casa::Complex>(table, "MODEL_DATA");
	}
}

void TimeFrequencyImager::WriteNewFlags(Mask2DCPtr newXX, Mask2DCPtr newXY, Mask2DCPtr newYX, Mask2DCPtr newYY)
{
	WriteNewFlagsPart(newXX, newXY, newYX, newYY, _antenna1Select, _antenna2Select, _spectralWindowSelect, 0, newXX->Width());
}

void TimeFrequencyImager::WriteNewFlags(Mask2DCPtr newXX, Mask2DCPtr newXY, Mask2DCPtr newYX, Mask2DCPtr newYY, int antenna1, int antenna2, int spectralWindow)
{
	WriteNewFlagsPart(newXX, newXY, newYX, newYY, antenna1, antenna2, spectralWindow, 0, newXX->Width());
}

void TimeFrequencyImager::WriteNewFlagsPart(Mask2DCPtr newXX, Mask2DCPtr newXY, Mask2DCPtr newYX, Mask2DCPtr newYY, int antenna1, int antenna2, int spectralWindow, size_t timeOffset, size_t timeEnd, size_t leftBorder, size_t rightBorder)
{
	initializePolarizations();
	checkPolarizations();

	size_t frequencyCount = _measurementSet->FrequencyCount();

	std::map<double,size_t> observationTimes;
	setObservationTimes(*_measurementSet, observationTimes);

	casa::Table *table = _measurementSet->OpenTable(true);
	casa::ROScalarColumn<int> antenna1Column(*table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(*table, "ANTENNA2");
	casa::ROScalarColumn<int> windowColumn(*table, "DATA_DESC_ID");
	casa::ROScalarColumn<double> timeColumn(*table, "TIME");
	casa::ArrayColumn<bool> flagColumn(*table, "FLAG");

	ScalarColumnIterator<int> antenna1Iter = ScalarColumnIterator<int>::First(antenna1Column);
	ScalarColumnIterator<int> antenna2Iter = ScalarColumnIterator<int>::First(antenna2Column);
	ScalarColumnIterator<int> windowIter = ScalarColumnIterator<int>::First(windowColumn);
	ScalarColumnIterator<double> timeIter = ScalarColumnIterator<double>::First(timeColumn);
	ArrayColumnIterator<bool> flagIter = ArrayColumnIterator<bool>::First(flagColumn);

	if(frequencyCount != newXX->Height())
	{
		std::cerr << "The frequency count in the measurement set (" << frequencyCount << ") does not match the image!" << std::endl;
	}
	if(timeEnd - timeOffset != newXX->Width())
	{
		std::cerr << "The number of time scans to write in the measurement set (" << (timeEnd - timeOffset) << ") does not match the image (" << newXX->Width() << ") !" << std::endl;
	}

	size_t rowsWritten = 0;
	for(size_t i=0;i<table->nrow();++i) {
		if((*antenna1Iter) == (int) antenna1 &&
		   (*antenna2Iter) == (int) antenna2 &&
		   (*windowIter) == (int) spectralWindow)
		{
			double time = *timeIter;
			size_t timeIndex = observationTimes.find(time)->second;
			if(timeIndex >= timeOffset + leftBorder && timeIndex < timeEnd - rightBorder)
			{
				casa::Array<bool> flag = *flagIter;
				casa::Array<bool>::iterator j = flag.begin();
				for(size_t f=0;f<(size_t) frequencyCount;++f) {
					if(_stokesIIndex >= 0)
					{
						if(_readStokesIDirectly) *j = newXX->Value(timeIndex - timeOffset, f);
						++j;
					}
					if(_xxIndex >= 0)
					{
						if(_readXX) *j = newXX->Value(timeIndex - timeOffset, f);
						++j;
					}
					if(_xyIndex >= 0)
					{
						if(_readXY) *j = newXY->Value(timeIndex - timeOffset, f);
						++j;
					}
					if(_yxIndex >= 0)
					{
						if(_readYX) *j = newYX->Value(timeIndex - timeOffset, f);
						++j;
					}
					if(_yyIndex >= 0)
					{
						if(_readYY) *j = newYY->Value(timeIndex - timeOffset, f);
						++j;
					}
				}
				flagIter.Set(flag);
				++rowsWritten;
			}
		}

		++antenna1Iter;
		++antenna2Iter;
		++timeIter;
		++windowIter;
		++flagIter;
	}
	AOLogger::Debug << "Rows written: " << rowsWritten << '\n';

	delete table;
}

void TimeFrequencyImager::ReadTimeData(size_t xOffset, int frequencyCount, const casa::Array<casa::Complex> data, const casa::Array<casa::Complex> *model)
{
	casa::Array<casa::Complex>::const_iterator i = data.begin();
	casa::Array<casa::Complex>::const_iterator m;
	if(_dataKind == ResidualData)
		m = model->begin();

	for(size_t f=0;f<(size_t) frequencyCount;++f) {
		double xxr, xxi, xyr, xyi, yxr, yxi, yyr, yyi, ir, ii;

		if(_dataKind == ResidualData) {
			const casa::Complex &iData = *i;
			const casa::Complex &iModel = *m;
			if(_stokesIIndex >= 0) { ++i; ++m; }

			const casa::Complex &xxData = *i;
			const casa::Complex &xxModel = *m;
			if(_xxIndex >= 0) { ++i; ++m; }

			const casa::Complex &xyData = *i;
			const casa::Complex &xyModel = *m;
			if(_xyIndex >= 0) { ++i; ++m; }

			const casa::Complex &yxData = *i;
			const casa::Complex &yxModel = *m;
			if(_yxIndex >= 0) { ++i; ++m; }

			const casa::Complex &yyData = *i;
			const casa::Complex &yyModel = *m;
			if(_yyIndex >= 0) { ++i; ++m; }

			ir = iData.real() - iModel.real();
			ii = iData.imag() - iModel.imag();
			xxr = xxData.real() - xxModel.real();
			xxi = xxData.imag() - xxModel.imag();
			xyr = xyData.real() - xyModel.real();
			xyi = xyData.imag() - xyModel.imag();
			yxr = yxData.real() - yxModel.real();
			yxi = yxData.imag() - yxModel.imag();
			yyr = yyData.real() - yyModel.real();
			yyi = yyData.imag() - yyModel.imag();
		} else {
			const casa::Complex &stokesI = *i;
			if(_stokesIIndex >= 0) ++i;
			const casa::Complex &xx = *i;
			if(_xxIndex >= 0) ++i;
			const casa::Complex &xy = *i;
			if(_xyIndex >= 0) ++i;
			const casa::Complex &yx = *i;
			if(_yxIndex >= 0) ++i;
			const casa::Complex &yy = *i;
			if(_yyIndex >= 0) ++i;

			xxr = xx.real();
			xxi = xx.imag();
			xyr = xy.real();
			xyi = xy.imag();
			yxr = yx.real();
			yxi = yx.imag();
			yyr = yy.real();
			yyi = yy.imag();
			ir = stokesI.real();
			ii = stokesI.imag();
		}
	
		if(_readXX)
		{
			_realXX->SetValue(xOffset, f, xxr);
			_imaginaryXX->SetValue(xOffset, f, xxi);
		}
		if(_readXY)
		{
			_realXY->SetValue(xOffset, f, xyr);
			_imaginaryXY->SetValue(xOffset, f, xyi);
		}
		if(_readYX)
		{
			_realYX->SetValue(xOffset, f, yxr);
			_imaginaryYX->SetValue(xOffset, f, yxi);
		}
		if(_readYY)
		{
			_realYY->SetValue(xOffset, f, yyr);
			_imaginaryYY->SetValue(xOffset, f, yyi);
		}
		if(_readStokesI)
		{
			if(_readStokesIDirectly) {
				_realStokesI->SetValue(xOffset, f, ir);
				_imaginaryStokesI->SetValue(xOffset, f, ii);
			} else {
				_realStokesI->SetValue(xOffset, f, xxr + yyr);
				_imaginaryStokesI->SetValue(xOffset, f, xxi + yyi);
			}
		}
	}
}

void TimeFrequencyImager::ReadTimeFlags(size_t xOffset, int frequencyCount, const casa::Array<bool> flag)
{
	casa::Array<bool>::const_iterator j = flag.begin();
	for(size_t f=0;f<(size_t) frequencyCount;++f) {
		bool stokesIF = false;
		if(_stokesIIndex >= 0) {
			stokesIF = *j;
			++j;
		}
		if(_xxIndex >= 0)
		{
			if(_readXX) _flagXX->SetValue(xOffset, f, *j);
			stokesIF |= *j;
			++j;
		}
		if(_xyIndex >= 0)
		{
			if(_readXY) _flagXY->SetValue(xOffset, f, *j);
			stokesIF |= *j;
			++j;
		}
		if(_yxIndex >= 0)
		{
			if(_readYX) _flagYX->SetValue(xOffset, f, *j);
			stokesIF |= *j;
			++j;
		}
		if(_yyIndex >= 0)
		{
			if(_readYY) _flagYY->SetValue(xOffset, f, *j);
			stokesIF |= *j;
			++j;
		}
		if(_readStokesI)
		{
			_flagCombined->SetValue(xOffset, f, stokesIF);
		}
	} 
}

void TimeFrequencyImager::ReadWeights(size_t xOffset, int frequencyCount, const casa::Array<float> weight)
{
	casa::Array<float>::const_iterator j = weight.begin();
	float xx = *j;
	++j;
	float xy = *j;
	++j;
	float yx = *j;
	++j;
	float yy = *j;
	++j;
	for(size_t f=0;f<(size_t) frequencyCount;++f) {
		if(_readXX)
		{
			_realXX->SetValue(xOffset, f, xx);
			_imaginaryXX->SetValue(xOffset, f, 0.0);
		}
		if(_readXY)
		{
			_realXY->SetValue(xOffset, f, xy);
			_imaginaryXY->SetValue(xOffset, f, 0.0);
		}
		if(_readYX)
		{
			_realYX->SetValue(xOffset, f, yx);
			_imaginaryYX->SetValue(xOffset, f, 0.0);
		}
		if(_readYY)
		{
			_realYY->SetValue(xOffset, f, yy);
			_imaginaryYY->SetValue(xOffset, f, 0.0);
		}
		if(_readStokesI)
		{
			_realStokesI->SetValue(xOffset, f, xx + yy);
			_imaginaryStokesI->SetValue(xOffset, f, 0.0);
		}
	} 
}

TimeFrequencyData TimeFrequencyImager::GetData() const
{
	TimeFrequencyData data;
	if(
		_realXX != 0 && _imaginaryXX != 0 &&
		_realXY != 0 && _imaginaryXY != 0 &&
		_realYX != 0 && _imaginaryYX != 0 &&
		_realYY != 0 && _imaginaryYY != 0)
	{
		data = TimeFrequencyData(_realXX, _imaginaryXX, _realXY, _imaginaryXY, _realYX, _imaginaryYX, _realYY, _imaginaryYY);
	} else if(
		_realXX != 0 && _imaginaryXX != 0 &&
		_realYY != 0 && _imaginaryYY != 0)
	{
		data = TimeFrequencyData(AutoDipolePolarisation, _realXX, _imaginaryXX, _realYY, _imaginaryYY);
	} else if(_realStokesI != 0 && _imaginaryStokesI != 0)
	{
		data = TimeFrequencyData(StokesIPolarisation, _realStokesI, _imaginaryStokesI);
	}

	if(_flagXX != 0 && _flagXY != 0 && _flagYX != 0 && _flagYY != 0)
	{
		if(data.Polarisation() != DipolePolarisation)
			throw BadUsageException("Trying to read dipole polarisation masks, but TF data does not have the dipole polarisations"); 
		data.SetIndividualPolarisationMasks(_flagXX, _flagXY, _flagYX, _flagYY);
	} else if(_flagXX != 0 && _flagYY != 0)
	{
		if(data.Polarisation() != AutoDipolePolarisation)
			throw BadUsageException("Trying to read auto dipole polarisation masks, but TF data does not have the auto dipole polarisations"); 
		data.SetIndividualPolarisationMasks(_flagXX, _flagYY);
	} else if(_flagCombined != 0)
	{
		data.SetGlobalMask(_flagCombined);
	}

	return data;
}

void TimeFrequencyImager::PartInfo(const std::string &msFile, size_t maxTimeScans, size_t &timeScanCount, size_t &partCount)
{
	MeasurementSet set(msFile);
	std::map<double,size_t> observationTimes;
	setObservationTimes(set, observationTimes);

	timeScanCount = observationTimes.size();
	if(maxTimeScans == 0)
		partCount = 1;
	else
	{
		partCount = (timeScanCount + maxTimeScans - 1) / maxTimeScans;
		if(partCount == 0)
			partCount = 1;
	}
}

void TimeFrequencyImager::initializePolarizations()
{
	casa::MeasurementSet ms(_measurementSet->Location());
	casa::Table polTable = ms.polarization();
	casa::ROArrayColumn<int> corTypeColumn(polTable, "CORR_TYPE"); 
	casa::Array<int> corType = corTypeColumn(0);
	casa::Array<int>::iterator iterend(corType.end());
	_xxIndex = -1, _xyIndex = -1, _yxIndex = -1, _yyIndex = -1; _stokesIIndex = -1;
	int polarizationCount = 0;
	for (casa::Array<int>::iterator iter=corType.begin(); iter!=iterend; ++iter)
	{
		switch(*iter) {
			case 1: _stokesIIndex = polarizationCount; break;
			case 5:
			case 9: _xxIndex = polarizationCount; break;
			case 6:
			case 10: _xyIndex = polarizationCount; break;
			case 7:
			case 11: _yxIndex = polarizationCount; break;
			case 8:
			case 12: _yyIndex = polarizationCount; break;
			default:
			{
				std::stringstream s;
				s << "There is a polarization in the measurement set that I can not handle (" << *iter << ", polarization index " << polarizationCount << ").";
				throw std::runtime_error(s.str());
			}
		}
		++polarizationCount;
  }
}

void TimeFrequencyImager::checkPolarizations()
{
	if(_readXX && _xxIndex < 0)
	{
		std::cerr << "Warning: trying to read XX polarization, but polarization not available." << std::endl;
		_readXX = false;
	}
	if(_readXY && _xyIndex < 0)
	{
		std::cerr << "Warning: trying to read XY polarization, but polarization not available." << std::endl;
		_readXY = false;
	}
	if(_readYX && _yxIndex < 0)
	{
		std::cerr << "Warning: trying to read YX polarization, but polarization not available." << std::endl;
		_readYX = false;
	}
	if(_readYY && _yyIndex < 0)
	{
		std::cerr << "Warning: trying to read YY polarization, but polarization not available." << std::endl;
		_readYY = false;
	}
	if(_readStokesI)
	{
		if(_stokesIIndex >= 0)
			_readStokesIDirectly = true;
		else if(_xxIndex >= 0 && _yyIndex >= 0)
		{
			_readStokesIDirectly = false;
		} else
		{
			std::cerr << "Warning: trying to read StokesI polarization, but polarization not available." << std::endl;
			_readStokesI = false;
		}
	}
}
