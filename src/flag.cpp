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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <deque>
#include <unistd.h> // for sleep()

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "msio/measurementset.h"
#include "msio/image2d.h"
#include "msio/mask2d.h"
#include "msio/pngfile.h"
#include "msio/timefrequencyimager.h"

#include "rfi/localfitmethod.h"
#include "rfi/methoditerator.h"
#include "rfi/mitigationtester.h"
#include "rfi/thresholdconfig.h"
#include "rfi/tiledimage.h"

#include "util/integerdomain.h"
#include "util/ffttools.h"

#include "types.h"

#include "rfi/svdmitigater.h" //needs to be last

using namespace std;

enum FlagMethod { Remove, LocalGaussianWeightedAvg, Svd };
enum ThresholdMethod { SumThreshold, VarThreshold };

struct Parameters {
	FlagMethod method;
	ThresholdMethod thresholdMethod;
	int svdRemoveCount;
	long double falsePositiveProbability;
	long double threshold;
	bool useThreshold;
	bool useOnlyAutoCorrelations;
	bool useAbsoluteValues;
	vector<string> sets;
	bool verbose;
	int threads;
	int fittingIterations;
	bool saveImages;
	bool writeStats;
	unsigned band;
	bool joinFlags;
	bool joinPoralizations;
	ThresholdConfig thresholdConfig;
};

Mask2DPtr Perform(Parameters &parameters, Image2DCPtr real, Image2DCPtr imaginary, Mask2DCPtr flag, const std::string &subname, const BandInfo &bandInfo);

struct WorkThreadFunc {
	Parameters parameters;
	deque<unsigned> *workList;
	boost::mutex *_heavyIOMutex;
	boost::mutex *_sharedMemoryMutex;
	IntegerDomain *antenna1Domain, *antenna2Domain;
	string msFilename;
	unsigned threadId;

	void operator()() {
		std::cout << "Spawned new thread (id=" << threadId << ")." << endl;
		boost::mutex::scoped_lock lock(*_sharedMemoryMutex);
		while(!workList->empty()) {
			unsigned antenna = workList->front();
			workList->pop_front();
			lock.unlock();
			PerformTask(antenna);
			lock.lock();
		}
		lock.unlock();
		std::cout << "Thread finished (id=" << threadId << ")." << endl;
	}

