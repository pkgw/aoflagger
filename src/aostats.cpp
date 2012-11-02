
#include <iostream>
#include <cmath>

#include <ms/MeasurementSets/MSTable.h>
#include <ms/MeasurementSets/MSColumns.h>

#include "msio/measurementset.h"
#include "strategy/algorithms/rfistatistics.h"

using namespace std;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		cerr << "Usage: " << argv[0] << " <MS>\n";
	} else {
		std::string msPath = argv[1];
		
		MeasurementSet ms(msPath);
		const BandInfo band = ms.GetBandInfo(0);
		
		const unsigned polarizationCount = ms.GetPolarizationCount();
		
		casa::Table table(msPath);
		casa::ROArrayColumn<casa::Complex> dataCol(table, "DATA");
		casa::ROArrayColumn<bool> flagCol(table, "FLAG");
		casa::ROScalarColumn<double> timeCol(table, "TIME");
		casa::ROScalarColumn<int> antenna1Col(table, "ANTENNA1");
		casa::ROScalarColumn<int> antenna2Col(table, "ANTENNA2");
		
		RFIStatistics statistics;
		
		for(size_t row=0;row<table.nrow();++row)
		{
			size_t antenna1 = antenna1Col(row);
			size_t antenna2 = antenna2Col(row);
			if(antenna1 != antenna2)
			{
				double time = timeCol(row);
				casa::Array<casa::Complex> dataArray = dataCol(row);
				casa::Array<bool> flagsArray = flagCol(row);
				casa::Array<casa::Complex>::const_iterator dataIterator = dataArray.begin();
				casa::Array<bool>::const_iterator flagIterator = flagsArray.begin();
				
				RFIStatistics::TimeFrequencyInfo tfInfo;
				tfInfo.time = time;
				tfInfo.centralFrequency = band.CenterFrequencyHz();
				for(unsigned freqIndex = 0;freqIndex < band.channels.size();++freqIndex)
				{
					for(unsigned p=0;p<polarizationCount;++p)
					{
						const bool flag = *flagIterator;
						const casa::Complex data = *dataIterator;
						const double amplitude = sqrt((double) data.imag()*data.imag() + data.real()*data.real());
						if(std::isfinite(amplitude))
						{
							if(flag)
							{
								++tfInfo.rfiCount;
								tfInfo.rfiAmplitude += amplitude;
							}
							++tfInfo.totalCount;
							tfInfo.totalAmplitude += amplitude;
						}
						++flagIterator;
						++dataIterator;
					}
				}
				statistics.Add(tfInfo, false);
			}
		}
		statistics.Save();
	}
}
