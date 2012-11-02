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
#ifndef AOFLAGGER_FILTERRESULTSTEST_H
#define AOFLAGGER_FILTERRESULTSTEST_H

#include <complex>
#include <fstream>

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/msio/pngfile.h>

#include <AOFlagger/util/ffttools.h>

#include <AOFlagger/imaging/defaultmodels.h>

#include <AOFlagger/strategy/actions/directionalcleanaction.h>
#include <AOFlagger/strategy/actions/foreachcomplexcomponentaction.h>
#include <AOFlagger/strategy/actions/fouriertransformaction.h>
#include <AOFlagger/strategy/actions/frequencyconvolutionaction.h>
#include <AOFlagger/strategy/actions/fringestopaction.h>
#include <AOFlagger/strategy/actions/iterationaction.h>
#include <AOFlagger/strategy/actions/setimageaction.h>
#include <AOFlagger/strategy/actions/strategyaction.h>
#include <AOFlagger/strategy/actions/timeconvolutionaction.h>
#include <AOFlagger/strategy/actions/uvprojectaction.h>

class FilterResultsTest : public UnitTest {
	public:
		FilterResultsTest() : UnitTest("Filter results")
		{
			std::ofstream("filter-stats.txt", std::ios_base::out | std::ios_base::trunc);
			AddTest(TestNoSource(), "No source");
			AddTest(TestConstantSource(), "Constant source");
			AddTest(TestVariableSource(), "Variable source");
			AddTest(TestFaintSource(), "Faint source");
			AddTest(TestMissedSource(), "Misslocated source");
		}
		
	private:
		const static unsigned CHANNEL_COUNT;
		const static double BANDWIDTH;
		
		struct TestNoSource : public Asserter
		{
			void operator()();
		};
		struct TestConstantSource : public Asserter
		{
			void operator()();
		};
		struct TestVariableSource : public Asserter
		{
			void operator()();
		};
		struct TestFaintSource : public Asserter
		{
			void operator()();
		};
		struct TestMissedSource : public Asserter
		{
			void operator()();
		};
		
		static rfiStrategy::Strategy *createStrategy(bool withFringeFilter, bool withTimeConv, bool withFrequencyConv, bool withTimeClean)
		{
			rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
			
			if(withFringeFilter)
			{
				rfiStrategy::FringeStopAction *fsAction = new rfiStrategy::FringeStopAction();
				fsAction->SetNewPhaseCentreRA(DefaultModels::DistortionRA());
				fsAction->SetNewPhaseCentreDec(DefaultModels::DistortionDec());
				fsAction->SetFringesToConsider(3.0);
				fsAction->SetMinWindowSize(500);
				fsAction->SetMaxWindowSize(500);
				strategy->Add(fsAction);

				rfiStrategy::SetImageAction *setAction = new rfiStrategy::SetImageAction();
				setAction->SetNewImage(rfiStrategy::SetImageAction::SwapRevisedAndContaminated);
				strategy->Add(setAction);
			}
			
			if(withFrequencyConv || withTimeConv)
			{
				rfiStrategy::ForEachComplexComponentAction *foccAction = new rfiStrategy::ForEachComplexComponentAction();
				strategy->Add(foccAction);

				if(withFrequencyConv)
				{
					rfiStrategy::FrequencyConvolutionAction *fcAction = new rfiStrategy::FrequencyConvolutionAction();
					fcAction->SetConvolutionSize(5.0);
					fcAction->SetKernelKind(rfiStrategy::FrequencyConvolutionAction::SincKernel);
					foccAction->Add(fcAction);
				}
				
				if(withTimeConv)
				{
					rfiStrategy::TimeConvolutionAction *tcAction = new rfiStrategy::TimeConvolutionAction();
					tcAction->SetSincScale(6.0);
					tcAction->SetIsSincScaleInSamples(false);
					tcAction->SetOperation(rfiStrategy::TimeConvolutionAction::SingleSincOperation);
					foccAction->Add(tcAction);
					
					rfiStrategy::SetImageAction *setAction = new rfiStrategy::SetImageAction();
					setAction->SetNewImage(rfiStrategy::SetImageAction::SwapRevisedAndContaminated);
					foccAction->Add(setAction);
				}
			}
			
			if(withTimeClean)
			{
				rfiStrategy::SetImageAction *set1Action = new rfiStrategy::SetImageAction();
				set1Action->SetNewImage(rfiStrategy::SetImageAction::SwapRevisedAndContaminated);
				strategy->Add(set1Action);
				
				rfiStrategy::IterationBlock *iterAction = new rfiStrategy::IterationBlock();
				iterAction->SetIterationCount(50);
				strategy->Add(iterAction);

				rfiStrategy::SetImageAction *set2Action = new rfiStrategy::SetImageAction();
				set2Action->SetNewImage(rfiStrategy::SetImageAction::FromRevised);
				iterAction->Add(set2Action);
				
				rfiStrategy::ForEachComplexComponentAction *feccAction = new rfiStrategy::ForEachComplexComponentAction();
				feccAction->SetOnAmplitude(false);
				feccAction->SetOnReal(true);
				feccAction->SetOnImaginary(true);
				iterAction->Add(feccAction);
				
				rfiStrategy::UVProjectAction *projAction = new rfiStrategy::UVProjectAction();
				projAction->SetDirectionRad(103.54 * (M_PI / 180.0));
				projAction->SetReverse(false);
				feccAction->Add(projAction);

				rfiStrategy::FourierTransformAction *ftAction = new rfiStrategy::FourierTransformAction();
				iterAction->Add(ftAction);
				
				rfiStrategy::DirectionalCleanAction *dcAction = new rfiStrategy::DirectionalCleanAction();
				dcAction->SetLimitingDistance(1.0/2.0);
				dcAction->SetChannelConvolutionSize(1);
				dcAction->SetMakePlot(false);
				dcAction->SetRemoveRatio(0.8);
				iterAction->Add(dcAction);
				
				rfiStrategy::SetImageAction *set3Action = new rfiStrategy::SetImageAction();
				set3Action->SetNewImage(rfiStrategy::SetImageAction::FromOriginal);
				strategy->Add(set3Action);
				
				//rfiStrategy::SetImageAction *set4Action = new rfiStrategy::SetImageAction();
				//set4Action->SetNewImage(rfiStrategy::SetImageAction::SwapRevisedAndContaminated);
				//strategy->Add(set4Action);
			}
			
			return strategy;
		}
		
