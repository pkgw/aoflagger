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

#include "msio/image2d.h"

#include "rfi/localfitmethod.h"
#include "rfi/methoditerator.h"
#include "rfi/mitigationtester.h"
#include "rfi/thresholdconfig.h"

#include "util/integerdomain.h"
#include "util/plot.h"

#include "roctree.h"

#include "rfi/svdmitigater.h" // last!

using namespace std;

class FlagTest : ROCTreeClassifier {
	public:
		FlagTest() { SetDefaultParameters(); }
		void Run(int argc, char *argv[])
		{
			ParseParameters(argc, argv);
			RunAllTests();
			Clean();
		}
		virtual void Perform(long double &falseRatio, long double &trueRatio, long double parameter);
	private:
		void SetDefaultParameters();
		void ParseParameters(int argc, char *argv[]);
		void RunAllTests();
		void RunTest(long double &falseRatio, long double &trueRatio);
		void Clean() { }

		SurfaceFitMethod *method;
		int testSet;
		MethodIterator iterator;

		// options
		ThresholdConfig::Method thresholdMethod;
		long double falseRate;
		unsigned width, height;
		enum MitigateMethod { Threshold, SVD } mitigateMethod;
		enum BackFitMethod { None, GaussianWeightedAvg, Median } backFitMethod;
		unsigned iterations;
		unsigned svdCount;
		IntegerDomain *testSets;
		bool verbose;
		long double expFactor;
		bool roc;
		std::string rocFilename;
		long double rocLimit;
		int repeat;
		int thresCount;
		int thresLength;
		bool savePng;
		bool thresCurve;
		long double thresCurveLimit;
		int hWindowSize, vWindowSize;
};

void FlagTest::SetDefaultParameters()
{
	thresholdMethod = ThresholdConfig::SumThreshold;
	falseRate = 0.001;
	width = 1024;
	height = 256;
	mitigateMethod = Threshold;
	backFitMethod = GaussianWeightedAvg;
	iterations = 3;
	svdCount = 2;
	testSets = new IntegerDomain(0, 7);
	verbose = false;
	expFactor = 0.0L;
	roc = false;
	rocFilename = "";
	rocLimit = 1.0L;
	repeat = 1;
	thresCount = 0;
	thresLength = 1;
	savePng = true;
	thresCurve = false;
	thresCurveLimit = 3.0;
	hWindowSize = 4;
	vWindowSize = 8;
}

void FlagTest::ParseParameters(int argc, char *argv[])
{
	int pindex = 1;
	while(pindex < argc) {
		string parameter = argv[pindex]+1;
		if(parameter == "tm1") thresholdMethod = ThresholdConfig::SumThreshold;
		else if(parameter == "tm2") thresholdMethod = ThresholdConfig::VarThreshold;
		else if(parameter == "m1") mitigateMethod = Threshold;
		else if(parameter == "m2") mitigateMethod = SVD;
		else if(parameter == "bm0") backFitMethod = None;
		else if(parameter == "bm1") backFitMethod = GaussianWeightedAvg;
		else if(parameter == "bm2") backFitMethod = Median;
		else if(parameter == "ww") {
			++pindex;
			hWindowSize = atoi(argv[pindex]);
		}
		else if(parameter == "wh") {
			++pindex;
			vWindowSize = atoi(argv[pindex]);
		}
		else if(parameter == "v") verbose = true;
		else if(parameter == "nopng") savePng = false;
		else if(parameter == "threscount") {
			++pindex;
			thresCount = atoi(argv[pindex]);
		}
		else if(parameter == "threslength") {
			++pindex;
			thresLength = atoi(argv[pindex]);
		}
		else if(parameter == "fr") {
			++pindex;
			falseRate = atof(argv[pindex]);
		}
		else if(parameter == "ts") {
			++pindex;
			delete testSets;
			testSets = new IntegerDomain(argv[pindex]);
		}
		else if(parameter == "svdcount") {
			++pindex;
			svdCount = atoi(argv[pindex]);
		}
		else if(parameter == "w") {
			++pindex;
			width = atoi(argv[pindex]);
		}
		else if(parameter == "h") {
			++pindex;
			height = atoi(argv[pindex]);
		}
		else if(parameter == "i") {
			++pindex;
			iterations = atoi(argv[pindex]);
		}
		else if(parameter == "x") {
			++pindex;
			expFactor = atof(argv[pindex]);
		}
		else if(parameter == "roc") {
			++pindex;
			rocFilename = argv[pindex];
			roc = true;
		}
		else if(parameter == "r") {
			++pindex;
			repeat = atoi(argv[pindex]);
		}
		else if(parameter == "roclimit") {
			++pindex;
			rocLimit = atof(argv[pindex]);
		}
		else if(parameter == "threscurve") {
			++pindex;
			thresCurve = true;
			thresCurveLimit = atof(argv[pindex]);
		}
		else {
			cerr << "Unknown parameter: -" << parameter << endl;
			throw;
		}
		++pindex;
	}
}