	void PerformTask(unsigned antenna)
	{
		TimeFrequencyImager imager;
		imager.SetReadFlags(true);
		boost::mutex::scoped_lock heavyIOLock(*_heavyIOMutex);
		heavyIOLock.unlock();
		cout << "|\n| Now processing autocorrelation of antenna " << antenna << "\n|" << endl;

		cout << "Reading measurement set(s)... (antenna " << antenna << ")" << endl;
		for(vector<string>::const_iterator i=parameters.sets.begin();i!=parameters.sets.end();++i) {
			heavyIOLock.lock();
			imager.SetMSFile(*i);
			imager.Image(antenna, antenna, parameters.band);
			heavyIOLock.unlock();
		}
		cout << endl;

		cout << "Processing " << antenna << ":XX..." << endl;
		stringstream subname1;
		subname1 << "xx-a" << antenna;
		Mask2DPtr maskXX = Perform(parameters, imager.RealXX(), imager.ImaginaryXX(), imager.FlagXX(), subname1.str(), imager.BandInfo());

		cout << "Processing " << antenna << ":XY..." << endl;
		stringstream subname2;
		subname2 << "xy-a" << antenna;
		Mask2DPtr maskXY = Perform(parameters, imager.RealXY(), imager.ImaginaryXY(), imager.FlagXY(), subname2.str(), imager.BandInfo());

		cout << "Processing " << antenna << ":YY..." << endl;
		stringstream subname3;
		subname3 << "yy-a" << antenna;
		Mask2DPtr maskYY = Perform(parameters, imager.RealYY(), imager.ImaginaryYY(), imager.FlagYY(), subname3.str(), imager.BandInfo());

		if(parameters.joinPoralizations)
		{
			cout << "Combining masks between polarisations..." << endl;
			maskXX->Join(maskXY);
			maskXX->Join(maskYY);
			(*maskXY) = maskXX;
			(*maskYY) = maskXX;
		}

		if(parameters.joinFlags) {
			cout << "Combining masks with original..." << endl;
			maskXX->Join(imager.FlagXX());
			maskXY->Join(imager.FlagXY());
			maskYY->Join(imager.FlagYY());
		}

		double oldFlagRatio = (
				(double) imager.FlagXX()->GetCount<true>() +
				imager.FlagXY()->GetCount<true>()*2.0 +
				imager.FlagYY()->GetCount<true>())
			/ (imager.FlagXX()->Width() * imager.FlagXX()->Height() * 4.0);
		double joinFlagRatio = round((
				(double) maskXX->GetCount<true>() +
				maskXY->GetCount<true>()*2.0 +
				maskYY->GetCount<true>())
			/ (maskXX->Width() * maskXX->Height() * 4.0)* 1000.0) / 10.0;
		cout << "Writing flags (" << joinFlagRatio << "% flagged, was previously " << (round(oldFlagRatio * 1000.0) / 10.0) << "%) ..." << endl;
		imager.SetMSFile(msFilename);
		for(unsigned a1index=0;a1index<antenna1Domain->ValueCount();++a1index) {
			for(unsigned a2index=0;a2index<antenna2Domain->ValueCount();++a2index) {
				unsigned a1 = antenna1Domain->GetValue(a1index);
				unsigned a2 = antenna2Domain->GetValue(a2index);
				if(a1 == antenna || a2 == antenna) {
					cout << "Joining flags with flagging of antenna " << a1 << " and " << a2 << "..." << endl;
					heavyIOLock.lock();
					imager.Image(a1, a2, parameters.band);
					double oldFlagRatio = round((
							(double) imager.FlagXX()->GetCount<true>() +
							imager.FlagXY()->GetCount<true>() +
							imager.FlagXY()->GetCount<true>() +
							imager.FlagYY()->GetCount<true>())
						/ (imager.FlagXX()->Width() * imager.FlagXX()->Height() * 4.0) * 1000.0) / 10.0;

					Mask2DPtr joinedMaskXX = Mask2D::CreateCopy(maskXX);
					Mask2DPtr joinedMaskXY = Mask2D::CreateCopy(maskXY);
					Mask2DPtr joinedMaskYY = Mask2D::CreateCopy(maskYY);

					joinedMaskXX->Join(imager.FlagXX());
					joinedMaskXY->Join(imager.FlagXY());
					joinedMaskYY->Join(imager.FlagYY());

					double newFlagRatio = round((
							(double) joinedMaskXX->GetCount<true>() +
							joinedMaskXY->GetCount<true>()*2.0 +
							joinedMaskYY->GetCount<true>())
						/ (joinedMaskXX->Width() * joinedMaskXX->Height() * 4.0) * 1000.0) / 10.0;
					imager.WriteNewFlags(joinedMaskXX, joinedMaskXY, joinedMaskXY, joinedMaskYY);
					heavyIOLock.unlock();

					cout << "A" << a1 << "<->A" << a2 << " was " << oldFlagRatio << "% flagged, joined with " << joinFlagRatio << "% flagged mask, now " << newFlagRatio << "% flagged." << endl;
				}
			}
		}
	}
};

