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
#include <fstream>
#include <stdexcept>
#include <map>
#include <cmath>
#include <iomanip>

#include "msio/date.h"

#include "strategy/algorithms/rfistatistics.h"
#include "strategy/algorithms/noisestatistics.h"
#include "strategy/algorithms/noisestatisticscollector.h"

#include "util/rng.h"

using namespace std;

void readChannels(RFIStatistics &statistics, string &filename, bool autocorrelation)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::ChannelInfo channel;
		f
		>> channel.frequencyHz;
		if(f.eof()) break;
		f
		>> channel.totalCount
		>> channel.totalAmplitude
		>> channel.rfiCount
		>> channel.rfiAmplitude
		>> channel.broadbandRfiCount
		>> channel.lineRfiCount
		>> channel.broadbandRfiAmplitude
		>> channel.lineRfiAmplitude
		>> channel.falsePositiveCount
		>> channel.falseNegativeCount
		>> channel.truePositiveCount
		>> channel.trueNegativeCount
		>> channel.falsePositiveAmplitude
		>> channel.falseNegativeAmplitude;
		statistics.Add(channel, autocorrelation);
	}
}

void readTimesteps(RFIStatistics &statistics, string &filename, bool autocorrelation)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::TimestepInfo timestep;
		f
		>> timestep.time;
		if(f.eof()) break;
		f
		>> timestep.totalCount
		>> timestep.totalAmplitude
		>> timestep.rfiCount
		>> timestep.rfiAmplitude
		>> timestep.broadbandRfiCount
		>> timestep.lineRfiCount
		>> timestep.broadbandRfiAmplitude
		>> timestep.lineRfiAmplitude;
		statistics.Add(timestep, autocorrelation);
	}
}

void readAmplitudes(RFIStatistics &statistics, string &filename, bool autocorrelation)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	double maxCount = 0.0;
	double maxAmp = 0.0;
	while(f.good())
	{
		double centralLogAmplitude;
		RFIStatistics::AmplitudeBin amplitude;
		f
		>> amplitude.centralAmplitude;
		if(!f.good()) break;
		f
		>> centralLogAmplitude
		>> amplitude.count
		>> amplitude.rfiCount
		>> amplitude.broadbandRfiCount
		>> amplitude.lineRfiCount
		>> amplitude.featureAvgCount
		>> amplitude.featureIntCount
		>> amplitude.featureMaxCount
		>> amplitude.xxCount
		>> amplitude.xyCount
		>> amplitude.yxCount
		>> amplitude.yyCount
		>> amplitude.xxRfiCount
		>> amplitude.xyRfiCount
		>> amplitude.yxRfiCount
		>> amplitude.yyRfiCount
		>> amplitude.stokesQCount
		>> amplitude.stokesUCount
		>> amplitude.stokesVCount
		>> amplitude.falsePositiveCount
		>> amplitude.falseNegativeCount
		>> amplitude.truePositiveCount
		>> amplitude.trueNegativeCount;
		statistics.Add(amplitude, autocorrelation);
		if(amplitude.count/amplitude.centralAmplitude > maxCount)
		{
			maxCount = amplitude.count/amplitude.centralAmplitude;
			maxAmp = amplitude.centralAmplitude;
		}
	}
	std::cout << std::setprecision(14) << " mode~=" << maxAmp << '(' << maxCount << ')' << '\n';
}

void readBaselines(RFIStatistics &statistics, string &filename)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::BaselineInfo baseline;
		f
		>> baseline.antenna1;
		if(f.eof()) break;
		f
		>> baseline.antenna2
		>> baseline.antenna1Name
		>> baseline.antenna2Name
		>> baseline.baselineLength
		>> baseline.baselineAngle
		>> baseline.count
		>> baseline.totalAmplitude
		>> baseline.rfiCount
		>> baseline.broadbandRfiCount
		>> baseline.lineRfiCount
		>> baseline.rfiAmplitude
		>> baseline.broadbandRfiAmplitude
		>> baseline.lineRfiAmplitude;
		statistics.Add(baseline);
	}
}

void readBaselineTime(RFIStatistics &statistics, string &filename)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::BaselineTimeInfo info;
		f
		>> info.antenna1Index;
		if(f.eof()) break;
		f
		>> info.antenna2Index
		>> info.time
		>> info.totalCount
		>> info.rfiCount;
		statistics.Add(info);
	}
}

void readBaselineFrequency(RFIStatistics &statistics, string &filename)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::BaselineFrequencyInfo info;
		f
		>> info.antenna1Index;
		if(f.eof()) break;
		f
		>> info.antenna2Index
		>> info.centralFrequency
		>> info.totalCount
		>> info.rfiCount;
		statistics.Add(info);
	}
}

void readTimeFrequency(RFIStatistics &statistics, string &filename, bool autocorrelation)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::TimeFrequencyInfo info;
		f
		>> info.time;
		if(f.eof()) break;
		f
		>> info.centralFrequency
		>> info.totalCount
		>> info.rfiCount
		>> info.totalAmplitude
		>> info.rfiAmplitude;
		statistics.Add(info, autocorrelation);
	}
}

