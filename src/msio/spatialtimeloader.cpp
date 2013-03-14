#include "spatialtimeloader.h"

#include <stdexcept>
#include <vector>

#include <tables/Tables/ExprNode.h>

#include "arraycolumniterator.h"
#include "scalarcolumniterator.h"

#include "../util/aologger.h"

SpatialTimeLoader::SpatialTimeLoader(MeasurementSet &measurementSet)
	:  _measurementSet(measurementSet), _sortedTable(0), _tableIter(0)
{
	casa::Table *rawTable = _measurementSet.OpenTable();
	casa::Block<casa::String> names(4);
	names[0] = "DATA_DESC_ID";
	names[1] = "TIME";
	names[2] = "ANTENNA1";
	names[3] = "ANTENNA2";
	_sortedTable = new casa::Table(rawTable->sort(names));
	delete rawTable;

	_channelCount = _measurementSet.FrequencyCount();
	_timestepsCount = _measurementSet.MaxScanIndex();
	_antennaCount = _measurementSet.AntennaCount();
	_polarizationCount = _measurementSet.GetPolarizationCount();

	casa::Block<casa::String> selectionNames(1);
	selectionNames[0] = "DATA_DESC_ID";
	_tableIter = new casa::TableIterator(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
}

SpatialTimeLoader::~SpatialTimeLoader()
{
	if(_sortedTable != 0)
		delete _sortedTable;
	delete _tableIter;
}

TimeFrequencyData SpatialTimeLoader::Load(unsigned channelIndex, bool fringeStop)
{
	const unsigned baselineCount = _antennaCount * (_antennaCount-1) / 2;
	
	casa::Table table = _tableIter->table();
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");
	casa::ROScalarColumn<double> timeColumn(table, "TIME");
	casa::ROArrayColumn<double> uvwColumn(table, "UVW");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");

	std::vector<Image2DPtr>
		realImages(_polarizationCount),
		imagImages(_polarizationCount);
	std::vector<Mask2DPtr>
		masks(_polarizationCount);
	for(unsigned p=0;p<_polarizationCount;++p)
	{
		realImages[p] = Image2D::CreateUnsetImagePtr(_timestepsCount, baselineCount);
		imagImages[p] = Image2D::CreateUnsetImagePtr(_timestepsCount, baselineCount);
		masks[p] = Mask2D::CreateUnsetMaskPtr(_timestepsCount, baselineCount);
	}
	
	ChannelInfo channelInfo = _measurementSet.GetBandInfo(0).channels[channelIndex];
	
	unsigned timeIndex = 0;
	double lastTime = timeColumn(0);
	for(unsigned row=0;row<table.nrow();++row)
	{
		const int
			a1 = antenna1Column(row),
			a2 = antenna2Column(row);
		const double
			time = timeColumn(row);
		if(time != lastTime)
		{
			timeIndex++;
			lastTime = time;
		}
		
		if(a1 != a2)
		{
			const casa::Array<casa::Complex> data = dataColumn(row);
			const casa::Array<bool> flags = flagColumn(row);
			const casa::Array<double> uvws = uvwColumn(row);

			casa::Array<casa::Complex>::const_iterator i = data.begin();
			casa::Array<bool>::const_iterator fI = flags.begin();
			casa::Array<double>::const_iterator uvwIter = uvws.begin();
			++uvwIter; ++uvwIter;
			const double wRotation = -channelInfo.MetersToLambda(*uvwIter) * M_PI * 2.0;
			
			unsigned baselineIndex = baselineCount - (_antennaCount-a1)*(_antennaCount-a1-1)/2+a2-a1-1;

			for(unsigned c=0;c<_channelCount;++c) {
				if(c == channelIndex)
				{
					AOLogger::Debug << "Reading timeIndex=" << timeIndex << ", baselineIndex=" << baselineIndex << ", a1=" << a1 << ", a2=" << a2 << ",w=" << wRotation << "\n";
					for(unsigned p=0;p<_polarizationCount;++p) {
						double realValue = i->real();
						double imagValue = i->imag();
						if(fringeStop)
						{
							double newRealValue = realValue * cosn(wRotation) - imagValue * sinn(wRotation);
							imagValue = realValue * sinn(wRotation) + imagValue * cosn(wRotation);
							realValue = newRealValue;
						}
						realImages[p]->SetValue(timeIndex, baselineIndex, realValue);
						imagImages[p]->SetValue(timeIndex, baselineIndex, imagValue);
						++i;
						masks[p]->SetValue(timeIndex, baselineIndex, *fI);
						++fI;
					}
				} else {
					for(unsigned p=0;p<_polarizationCount;++p) {
						++i;
						++fI;
					}
				}
			}
		}
	}
	casa::ROScalarColumn<int> bandColumn(table, "DATA_DESC_ID");
	const BandInfo band = _measurementSet.GetBandInfo(bandColumn(0));

	TimeFrequencyData data;
	if(_polarizationCount == 4)
	{
		data = TimeFrequencyData(realImages[0], imagImages[0], realImages[1], imagImages[1], realImages[2], imagImages[2], realImages[3], imagImages[3]);
		data.SetIndividualPolarisationMasks(masks[0], masks[1], masks[2], masks[3]);
	} else if(_polarizationCount == 2)
	{
		data = TimeFrequencyData(AutoDipolePolarisation, realImages[0], imagImages[0], realImages[1], imagImages[1]);
		data.SetIndividualPolarisationMasks(masks[0], masks[1]);
	} else if(_polarizationCount == 1)
	{
		data = TimeFrequencyData(SinglePolarisation, realImages[0], imagImages[0]);
		data.SetGlobalMask(masks[0]);
	} else {
		throw std::runtime_error("Unknown number of polarizations!");
	}
	return data;
}