int main(int argc, char *argv[])
{
	Parameters parameters;
	parameters.method = LocalGaussianWeightedAvg;
	parameters.thresholdMethod = SumThreshold;
	parameters.svdRemoveCount = 5;
	parameters.falsePositiveProbability = 0.001;
	parameters.threshold = 3.0;
	parameters.useThreshold = false;
	parameters.useOnlyAutoCorrelations = false;
	parameters.useAbsoluteValues = true;
	parameters.verbose = false;
	parameters.threads = 2;
	parameters.fittingIterations = 5;
	parameters.saveImages = false;
	parameters.writeStats = false;
	parameters.band = 0;
	parameters.joinFlags = true;
	parameters.joinPoralizations = true;

	int pindex = 1;
	bool joinFlagsSet = false;
	while(pindex < argc && argv[pindex][0] == '-') {
		string parameter = argv[pindex]+1;
		if(parameter == "m0") parameters.method = Remove;
		else if(parameter == "m1") parameters.method = LocalGaussianWeightedAvg;
		else if(parameter == "svd") { parameters.svdRemoveCount = atoi(argv[pindex+1]); ++pindex; }
		else if(parameter == "i") { parameters.fittingIterations = atoi(argv[pindex+1]); ++pindex; }
		else if(parameter == "t") {
			parameters.threshold = atof(argv[pindex+1]);
			parameters.useThreshold = true;
			++pindex;
		}
		else if(parameter == "p") {
			parameters.falsePositiveProbability = atof(argv[pindex+1]);
			parameters.useThreshold = false;
			++pindex;
		}
		else if(parameter == "j") { parameters.threads = atoi(argv[pindex+1]); ++pindex; }
		else if(parameter == "m2") parameters.method = Svd;
		else if(parameter == "f") { parameters.joinFlags = false; joinFlagsSet = true; }
		else if(parameter == "fp") { parameters.joinPoralizations = false; }
		else if(parameter == "a") parameters.useOnlyAutoCorrelations = true;
		else if(parameter == "ri") parameters.useAbsoluteValues = false;
		else if(parameter == "v") parameters.verbose = true;
		else if(parameter == "si") parameters.saveImages = true;
		else if(parameter == "ss") parameters.writeStats = true;
		else if(parameter == "ms") { parameters.sets.push_back(argv[pindex+1]); ++pindex; }
		else if(parameter == "tm1") parameters.thresholdMethod = SumThreshold;
		else if(parameter == "tm2") parameters.thresholdMethod = VarThreshold;
		else {
			cerr << "Unknown parameter: -" << parameter << endl;
			return -1;
		}
		++pindex;
	}
	if(!joinFlagsSet) parameters.joinFlags = ((!parameters.useOnlyAutoCorrelations) && parameters.method != Remove);

	if(argc-pindex < 4) {
		cerr << "Syntax:\n\t" << argv[0] << " [options] <ms out> <antenna1 domain> <antenna2 domain> <band>\n"
				 << "options:\n"
			   << " -m<n>    : method to use for flagging (0: remove flags, 1: local gaussian weighted average, 2: svd).\n"
		     << " -svd <i> : number of singular values to remove when using the svd method.\n"
		     << " -f       : don't combine with original flags; previous flagging in measurement set will be removed.\n"
				 << " -fp      : don't join flags of polarizations.\n"
		     << " -t <t>   : set threshold to t * sigma.\n"
		     << " -p <p>   : set theoretical false positive rate to p.\n"
				 << " -ms <ms> : add a measurement set as input file. If no ms-es are given, ms out will also be used\n"
			   << "            as input. Can be specified multiple times for ms joining.\n" 
		     << " -a       : Use only the auto correlations to determine flags -- each antenna combination will\n"
         << "            be its union of the auto correlations of the first and the second antenna\n"
		     << " -ri      : threshold the real and imaginary components separately instead\n"
		     << "            of thresholding the absolute values (also assumes Rayleigh noise dis instead of Gaussian).\n"
		     << " -v       : be more verbose\n"
		     << " -i <c>   : number of iterations (only applicable to iterative methods)\n"
		     << " -j <tc>  : number of threads to be spawned, if task is parellizable\n"
		     << " -si      : save images (directory \"flag\" should exist)\n"
		     << " -ss      : save stats (directory \"flag\" should exist)\n"
		     << " -tm1     : use the sum-threshold method\n"
         << " -tm2     : use the var-threshold method\n"
				 << "example:\n"
				 << "\t" << argv[0] << " -m0 vega.ms 0-9 10-13 0\n";
		return -1;
	}

	string msFilename = argv[pindex];
	if(parameters.sets.size() == 0) parameters.sets.push_back(msFilename);
	IntegerDomain
		antenna1Domain(argv[pindex+1]),
		antenna2Domain(argv[pindex+2]);
	parameters.band = atoi(argv[pindex+3]);

	if(parameters.useAbsoluteValues)
		cout << "The absolute values will be thresholded." << endl;
	else
		cout << "The real and imaginary components will be thresholded separately." << endl;
	if(parameters.joinFlags)
		cout << "Flags will be joined with original flagging." << endl;
	else
		cout << "Flags will not be joined with original flagging. " << endl;
	if(parameters.useThreshold)
		cout << "Threshold: " << parameters.threshold << " * sigma" << endl;
	else
		cout << "False-positive probability: " << parameters.falsePositiveProbability << endl;
	cout << "Flagging combination of antenna's " << antenna1Domain.GetValue(0);
	for(unsigned i=1;i<antenna1Domain.ValueCount();++i)
		cout << ", " << antenna1Domain.GetValue(i);
	cout << " with antenna's " << antenna2Domain.GetValue(0);
	for(unsigned i=1;i<antenna2Domain.ValueCount();++i)
		cout << ", " << antenna2Domain.GetValue(i);
	cout << "." << endl;
	if(parameters.useOnlyAutoCorrelations) cout << "Only the auto correlations will be used for flagging!" << endl;

	switch(parameters.method)
	{
		case LocalGaussianWeightedAvg:
			cout << "Flagging with the local Gaussian weighted average method, " << parameters.fittingIterations << " iterations." << endl;
			break;
		case Svd:
			cout << "Flagging with the singular value decomposition method, removing " << parameters.svdRemoveCount << " singular values. " << endl;
			break;
		case Remove:
			cout << "Clearing flag tables!" << endl;
			break;
	}

	switch(parameters.method) {
		case SumThreshold:
		case Svd:
		parameters.thresholdConfig.SetMethod(ThresholdConfig::SumThreshold);
		break;
		case VarThreshold:
		parameters.thresholdConfig.SetMethod(ThresholdConfig::VarThreshold);
		break;
	}
	if(parameters.method == Svd)
		parameters.thresholdConfig.InitializeLengthsDefault(1);
	else
		parameters.thresholdConfig.InitializeLengthsDefault();
	parameters.thresholdConfig.SetVerbose(parameters.verbose);
	ThresholdConfig::Distribution dist;
	if(parameters.useAbsoluteValues)
		dist = ThresholdConfig::Rayleigh;
	else
		dist = ThresholdConfig::Gaussian;
	if(parameters.useThreshold)
		parameters.thresholdConfig.InitializeThresholdsFromFirstThreshold(parameters.threshold, dist);
	else
		parameters.thresholdConfig.InitializeThresholdsWithFalseRate(1000, parameters.falsePositiveProbability, dist);

	if(parameters.useAbsoluteValues)
		std::cout << "False alarm rate on Rayleigh distributed noise: ";
	else
		std::cout << "False alarm rate on Gaussian noise: ";
	std::cout << parameters.thresholdConfig.CalculateFalseAlarmRate(4000, dist) << std::endl;

	if(parameters.useOnlyAutoCorrelations) {
		// Analyse only the auto correlations
		IntegerDomain autoAntennas(antenna1Domain);
		autoAntennas.Join(antenna2Domain);

		std::deque<unsigned> antennaTasks;
		for(unsigned aIndex=0;aIndex<autoAntennas.ValueCount();++aIndex)
			antennaTasks.push_back(autoAntennas.GetValue(aIndex));

		boost::mutex sharedMemoryMutex, heavyIOMutex;
		boost::thread_group group;
		for(int i=0;i<parameters.threads;++i) {
			WorkThreadFunc threadFunc;
			threadFunc._heavyIOMutex = &heavyIOMutex;
			threadFunc._sharedMemoryMutex = &sharedMemoryMutex;
			threadFunc.antenna1Domain = &antenna1Domain;
			threadFunc.antenna2Domain = &antenna2Domain;
			threadFunc.msFilename = msFilename;
			threadFunc.parameters = parameters;
			threadFunc.workList = &antennaTasks;
			threadFunc.threadId = i;

			group.create_thread(threadFunc);
			sleep(45);
		}
		group.join_all();
	} else {
		// Analyse every correlation between all (specified) antenna's
		TimeFrequencyImager imager;
		imager.SetReadFlags(true);
		for(unsigned a1index=0;a1index<antenna1Domain.ValueCount();++a1index) {
			for(unsigned a2index=0;a2index<antenna2Domain.ValueCount();++a2index) {
				unsigned a1 = antenna1Domain.GetValue(a1index);
				unsigned a2 = antenna2Domain.GetValue(a2index);
				cout << "|\n| Now processing antenna's " << a1 << " and " << a2 << "\n|" << endl;
	
				cout << "Reading measurement set..." << endl;
				imager.Image(a1, a2, parameters.band);
	
				cout << "Processing XX..." << endl;
				stringstream subname;
				subname << "-a" << a1 << "a" << a2;
				Mask2DPtr maskXX = Perform(parameters, imager.RealXX(), imager.ImaginaryXX(), imager.FlagXX(), string("xx") + subname.str(), imager.BandInfo());
	
				cout << "Processing XY..." << endl;
				Mask2DPtr maskXY = Perform(parameters, imager.RealXY(), imager.ImaginaryXY(), imager.FlagXY(), string("xy") + subname.str(), imager.BandInfo());
	
				cout << "Processing YX..." << endl;
				Mask2DPtr maskYX = Perform(parameters, imager.RealYX(), imager.ImaginaryYX(), imager.FlagYX(), string("yx") + subname.str(), imager.BandInfo());
	
				cout << "Processing YY..." << endl;
				Mask2DPtr maskYY = Perform(parameters, imager.RealYY(), imager.ImaginaryYY(), imager.FlagYY(), string("yy") + subname.str(), imager.BandInfo());
				
				if(parameters.joinFlags) {
					cout << "Combining masks..." << endl;
					maskXX->Join(imager.FlagXX());
					maskXY->Join(imager.FlagXY());
					maskYX->Join(imager.FlagYX());
					maskYY->Join(imager.FlagYY());
				}

				double oldFlagRatio = (long double) (
						imager.FlagXX()->GetCount<true>() +
						imager.FlagXY()->GetCount<true>() +
						imager.FlagYX()->GetCount<true>() +
						imager.FlagYY()->GetCount<true>())
					/ (imager.FlagXX()->Width() * imager.FlagXX()->Height() * 4);
				double newFlagRatio = (long double) (
						maskXX->GetCount<true>() +
						maskXY->GetCount<true>() +
						maskYX->GetCount<true>() +
						maskYY->GetCount<true>())
					/ (maskXX->Width() * maskXX->Height() * 4);
	
				cout << "Writing flags (" << (round(newFlagRatio * 1000.0) / 10.0) << "% flagged, was previously " << (round(oldFlagRatio * 1000.0) / 10.0) << "%) ..." << endl;
				imager.WriteNewFlags(maskXX, maskXY, maskYX, maskYY, a1, a2, parameters.band);
			}
		}
	}

  return EXIT_SUCCESS;
}

