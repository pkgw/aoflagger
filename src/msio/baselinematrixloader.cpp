#include "baselinematrixloader.h"

#include <stdexcept>

#include <tables/Tables/ExprNode.h>

#include "arraycolumniterator.h"
#include "scalarcolumniterator.h"
#include "spatialmatrixmetadata.h"

BaselineMatrixLoader::BaselineMatrixLoader(MeasurementSet &measurementSet)
	: _sortedTable(0), _tableIter(0), _currentIterIndex(0), _measurementSet(measurementSet), _timeIndexCount(0), _metaData(0)
{
	casa::Table *rawTable = new casa::Table(_measurementSet.Path());
	casa::Block<casa::String> names(4);
	names[0] = "DATA_DESC_ID";
	names[1] = "TIME";
	names[2] = "ANTENNA1";
	names[3] = "ANTENNA2";
	_sortedTable = new casa::Table(rawTable->sort(names));
	delete rawTable;

	casa::Block<casa::String> selectionNames(2);
	selectionNames[0] = "DATA_DESC_ID";
	selectionNames[1] = "TIME";
	casa::TableIterator iter(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
	while(!iter.pastEnd())
	{
		iter.next();
		++_timeIndexCount;
	}
	_frequencyCount = _measurementSet.FrequencyCount(0);

	_tableIter = new casa::TableIterator(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
}

BaselineMatrixLoader::~BaselineMatrixLoader()
{
	if(_sortedTable != 0)
		delete _sortedTable;
	if(_metaData != 0)
		delete _metaData;
	delete _tableIter;
}

TimeFrequencyData BaselineMatrixLoader::LoadSummed(size_t timeIndex)
{
	casa::Block<casa::String> selectionNames(2);
	selectionNames[0] = "DATA_DESC_ID";
	selectionNames[1] = "TIME";
	if(timeIndex < _currentIterIndex)
	{
		delete _tableIter;
		_tableIter = new casa::TableIterator(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
		_currentIterIndex = 0;
	}
	while(!_tableIter->pastEnd() && timeIndex-_currentIterIndex > 0)
	{
		_tableIter->next();
		++_currentIterIndex;
	}
	if(_tableIter->pastEnd())
	{
		throw std::runtime_error("Time index not found");
	}

	casa::Table table = _tableIter->table();
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");

	ScalarColumnIterator<int> antenna1Iter = ScalarColumnIterator<int>::First(antenna1Column);
	ScalarColumnIterator<int> antenna2Iter = ScalarColumnIterator<int>::First(antenna2Column);

	// Find highest antenna index
	int nrAntenna = 0;
	for(size_t i=0;i<table.nrow();++i)
	{
		int
			a1 = *antenna1Iter,
			a2 = *antenna2Iter;

		if(a1 > nrAntenna)
			nrAntenna = a1;
		if(a2 > nrAntenna)
			nrAntenna = a2;

		++antenna1Iter;
		++antenna2Iter;
	}
	++nrAntenna;

	if(_metaData != 0)
		delete _metaData;
	_metaData = new SpatialMatrixMetaData(nrAntenna);

	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
	casa::ROArrayColumn<double> uvwColumn(table, "UVW");

	antenna1Iter = ScalarColumnIterator<int>::First(antenna1Column);
	antenna2Iter = ScalarColumnIterator<int>::First(antenna2Column);
	ROArrayColumnIterator<casa::Complex> dataIter =
		ROArrayColumnIterator<casa::Complex>::First(dataColumn);
	ROArrayColumnIterator<bool> flagIter =
		ROArrayColumnIterator<bool>::First(flagColumn);
	ROArrayColumnIterator<double> uvwIter =
		ROArrayColumnIterator<double>::First(uvwColumn);

	Image2DPtr
		xxRImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		xxIImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		xyRImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		xyIImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yxRImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yxIImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yyRImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yyIImage = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna);
	Mask2DPtr
		xxMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna),
		xyMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna),
		yxMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna),
		yyMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna);

	for(size_t j=0;j<table.nrow();++j)
	{
		int
			a1 = *antenna1Iter,
			a2 = *antenna2Iter;
		casa::Array<casa::Complex> data =
			*dataIter;
		casa::Array<bool> flags =
			*flagIter;

		casa::Array<casa::Complex>::const_iterator i = data.begin();
		casa::Array<bool>::const_iterator fI = flags.begin();
		num_t
			xxr = 0.0, xxi = 0.0, xyr = 0.0, xyi = 0.0, yxr = 0.0, yxi = 0.0, yyr = 0.0, yyi = 0.0;
		size_t
			xxc = 0, xyc = 0, yxc = 0, yyc = 0;
		for(size_t f=0;f<(size_t) _frequencyCount;++f) {
			const casa::Complex &xx = *i;
			++i;
			const casa::Complex &xy = *i;
			++i;
			const casa::Complex &yx = *i;
			++i;
			const casa::Complex &yy = *i;
			++i;
			bool xxF = *fI;
			++fI;
			bool xyF = *fI;
			++fI;
			bool yxF = *fI;
			++fI;
			bool yyF = *fI;
			++fI;
			if(std::isfinite(xxr) && std::isfinite(xxi) && !xxF)
			{
				xxr += xx.real();
				xxi += xx.imag();
				++xxc;
			}
			if(std::isfinite(xyr) && std::isfinite(xyi) && !xyF)
			{
				xyr += xy.real();
				xyi += xy.imag();
				++xyc;
			}
			if(std::isfinite(yxr) && std::isfinite(yxi) && !yxF)
			{
				yxr += yx.real();
				yxi += yx.imag();
				++yxc;
			}
			if(std::isfinite(yyr) && std::isfinite(yyi) && !yyF)
			{
				yyr += yy.real();
				yyi += yy.imag();
				++yyc;
			}
		}

		xxRImage->SetValue(a1, a2, xxr / xxc);
		xxIImage->SetValue(a1, a2, xxi / xxc);
		xyRImage->SetValue(a1, a2, xyr / xxc);
		xyIImage->SetValue(a1, a2, xyi / xxc);
		yxRImage->SetValue(a1, a2, yxr / xxc);
		yxIImage->SetValue(a1, a2, yxi / xxc);
		yyRImage->SetValue(a1, a2, yyr / xxc);
		yyIImage->SetValue(a1, a2, yyi / xxc);

		xxMask->SetValue(a1, a2, xxc == 0);
		xyMask->SetValue(a1, a2, xyc == 0);
		yxMask->SetValue(a1, a2, yxc == 0);
		yyMask->SetValue(a1, a2, yyc == 0);

		UVW uvw;
		casa::Array<double> arr = *uvwIter;
		casa::Array<double>::const_iterator uvwArrayIterator = arr.begin();
		uvw.u = *uvwArrayIterator;
		++uvwArrayIterator;
		uvw.v = *uvwArrayIterator;
		++uvwArrayIterator;
		uvw.w = *uvwArrayIterator;
		_metaData->SetUVW(a1, a2, uvw);

		if(a1 != a2)
		{
			xxRImage->SetValue(a2, a1, xxr / xxc);
			xxIImage->SetValue(a2, a1, -xxi / xxc);
			xyRImage->SetValue(a2, a1, xyr / xxc);
			xyIImage->SetValue(a2, a1, -xyi / xxc);
			yxRImage->SetValue(a2, a1, yxr / xxc);
			yxIImage->SetValue(a2, a1, -yxi / xxc);
			yyRImage->SetValue(a2, a1, yyr / xxc);
			yyIImage->SetValue(a2, a1, -yyi / xxc);

			xxMask->SetValue(a2, a1, xxc == 0);
			xyMask->SetValue(a2, a1, xyc == 0);
			yxMask->SetValue(a2, a1, yxc == 0);
			yyMask->SetValue(a2, a1, yyc == 0);

			uvw.u = -uvw.u;
			uvw.v = -uvw.v;
			uvw.w = -uvw.w;
			_metaData->SetUVW(a2, a1, uvw);
		}

		++antenna1Iter;
		++antenna2Iter;
		++dataIter;
		++uvwIter;
		++flagIter;
	}
	casa::ROScalarColumn<int> bandColumn(table, "DATA_DESC_ID");
	BandInfo band = _measurementSet.GetBandInfo(bandColumn(0));
	_metaData->SetFrequency(band.CenterFrequencyHz());

	TimeFrequencyData data(xxRImage, xxIImage, xyRImage, xyIImage, yxRImage, yxIImage, yyRImage, yyIImage);
	data.SetIndividualPolarisationMasks(xxMask, xyMask, yxMask, yyMask);
	return data;
}