void fitGaus(RFIStatistics &statistics)
{
	const std::map<double, class RFIStatistics::AmplitudeBin> &amplitudes = statistics.GetCrossAmplitudes();
	
	std::map<double, long unsigned> distribution;
	for(std::map<double, class RFIStatistics::AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
	{
		distribution.insert(std::pair<double, long unsigned>(i->first, i->second.featureAvgCount));
	}
	if(!distribution.empty())
	{
		// Find largest value
		long unsigned max = distribution.begin()->second;
		double ampOfMax = distribution.begin()->first;
		for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
		{
			if(i->second > max) {
				max = i->second;
				ampOfMax = i->first;
			}
		}
		std::cout << "Maximum occurring amplitude=" << ampOfMax << std::endl;
		std::cout << "Count=" << max << std::endl;
		double promileArea = 0.0;
		double promileLimit = max / 1000.0;
		long unsigned popSize = 0;
		for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
		{
			if(i->second > promileLimit) {
				promileArea += i->second;
				//promileEnd = i->first;
				//if(promileStart == 0.0)
				//	promileStart = i->first;
			}
			popSize += i->second;
		}
		double halfPromileArea = promileArea / 2.0;
		double mean = 0.0;
		promileArea = 0.0;
		for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
		{
			if(i->second > promileLimit) {
				promileArea += i->second;
			}
			if(promileArea > halfPromileArea)
			{
				mean = i->first;
				break;
			}
		}
		std::cout << "Mean=" << mean << std::endl;
		double halfStddevArea = 0.682689492137 * halfPromileArea;
		double stddev = 0.0;
					// Note: need a cast to const for older compilers
		for(std::map<double, long unsigned>::const_reverse_iterator i=distribution.rbegin();i!=static_cast<const std::map<double, long unsigned> >(distribution).rend();++i)
		{
			if(i->first <= mean) {
				halfStddevArea -= i->second;
			}
			if(halfStddevArea <= 0.0)
			{
				stddev = i->first;
				break;
			}
		}
		std::cout << "Stddev=" << stddev << std::endl;

		ofstream f("fit.txt");
		f
		<< setprecision(15)
		<< "Amplitude\tLogAmplitude\tCount\tCount\tGaussian\tGaussian\tRayleigh\tRayleigh\n";
		for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
		{
			if(i != distribution.begin())
			{
				double g = RNG::EvaluateGaussian(i->first - mean, stddev)*popSize;
				double r = RNG::EvaluateRayleigh(i->first, mean)*popSize;
				double binsize = i->first / 100.0;
				g *= binsize;
				r *= binsize;
				f
				<< i->first << '\t' << log10(i->first) << '\t'
				<< i->second << '\t' << log10(i->second) << '\t'
				<< g << '\t' << log10(g) << '\t' << r << '\t' << log10(r) << '\n';
			}
		}
	}
}

void Save(NoiseStatisticsCollector &stats, const std::string baseName)
{
	if(!stats.Empty())
	{
		stats.SaveTA(baseName + "-ta.txt");
		stats.SaveTF(baseName + "-tf.txt");
		stats.SaveTimeAntennaPlot(baseName + "-plotdata.txt", baseName + "-timestation.plt");
	}
}

int main(int argc, char **argv)
{
	cout << 
		"RFI statistics collector\n"
		"This program will collect statistics of several rficonsole runs and\n"
		"write them in one file.\n\n"
		"Author: André Offringa (offringa@astro.rug.nl)\n"
		<< endl;

	if(argc == 1)
	{
		std::cerr << "Usage: " << argv[0] << " [-c <N>] [-i] [files]" << std::endl;
	}
	else
	{
		int argStart = 1;
		int channelCount = 256;
		bool ignoreFirst = true;
		std::string argString(argv[argStart]);
		while(argString.size()>0 && argString[0]=='-')
		{
			if(argString == "-c")
			{
				argStart++;
				channelCount = atoi(argv[argStart]);
			}
			else if(argString == "-i")
				ignoreFirst = false;
			else {
				std::cerr << "Wrong option: " << argString << "\n";
				exit(-1);
			}
			++argStart;
			if(argStart < argc)
				argString = argv[argStart];
			else {
				std::cerr << "No files specified\n";
				exit(-1);
			}
		}

		ofstream amplitudeSlopeFile("amplitudeSlopes.txt");
		amplitudeSlopeFile << "0.01-0.1\t10-100\tcount\n";
		std::map<double, double> frequencyFlags;
		std::map<double, long unsigned> timeTotalCount, timeFlagsCount;
		RFIStatistics statistics;
		statistics.SetIgnoreFirstChannel(ignoreFirst);
		statistics.SetChannelCountPerSubband(channelCount);
		
		NoiseStatisticsCollector noise0, noise1, noise2, noise4, noise8;

		for(int i=argStart;i<argc;++i)
		{
			string filename = argv[i];
			cout << "Reading " << filename << "..." << endl;
			if(filename.find("counts-channels-auto.txt")!=string::npos)
				readChannels(statistics, filename, true);
			else if(filename.find("counts-channels-cross.txt")!=string::npos)
				readChannels(statistics, filename, false);
			else if(filename.find("counts-timesteps-auto.txt")!=string::npos)
				readTimesteps(statistics, filename, true);
			else if(filename.find("counts-timesteps-cross.txt")!=string::npos)
				readTimesteps(statistics, filename, false);
			else if(filename.find("counts-amplitudes-auto.txt")!=string::npos)
				readAmplitudes(statistics, filename, true);
			else if(filename.find("counts-amplitudes-cross.txt")!=string::npos)
			{
				readAmplitudes(statistics, filename, false);
				RFIStatistics single;
				readAmplitudes(single, filename, false);
				amplitudeSlopeFile
				<< single.AmplitudeCrossSlope(0.01, 0.1) << '\t'
				<< single.AmplitudeCrossSlope(10.0, 100.0) << '\t'
				<< single.AmplitudeCrossCount(10.0, 100.0) << '\n';
			}
			else if(filename.find("counts-baselines.txt")!=string::npos)
				readBaselines(statistics, filename);
			else if(filename.find("counts-baseltime.txt")!=string::npos)
				readBaselineTime(statistics, filename);
			else if(filename.find("counts-baselfreq.txt")!=string::npos)
				readBaselineFrequency(statistics, filename);
			else if(filename.find("counts-timefreq-auto.txt")!=string::npos)
				readTimeFrequency(statistics, filename, true);
			else if(filename.find("counts-timefreq-cross.txt")!=string::npos)
				readTimeFrequency(statistics, filename, false);
			else if(filename.find("counts-subbands-auto.txt")!=string::npos)
				; // skip
			else if(filename.find("counts-subbands-cross.txt")!=string::npos)
				; // skip
			else if(filename.find("counts-timeint-auto.txt")!=string::npos)
				; // skip
			else if(filename.find("counts-timeint-cross.txt")!=string::npos)
				; // skip
			else if(filename.find("counts-obaselines.txt")!=string::npos)
				; // skip
			else if(filename.find("counts-stationstime.txt")!=string::npos)
				; // skip
			else if(filename.find("noise-statistics1-ta.txt")!=string::npos)
				noise1.ReadTA(filename);
			else if(filename.find("noise-statistics1-tf.txt")!=string::npos)
				noise1.ReadTF(filename);
			else if(filename.find("noise-statistics2-ta.txt")!=string::npos)
				noise2.ReadTA(filename);
			else if(filename.find("noise-statistics2-tf.txt")!=string::npos)
				noise2.ReadTF(filename);
			else if(filename.find("noise-statistics4-ta.txt")!=string::npos)
				noise4.ReadTA(filename);
			else if(filename.find("noise-statistics4-tf.txt")!=string::npos)
				noise4.ReadTF(filename);
			else if(filename.find("noise-statistics8-ta.txt")!=string::npos)
				noise8.ReadTA(filename);
			else if(filename.find("noise-statistics8-tf.txt")!=string::npos)
				noise8.ReadTF(filename);
			else if(filename.find("noise-statistics")!=string::npos)
			{
				if(filename.find("-ta.txt")!=string::npos)
					noise0.ReadTA(filename);
				else if(filename.find("-tf.txt")!=string::npos)
					noise0.ReadTF(filename);
				else
					throw runtime_error("Could not determine type of noise file.");
			}
			else
				throw runtime_error("Could not determine type of file.");
		}
		fitGaus(statistics);
		statistics.Save();
		std::cout << "Cross correlations: "
		<< (round(statistics.RFIFractionInCrossChannels()*10000)/100) << "% RFI in channels, "
		<< (round(statistics.RFIFractionInCrossTimeSteps()*10000)/100) << "% RFI in timesteps.\n"
		<< "Auto correlations: "
		<< (round(statistics.RFIFractionInAutoChannels()*10000)/100) << "% RFI in channels, "
		<< (round(statistics.RFIFractionInAutoTimeSteps()*10000)/100) << "% RFI in timesteps.\n"
		<< std::setprecision(14)
		<< "Cross correlation slope fit between 0.01 and 0.1 amplitude: "
		<< statistics.AmplitudeCrossSlope(0.01, 0.1) << "\n"
		<< "Cross correlation slope fit between 10 and 100 amplitude: "
		<< statistics.AmplitudeCrossSlope(10.0, 100.0) << "\n";
		Save(noise0, "noise-statistics");
		Save(noise1, "noise-statistics1");
		Save(noise2, "noise-statistics2");
		Save(noise4, "noise-statistics4");
		Save(noise8, "noise-statistics8");
	}
}