		static double SquaredValue(const UVImager &imager, unsigned x, unsigned y)
		{
			double a = imager.FTReal().Value(x, y), b = imager.FTImaginary().Value(x, y);
			return a*a + b*b;
		}
		
		static void AddStatistics(const UVImager &imager, const std::string filename, double centerPower, double sidelobePower, double onAxisPower)
		{
			std::ofstream statFile("filter-stats.txt", std::ios_base::out | std::ios_base::app);
			double sourceSum =
				SquaredValue(imager, 625, 172) +
				SquaredValue(imager, 625, 173) +
				SquaredValue(imager, 625, 174) +
				SquaredValue(imager, 625, 175) +
				SquaredValue(imager, 626, 173) +
				SquaredValue(imager, 626, 174);
			statFile << filename << '\t' << sqrt(sourceSum/6.0);
			
			double sourceSidelobeRMS = 0.0;
			unsigned countSource = 0;
			for(unsigned y=0; y<346; ++y)
			{
				for(unsigned x=510; x<740; ++x)
				{
					sourceSidelobeRMS += SquaredValue(imager, x, y);
					++countSource;
				}
			}
			statFile << '\t' << sqrt(sourceSidelobeRMS/countSource);
			
			double onAxisRMS = 0.0;
			unsigned countOnAxis = 0;
			for(unsigned y=650; y<1000; ++y)
			{
				for(unsigned x=700; x<850; ++x)
				{
					onAxisRMS += SquaredValue(imager, x, y);
					++countOnAxis;
				}
			}
			statFile << '\t' << sqrt(onAxisRMS/countOnAxis);
			
			statFile << '\t' << 10.0*log10((centerPower - 1172.84) / (sqrt(sourceSum/6.0) - 1172.84));
			statFile << '\t' << 10.0*log10((sidelobePower - 2018.02) / (sqrt(sourceSidelobeRMS/countSource) - 2018.02));
			statFile << '\t' << 100.0*(1.0-(sqrt(onAxisRMS/countOnAxis)) / (onAxisPower)) << '%';
			statFile << '\n';
		}
		
		static void SaveImaged(const TimeFrequencyData &original, rfiStrategy::ArtifactSet &artifacts, const std::string &filename, bool difference, double centerPower, double sidelobePower, double onAxisPower)
		{
			UVImager imager(1024*1.5, 1024*1.5);
			
			TimeFrequencyData *data;
			if(difference)
			{
				data =
					TimeFrequencyData::CreateTFDataFromDiff(original, artifacts.ContaminatedData());
			} else {
				data =
					new TimeFrequencyData(artifacts.ContaminatedData());
			}
			
			imager.SetUVScaling(0.0012);
			imager.Image(*data, artifacts.MetaData());
			imager.PerformFFT();
			
			if(!difference)
				AddStatistics(imager, filename, centerPower, sidelobePower, onAxisPower);

			InvPositiveMap colorMap;
			//const Image2DPtr uvImage = Image2D::CreateCopyPtr(imager.RealUVImage());
			//PngFile::Save(*uvImage, std::string("UV-") + filename, colorMap);
			
			const Image2DPtr image = Image2D::CreateCopyPtr(imager.FTReal());
			
			image->SetTrim(400, 0, 1000, 1200);
			FFTTools::SignedSqrt(*image);
			image->SetToAbs();
			
			PngFile::Save(*image, filename, colorMap, 0.01);
			
			delete data;
		}
		