Mask2DPtr Perform(Parameters &parameters, Image2DCPtr real, Image2DCPtr imaginary, Mask2DCPtr flag, const std::string &subname, const BandInfo &bandInfo)
{
	bool containsUnflagged = false;
	for(unsigned y=0;y<flag->Height();++y) {
		for(unsigned x=0;x<flag->Width();++x) {
			if(flag->Value(x, y)) {
				containsUnflagged = true;
				goto endOfLoop;
			}
		}
	}
endOfLoop:
	if(!containsUnflagged && !parameters.method == Remove) {
		std::cout << "Image contains only zeros! Skipping!" << std::endl;
		return Mask2D::CreateSetMaskPtr<false>(real->Width(), real->Height());
	} else {
		MethodIterator iterator;
		iterator.SetSavePNGs(parameters.saveImages);
		iterator.SetWriteStats(parameters.writeStats);
		iterator.SetOriginalFlag(flag);
		iterator.SetThresholdConfig(parameters.thresholdConfig);
		iterator.SetVerbose(parameters.verbose);
		iterator.SetBandInfo(&bandInfo);

		switch(parameters.method) {
		case LocalGaussianWeightedAvg: {
			LocalFitMethod fitMethod;
			fitMethod.SetToWeightedAverage(20, 40, 7.5, 15.0); // 20 40 15 30 (10, 20, 7.5, 15.0)
		
			if(parameters.useAbsoluteValues) {
				Image2DPtr absolute = Image2DPtr(FFTTools::CreateAbsoluteImage(*real, *imaginary));
				TimeFrequencyData data(TimeFrequencyData::AmplitudePart, TimeFrequencyData::SinglePolarisation, absolute);
				iterator.SetInput(data);
				iterator.IterateBackgroundFit(fitMethod, parameters.threads, std::string("flag/") + subname + "-imgabsolute", parameters.fittingIterations);
				Mask2DPtr mask = Mask2D::CreateCopy(iterator.GetMask());
				return mask;
			} else {
				TimeFrequencyData rData(TimeFrequencyData::RealPart, TimeFrequencyData::SinglePolarisation, real);
				iterator.SetInput(rData);
				iterator.IterateBackgroundFit(fitMethod, parameters.threads, std::string("flag/") + subname + "-imgreal", parameters.fittingIterations);
				Mask2DPtr mask = Mask2D::CreateCopy(iterator.GetMask());
		
				TimeFrequencyData iData(TimeFrequencyData::ImaginaryPart, TimeFrequencyData::SinglePolarisation, imaginary);
				iterator.SetInput(iData);
				iterator.IterateBackgroundFit(fitMethod, parameters.threads, std::string("flag/") + subname + "-imgimaginary", parameters.fittingIterations);
	
				mask->Join(iterator.GetMask());
				return mask;
			}
		}
	
		case Svd: {
			TimeFrequencyData data(TimeFrequencyData::SinglePolarisation, real, imaginary);

			SVDMitigater mitigater;

			iterator.SetInput(data);
			iterator.IterateBackgroundFit(mitigater, parameters.threads, std::string("flag/") + subname + "-imgcomplex", parameters.fittingIterations);
			Mask2DPtr mask = Mask2D::CreateCopy(iterator.GetMask());
			return mask;
			}
	
		case Remove: {
			return Mask2D::CreateSetMaskPtr<false>(real->Width(), real->Height());
			}
		}
	
	}
	return Mask2DPtr();
}
