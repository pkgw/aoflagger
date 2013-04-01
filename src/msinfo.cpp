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
#include <cstdlib>
#include <sstream>
#include <vector>

#include <ms/MeasurementSets/MeasurementSet.h>

#include "msio/measurementset.h"
#include "msio/antennainfo.h"

#include "strategy/algorithms/thresholdtools.h"

using namespace std;

void AddLengths(Mask2DCPtr mask, int *counts, int countSize);

std::string BoolToStr(bool val)
{
	if(val)
		return "true";
	else
		return "false";
}

int main(int argc, char *argv[])
{
	if(argc < 2) {
		cout << "This program will provide you general information about a measurement set.\nUsage:\n\t" << argv[0] << " [options] <measurement set>\n"
		     << endl;
		exit(-1);
	}

#ifdef HAS_LOFARSTMAN
	register_lofarstman();
#endif // HAS_LOFARSTMAN

	//bool saveFlagLength = false;
	//string flagLengthFile;
	bool antennaeList = false;
	int pindex = 1;
	while(pindex < argc && argv[pindex][0] == '-') {
		string parameter = argv[pindex]+1;
		if(parameter == "antennae")
		{
			antennaeList = true;
		} else
		/*if(parameter == "flaglength") {
			saveFlagLength = true;
			flagLengthFile = argv[pindex+1];
			++pindex;
		} else*/
		{
			cerr << "Bad parameter: -" << parameter << endl;
			exit(-1);
		}
		++pindex;
	}
	
	string measurementFile = argv[pindex];
	if(antennaeList)
	{
		MeasurementSet set(measurementFile);
		const size_t antennaCount = set.AntennaCount();
		for(size_t i=0;i<antennaCount;++i)
		{
			cout.width(16);
			cout.precision(16);
			const AntennaInfo antenna = set.GetAntennaInfo(i);
			cout << antenna.id << '\t' << antenna.position.Latitude()*180.0/M_PI << '\t' << antenna.position.Longitude()*180.0/M_PI << '\t' << antenna.position.Altitude() << '\n';
		}
	}
	else {
		cout << "Opening measurementset " << measurementFile << endl; 
		MeasurementSet set(measurementFile);
		size_t antennaCount = set.AntennaCount();
		cout
			<< "Telescope name: " << set.TelescopeName() << '\n'
			<< "Number of antennea: " << antennaCount << '\n'
			<< "Number of time scans: " << set.TimestepCount() << '\n'
			<< "Number of channels/band: " << set.FrequencyCount(0) << '\n'
			<< "Number of fields: " << set.FieldCount() << '\n'
			<< "Number of bands: " << set.BandCount() << '\n';
		casa::Table *table = new casa::MeasurementSet(set.Path());
		cout << "Has DATA column: " << BoolToStr(table->tableDesc().isColumn("DATA")) << "\n";
		cout << "Has CORRECTED_DATA column: " << BoolToStr(table->tableDesc().isColumn("CORRECTED_DATA")) << "\n";
		cout << "Has MODEL_DATA column: " << BoolToStr(table->tableDesc().isColumn("MODEL_DATA")) << "\n";
		delete table;
		std::vector<long double> baselines;
		for(size_t i=0;i<antennaCount;++i)
		{
			cout << "==Antenna " << i << "==\n";
			AntennaInfo info = set.GetAntennaInfo(i);
			cout <<
				"Diameter: " << info.diameter << "\n"
				"Name: " << info.name << "\n"
				"Position: " << info.position.ToString() << "\n"
				"Mount: " << info.mount << "\n"
				"Station: " << info.station << "\n"
				"Providing baselines: ";
			for(size_t j=0;j<antennaCount;++j) {
				AntennaInfo info2 = set.GetAntennaInfo(j);
				Baseline b(info.position, info2.position);
				long double dist = b.Distance();
				long double angle = b.Angle() * 180.0 / M_PIn;
				cout << dist << "m/" << angle << "` ";
				baselines.push_back(dist);
			}
			cout << "\n" << endl;
		}
		sort(baselines.begin(), baselines.end());
		cout << "All provided baselines: ";
		unsigned i=0;
		while(i<baselines.size()-1) {
			if(baselines[i+1]-baselines[i] < 1.0)
				baselines.erase(baselines.begin() + i);
			else
				++i;
		}
		for(vector<long double>::const_iterator i=baselines.begin();i!=baselines.end();++i)
			cout << (*i) << " ";
		cout << endl;

		for(unsigned i=0;i!=set.BandCount();++i) {
			cout << "== Spectral band index " << i << " ==" << endl;
			BandInfo bandInfo = set.GetBandInfo(i);
			cout << "Channel count: " << bandInfo.channels.size() << std::endl;
			cout << "Channels: ";
			for(unsigned j=0;j<bandInfo.channels.size();++j) {
				if(j > 0) cout << ", ";
				cout << round(bandInfo.channels[j].frequencyHz/1000000) << "MHz";
			}
			cout << endl;
		}
		
		for(unsigned i=0;i<set.FieldCount();++i) {
			FieldInfo fieldInfo = set.GetFieldInfo(i);
			cout << "Field " << i << ":\n\tdelay direction=" << fieldInfo.delayDirectionDec << " dec, " << fieldInfo.delayDirectionRA << "ra.\n\tdelay direction (in degrees)=" << (fieldInfo.delayDirectionDec/M_PIn*180.0L) << " dec," << (fieldInfo.delayDirectionRA/M_PIn*180.0L) << " ra." << endl;
		}

		/*
		long unsigned flaggedCount = 0;
		long unsigned sampleCount = 0;
		for(unsigned b=0;b<=set.MaxSpectralBandIndex();++b) {
			cout << "Processing band " << b << "..." << endl;
			int lengthsSize = set.MaxScanIndex()+1;
			int *lengthCounts = new int[lengthsSize];
			for(int i=0;i<lengthsSize;++i)
				lengthCounts[i] = 0;
			MeasurementSet set(measurementFile);
			TimeFrequencyImager imager(set);
			for(unsigned a1=0;a1<set.AntennaCount();++a1) {
				for(unsigned a2=a1;a2<set.AntennaCount();++a2) {
					//std::cout << "A" << a1 << " vs A" << a2 << endl;
					imager.SetReadData(false);
					imager.SetReadFlags(true);
					try {
						imager.Image(a1, a2, b);
					} catch(std::exception &e) {
						std::cerr << "Error reading baseline " << a1 << " x " << a2 << ": " << e.what() << std::endl;
					}
					if(imager.HasFlags()) {
						long unsigned thisCount;
						long unsigned thisSamples;

						thisCount = imager.FlagXX()->GetCount<true>();
						thisSamples = imager.FlagXX()->Width()*imager.FlagXX()->Height();

						thisCount += imager.FlagXY()->GetCount<true>();
						thisSamples += imager.FlagXY()->Width()*imager.FlagXY()->Height();

						thisCount += imager.FlagYX()->GetCount<true>();
						thisSamples += imager.FlagYX()->Width()*imager.FlagYX()->Height();

						thisCount += imager.FlagYY()->GetCount<true>();
						thisSamples += imager.FlagYY()->Width()*imager.FlagYY()->Height();

					if(thisCount == 0 || round(1000.0L * (long double) thisCount / thisSamples)*0.1L == 100.0L) {
							//cout << " (ignored)";
						} else {
							
							cout << "Flagging for antenna " << set.GetAntennaInfo(a1).name
								<< " x " << set.GetAntennaInfo(a2).name << ": "
								<< 0.1L * round(1000.0L * (long double) thisCount / thisSamples) << "%" << endl;
							flaggedCount += thisCount;
							sampleCount += thisSamples;
							if(saveFlagLength)
							{
								AddLengths(imager.FlagXX(), lengthCounts, lengthsSize);
								AddLengths(imager.FlagXY(), lengthCounts, lengthsSize);
								AddLengths(imager.FlagYX(), lengthCounts, lengthsSize);
								AddLengths(imager.FlagYY(), lengthCounts, lengthsSize);
							}
						}
					}
				}
			}
			cout << "Total percentage RFI in this band: " << round(flaggedCount*1000.0/sampleCount)/10.0 << "% "
					<< "(" << flaggedCount << " / " << sampleCount << ")" << endl;
			
			if(saveFlagLength) {
				num_t *data = new num_t[lengthsSize];
				for(int i=0;i<lengthsSize;++i)
					data[i] = lengthCounts[i];
				if(set.FrequencyCount() < (set.MaxScanIndex()+1) && set.FrequencyCount()>2 && data[set.FrequencyCount()-1] > data[set.FrequencyCount()-2])
					data[set.FrequencyCount()-1] = data[set.FrequencyCount()-2];
				ThresholdTools::OneDimensionalGausConvolution(data, lengthsSize, 200.0);
				ofstream f(flagLengthFile.c_str());
				for(int i = 0; i<lengthsSize;++i) {
					f << (i+1) << "\t" << lengthCounts[i] << "\t" << data[i] << endl;
				}
				f.close();
			}
			delete[] lengthCounts;
		}*/
		return EXIT_SUCCESS;
	}
}

void AddLengths(Mask2DCPtr mask, int *counts, int countSize)
{
	int *countTmp = new int[countSize];
	ThresholdTools::CountMaskLengths(mask, countTmp, countSize);
	for(int i=0;i<countSize;++i)
		counts[i] += countTmp[i];
	delete[] countTmp; 
}
