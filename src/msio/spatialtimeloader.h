#ifndef SPATIALTIMELOADER_H
#define SPATIALTIMELOADER_H

#include <cstring>

#include <tables/Tables/TableIter.h>

#include "timefrequencydata.h"
#include "measurementset.h"

class SpatialTimeLoader
{
	public:
		explicit SpatialTimeLoader(MeasurementSet &measurementSet);
		~SpatialTimeLoader();

		TimeFrequencyData Load(unsigned channelIndex, bool fringeStop = true);

		unsigned ChannelCount() const { return _channelCount; }
		
		unsigned TimestepsCount() const { return _timestepsCount; }
	private:
		MeasurementSet &_measurementSet;
		casa::Table *_sortedTable;
		casa::TableIterator *_tableIter;
		unsigned _channelCount;
		unsigned _timestepsCount;
		unsigned _antennaCount;
		unsigned _polarizationCount;
};

#endif