		static void Run(rfiStrategy::Strategy *strategy, std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data, const std::string &appliedName, const std::string &differenceName, double centerPower, double sidelobePower, double onAxisPower)
		{
			rfiStrategy::ArtifactSet artifacts(0);
			DummyProgressListener listener;
			
			artifacts.SetOriginalData(data.first);
			artifacts.SetContaminatedData(data.first);
			TimeFrequencyData zero(data.first);
			zero.SetImagesToZero();
			artifacts.SetRevisedData(zero);
			artifacts.SetMetaData(data.second);
			
			strategy->Perform(artifacts, listener);
			
			SaveImaged(data.first, artifacts, appliedName, false, centerPower, sidelobePower, onAxisPower);
			SaveImaged(data.first, artifacts, differenceName, true, centerPower, sidelobePower, onAxisPower);
		}
		
		static void RunAllMethods(std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data, const std::string &setPrefix, const std::string &setName, double centerPower, double sidelobePower, double onAxisPower)
		{
			rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
			Run(strategy, data, setPrefix + "0-" + setName + "-Original.png", "Empty.png", centerPower, sidelobePower, onAxisPower);
			delete strategy;

			strategy = createStrategy(true, false, false, false);
			Run(strategy, data, setPrefix + "1-" + setName + "-FringeFilter-Applied.png", setPrefix + "1-" + setName + "-FringeFilter-Difference.png", centerPower, sidelobePower, onAxisPower);
			delete strategy;
			
			strategy = createStrategy(false, true, false, false);
			Run(strategy, data, setPrefix + "2-" + setName + "-TimeFilter-Applied.png", setPrefix + "2-" + setName + "-TimeFilter-Difference.png", centerPower, sidelobePower, onAxisPower);
			delete strategy;
			
			strategy = createStrategy(false, false, true, false);
			Run(strategy, data, setPrefix + "3-" + setName + "-FreqFilter-Applied.png", setPrefix + "3-" + setName + "-FreqFilter-Difference.png", centerPower, sidelobePower, onAxisPower);
			delete strategy;

			strategy = createStrategy(false, false, false, true);
			Run(strategy, data, setPrefix + "4-" + setName + "-TimeClean-Applied.png", setPrefix + "4-" + setName + "-TimeClean-Difference.png", centerPower, sidelobePower, onAxisPower);
			delete strategy;

			strategy = createStrategy(false, true, true, false);
			Run(strategy, data, setPrefix + "5-" + setName + "-TimeFreq-Applied.png", setPrefix + "5-" + setName + "-TimeFreq-Difference.png", centerPower, sidelobePower, onAxisPower);
			delete strategy;
		}
};

const unsigned FilterResultsTest::CHANNEL_COUNT = 64;
const double FilterResultsTest::BANDWIDTH = 2500000.0;


inline void FilterResultsTest::TestNoSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::NoDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "N", "NoSource", 1172.84, 2018.02, 34340.5);

	data = DefaultModels::LoadSet(DefaultModels::EmptySet, DefaultModels::NoDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "O", "NoSource", 0.0, 0.0, 0.0);
}

inline void FilterResultsTest::TestConstantSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::ConstantDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "A", "ConstantSource", 541855.0, 36144.3, 47121.4);

	data = DefaultModels::LoadSet(DefaultModels::EmptySet, DefaultModels::ConstantDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "P", "ConstantSource", 6627.86, 959.102, 1172.99);
}

inline void FilterResultsTest::TestVariableSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::VariableDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "B", "VariableSource", 1.36159e+06, 99793.8, 56885.0);

	data = DefaultModels::LoadSet(DefaultModels::EmptySet, DefaultModels::VariableDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "Q", "VariableSource", 629649.0, 50508.9, 2456.41);
}

inline void FilterResultsTest::TestFaintSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::FaintDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "C", "FaintSource", 79725.9, 5972.41, 36504.8);

	data = DefaultModels::LoadSet(DefaultModels::EmptySet, DefaultModels::FaintDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "R", "FaintSource", 6031.9, 584.827, 335.651);
}

inline void FilterResultsTest::TestMissedSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::MislocatedDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "D", "MissedSource", 232796.0, 100212.6, 57039.4);

	data = DefaultModels::LoadSet(DefaultModels::EmptySet, DefaultModels::MislocatedDistortion, 0.0, CHANNEL_COUNT, BANDWIDTH);
 	RunAllMethods(data, "S", "MissedSource", 95403.1, 50750.5, 2526.13);
}

#endif