void FlagTest::RunAllTests()
{
	iterator.SetSavePNGs(true);
	iterator.SetWriteStats(false);
	iterator.SetVerbose(verbose);
	ThresholdConfig config;
	config.SetVerbose(verbose);
	if(expFactor != 0.0L) {
		config.SetExpThresholdFactor(expFactor);
	}
	if(roc)
		falseRate = 0.0;

	if(mitigateMethod == Threshold) {
		LocalFitMethod *thresholdMitigater = new LocalFitMethod();
		method = thresholdMitigater;
		switch(backFitMethod) {
			case None:
				thresholdMitigater->SetToNone();
				break;
			case GaussianWeightedAvg:
				thresholdMitigater->SetToWeightedAverage(20, 40, 7.5, 15.0);
				break;
			case Median:
				thresholdMitigater->SetToMedianFilter(hWindowSize, vWindowSize);
				break;
		}
	
		config.SetMethod(thresholdMethod);
		config.SetFitBackground(method);
		if(!thresCurve) {
			config.InitializeLengthsDefault(thresCount);
			config.InitializeThresholdsFromFirstThreshold(3.9375, ThresholdConfig::Rayleigh);
			config.InitializeThresholdsWithFalseRate(500, falseRate, ThresholdConfig::Rayleigh);
		}
	} else { // SVD
		SVDMitigater *svdMitigater = new SVDMitigater();
		method = svdMitigater;
		svdMitigater->SetRemoveCount(svdCount);
		svdMitigater->SetVerbose(verbose);

		config.InitializeLengthsSingleSample();
		config.InitializeThresholdsWithFalseRate(500, falseRate, ThresholdConfig::Rayleigh);
		iterations = 1;
	}

	iterator.SetThresholdConfig(config);
	iterator.SetSavePNGs(savePng);

	for(unsigned setIndex=0;setIndex<testSets->ValueCount();++setIndex) {
		testSet = testSets->GetValue(setIndex);
		cout << "#== Data set: " << MitigationTester::GetTestSetDescription(testSet) << " ==" << endl;
		if(testSet==1 && mitigateMethod==SVD) {
			cout << "This data set will be skipped because of a bug in the SVD library" << endl;
		} else {
			if(roc) {
				ROCTree tree(config.GetThreshold(0), *this);
				for(long double a=0.0L;a<rocLimit + rocLimit/20.0L;a += rocLimit/10.0L) {
					long double falseRatio, trueRatio;
					tree.Find(a, rocLimit/20.0, falseRatio, trueRatio);
					cout
						<< "Correct: " << roundf(trueRatio * 1000.0L)/10.0L << "% of contaminated samples" << endl
						<< "False: " << roundf(falseRatio * 1000.0L)/10.0L << "% of all samples" << endl;
				}
				tree.Save(rocFilename);
			}
			else if(thresCurve) {
				long double thresholdStep = 0.01L;
				cout << "#threshold\tfalseRatio\ttrueRatio\tcombinations\tfirstCombinationLength" << endl;
				for(long double threshold = 0.0; threshold <= thresCurveLimit + thresholdStep*thresCurveLimit/2.0L ; threshold += thresholdStep*thresCurveLimit)
				{
					config.InitializeLengthsFrom(thresCount, thresLength);
					if(thresCount == 1)
						config.SetThreshold(0, threshold);
					else
						config.InitializeThresholdsFromFirstThreshold(threshold, ThresholdConfig::Gaussian);
					long double falseRatio, trueRatio;
					RunTest(falseRatio, trueRatio);
					cout << config.GetThreshold(0) << "\t" << falseRatio << "\t" << trueRatio << "\t" << config.GetOperations() << "\t" << config.GetLength(0) << endl;
				}
			}
			else {
				long double falseRatio, trueRatio;
				RunTest(falseRatio, trueRatio);
				cout
					<< "Correct: " << roundf(trueRatio * 1000.0L)/10.0L << "% of contaminated samples" << endl
					<< "False: " << roundf(falseRatio * 1000.0L)/10.0L << "% of all samples" << endl;
			}
		}
	}
	delete method;
	delete testSets;
}

void FlagTest::RunTest(long double &falseRatio, long double &trueRatio)
{
	bool iterationFinished;
	int repeated = 0;
	size_t totalCorrect=0, totalNotfound=0, totalError=0, totalTotal=0, totalTotalRFI=0;
	do {
		iterationFinished = true;
		Mask2DPtr rfi = Mask2D::CreateSetMaskPtr<false>(width, height);
		Image2DPtr data = MitigationTester::CreateTestSet(testSet, rfi, width, height);
	
		iterator.SetOriginalFlag(rfi);

		TimeFrequencyData *input;
		if(mitigateMethod == SVD) {
			input = new TimeFrequencyData(TimeFrequencyData::SinglePolarisation, data, data);
			iterator.SetInput(*input);
		} else {
			input = new TimeFrequencyData(TimeFrequencyData::AmplitudePart, TimeFrequencyData::SinglePolarisation, data);
			iterator.SetInput(*input);
		}
	
		iterator.IterateBackgroundFit(*method, 1, "flags/testset", iterations);
	
		size_t correct=0, notfound=0, error=0, total=rfi->Width()*rfi->Height();
		MitigationTester::CountResults(iterator.GetMask(), rfi, correct, notfound, error);
		size_t totalRfi = rfi->GetCount<true>();
		totalCorrect += correct;
		totalNotfound += notfound;
		totalError += error;
		totalTotal += total;
		totalTotalRFI += totalRfi;
		if(totalRfi==0) {
			totalRfi = 1; correct = 1; notfound = 0;
		}
	
		delete input;

		if(repeated+1 < repeat) {
			iterationFinished = false;
			repeated++;
		}
	} while(!iterationFinished);
	falseRatio = (long double) totalError / totalTotal;
	trueRatio = (long double) totalCorrect / totalTotalRFI;
}

void FlagTest::Perform(long double &falseRatio, long double &trueRatio, long double parameter)
{
	iterator.Config().InitializeThresholdsFromFirstThreshold(parameter, ThresholdConfig::Gaussian);
	RunTest(falseRatio, trueRatio);
}

int main(int argc, char *argv[])
{
	FlagTest flagTest;
	flagTest.Run(argc, argv);
}