void BaselineMatrixLoader::LoadPerChannel(size_t timeIndex, std::vector<TimeFrequencyData> &data)
{
	casa::Block<casa::String> selectionNames(2);
	selectionNames[0] = "DATA_DESC_ID";
	selectionNames[1] = "TIME";
	if(timeIndex < _currentIterIndex)
	{
		delete _tableIter;
		_tableIter = new casa::TableIterator(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
		_currentIterIndex = 0;
	}
	while(!_tableIter->pastEnd() && timeIndex-_currentIterIndex > 0)
	{
		_tableIter->next();
		++_currentIterIndex;
	}
	if(_tableIter->pastEnd())
	{
		throw std::runtime_error("Time index not found");
	}

	casa::Table table = _tableIter->table();
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");

	// Find highest antenna index
	int nrAntenna = 0;
	for(size_t i=0;i<table.nrow();++i)
	{
		int
			a1 = antenna1Column(i),
			a2 = antenna2Column(i);

		if(a1 > nrAntenna)
			nrAntenna = a1;
		if(a2 > nrAntenna)
			nrAntenna = a2;
	}
	++nrAntenna;

	if(_metaData != 0)
		delete _metaData;
	_metaData = new SpatialMatrixMetaData(nrAntenna);

	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
	casa::ROArrayColumn<double> uvwColumn(table, "UVW");

	std::vector<Image2DPtr>
		xxRImage(_frequencyCount), xxIImage(_frequencyCount), xyRImage(_frequencyCount), xyIImage(_frequencyCount),
		yxRImage(_frequencyCount), yxIImage(_frequencyCount), yyRImage(_frequencyCount), yyIImage(_frequencyCount);
	std::vector<Mask2DPtr>
		xxMask(_frequencyCount), xyMask(_frequencyCount), yxMask(_frequencyCount), yyMask(_frequencyCount);
	for(size_t f=0;f<(size_t) _frequencyCount;++f)
	{
		xxRImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		xxIImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		xyRImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		xyIImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yxRImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yxIImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yyRImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna),
		yyIImage[f] = Image2D::CreateZeroImagePtr(nrAntenna, nrAntenna);
		xxMask[f] = Mask2D::CreateSetMaskPtr<true>(nrAntenna, nrAntenna),
		xyMask[f] = Mask2D::CreateSetMaskPtr<true>(nrAntenna, nrAntenna),
		yxMask[f] = Mask2D::CreateSetMaskPtr<true>(nrAntenna, nrAntenna),
		yyMask[f] = Mask2D::CreateSetMaskPtr<true>(nrAntenna, nrAntenna);
	}

	for(size_t row=0;row<table.nrow();++row)
	{
		int
			a1 = antenna1Column(row),
			a2 = antenna2Column(row);
		casa::Array<casa::Complex> data =
			dataColumn(row);
		casa::Array<bool> flags =
			flagColumn(row);

		casa::Array<casa::Complex>::const_iterator i = data.begin();
		casa::Array<bool>::const_iterator fI = flags.begin();
		for(size_t f=0;f<(size_t) _frequencyCount;++f) {
			const casa::Complex xx = *i;
			++i;
			const casa::Complex xy = *i;
			++i;
			const casa::Complex yx = *i;
			++i;
			const casa::Complex yy = *i;
			++i;
			bool xxF = *fI;
			++fI;
			bool xyF = *fI;
			++fI;
			bool yxF = *fI;
			++fI;
			bool yyF = *fI;
			++fI;
			if(!std::isfinite(xx.real()) || !std::isfinite(xx.imag()))
				xxF = true;
			if(!std::isfinite(xy.real()) || !std::isfinite(xy.imag()))
				xyF = true;
			if(!std::isfinite(yx.real()) || !std::isfinite(yx.imag()))
				yxF = true;
			if(!std::isfinite(yy.real()) || !std::isfinite(yy.imag()))
				yyF = true;
			xxRImage[f]->SetValue(a1, a2, xx.real());
			xxIImage[f]->SetValue(a1, a2, xx.imag());
			xyRImage[f]->SetValue(a1, a2, xy.real());
			xyIImage[f]->SetValue(a1, a2, xy.imag());
			yxRImage[f]->SetValue(a1, a2, yx.real());
			yxIImage[f]->SetValue(a1, a2, yx.imag());
			yyRImage[f]->SetValue(a1, a2, yy.real());
			yyIImage[f]->SetValue(a1, a2, yy.imag());

			xxMask[f]->SetValue(a1, a2, !xxF);
			xyMask[f]->SetValue(a1, a2, !xyF);
			yxMask[f]->SetValue(a1, a2, !yxF);
			yyMask[f]->SetValue(a1, a2, !yyF);

			if(a1 != a2)
			{
				xxRImage[f]->SetValue(a2, a1, xx.real());
				xxIImage[f]->SetValue(a2, a1, -xx.imag());
				xyRImage[f]->SetValue(a2, a1, xy.real());
				xyIImage[f]->SetValue(a2, a1, -xy.imag());
				yxRImage[f]->SetValue(a2, a1, yx.real());
				yxIImage[f]->SetValue(a2, a1, -yx.imag());
				yyRImage[f]->SetValue(a2, a1, yy.real());
				yyIImage[f]->SetValue(a2, a1, -yy.imag());
	
				xxMask[f]->SetValue(a2, a1, !xxF);
				xyMask[f]->SetValue(a2, a1, !xyF);
				yxMask[f]->SetValue(a2, a1, !yxF);
				yyMask[f]->SetValue(a2, a1, !yyF);
			}
		}

		UVW uvw;
		casa::Array<double> arr = uvwColumn(row);
		casa::Array<double>::const_iterator uvwArrayIterator = arr.begin();
		uvw.u = *uvwArrayIterator;
		++uvwArrayIterator;
		uvw.v = *uvwArrayIterator;
		++uvwArrayIterator;
		uvw.w = *uvwArrayIterator;
		_metaData->SetUVW(a1, a2, uvw);

		if(a1 != a2)
		{
			uvw.u = -uvw.u;
			uvw.v = -uvw.v;
			uvw.w = -uvw.w;
			_metaData->SetUVW(a2, a1, uvw);
		}
	}
	casa::ROScalarColumn<int> bandColumn(table, "DATA_DESC_ID");
	BandInfo band = _measurementSet.GetBandInfo(bandColumn(0));
	_metaData->SetFrequency(band.CenterFrequencyHz());

	data.clear();
	for(size_t f=0;f<_frequencyCount;++f)
	{
		TimeFrequencyData singleMatrix(xxRImage[f], xxIImage[f], xyRImage[f], xyIImage[f], yxRImage[f], yxIImage[f], yyRImage[f], yyIImage[f]);
		singleMatrix.SetIndividualPolarisationMasks(xxMask[f], xyMask[f], yxMask[f], yyMask[f]);
		data.push_back(singleMatrix);
	}
}

