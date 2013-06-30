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
#ifndef AOFLAGGER_STATISTICSCOLLECTIONTEST_H
#define AOFLAGGER_STATISTICSCOLLECTIONTEST_H

#include <cmath>
#include <iomanip>

#include "../testingtools/asserter.h"
#include "../testingtools/unittest.h"

#include "../../quality/statisticscollection.h"
#include "../../quality/qualitytablesformatter.h"

class StatisticsCollectionTest : public UnitTest {
	public:
    StatisticsCollectionTest() : UnitTest("Statistics collection")
		{
			AddTest(TestConstructor(), "Class constructor");
			AddTest(TestStatisticsCollecting(), "Collecting statistics");
			AddTest(TestImageCollecting(), "Collecting from image");
			AddTest(TestComparison<false>(), "Add() and AddImage() do the same thing");
			//AddTest(TestComparison<true>(), "Speed of collecting");
		}
	private:
		static void AssertZero(const DefaultStatistics &statistics, Asserter &asserter, const char *description)
		{
			for(size_t i=0;i<statistics.PolarizationCount();++i)
			{
				asserter.AssertEquals(statistics.count[i], 0ul, description);
				asserter.AssertEquals(statistics.sum[i].real(), 0.0, description);
				asserter.AssertEquals(statistics.sum[i].imag(), 0.0, description);
				asserter.AssertEquals(statistics.sumP2[i].real(), 0.0, description);
				asserter.AssertEquals(statistics.sumP2[i].imag(), 0.0, description);
				asserter.AssertEquals(statistics.dCount[i], 0ul, description);
				asserter.AssertEquals(statistics.dSum[i].real(), 0.0, description);
				asserter.AssertEquals(statistics.dSum[i].imag(), 0.0, description);
				asserter.AssertEquals(statistics.dSumP2[i].real(), 0.0, description);
				asserter.AssertEquals(statistics.dSumP2[i].imag(), 0.0, description);
				asserter.AssertEquals(statistics.rfiCount[i], 0ul, description);
			}
		}
		
		static void AssertBasicExample(const DefaultStatistics &statistics, Asserter &asserter, const std::string &description)
		{
			asserter.AssertEquals(statistics.count[0], 3ul, description + " (count)");
			asserter.AssertEquals(statistics.sum->real(), 6.0, description + " (real sum)");
			asserter.AssertEquals(statistics.sum->imag(), 18.0, description + " (imag sum)");
			asserter.AssertEquals(statistics.sumP2->real(), 14.0, description + " (real sum^2)");
			asserter.AssertEquals(statistics.sumP2->imag(), 116.0, description + " (imag sum^2)");
			asserter.AssertEquals(statistics.dCount[0], 2ul, description + " (dCount)");
			asserter.AssertAlmostEqual(statistics.dSum->real(), 2.0 * M_SQRT1_2, description + " (real dSum)");
			asserter.AssertAlmostEqual(statistics.dSum->imag(), 4.0 * M_SQRT1_2, description + " (imag dSum)");
			// The correct values are calculated e.g. as in:
			// (2.0 * M_SQRT1_2 - 1.0 * M_SQRT1_2) ^ 2 + (3.0 * M_SQRT1_2 - 2.0 * M_SQRT1_2) ^ 2 (= 1/2 + 1/2 )
			asserter.AssertAlmostEqual(statistics.dSumP2->real(), 1.0, description + " (real dSum^2)");
			// (6.0 * M_SQRT1_2 - 4.0 * M_SQRT1_2) ^ 2 + (8.0 * M_SQRT1_2 - 6.0 * M_SQRT1_2) ^ 2 (= 2 + 2 )
			asserter.AssertAlmostEqual(statistics.dSumP2->imag(), 4.0, description + " (imag dSum^2)");
			asserter.AssertEquals(statistics.rfiCount[0], 0ul, description + " (rfiCount)");
		}
		
		struct TestConstructor : public Asserter
		{
			void operator()();
		};
		struct TestStatisticsCollecting : public Asserter
		{
			void operator()();
		};
		struct TestImageCollecting : public Asserter
		{
			void operator()();
		};
		template<bool SpeedTest>
		struct TestComparison : public Asserter
		{
			void operator()();
			void testCollectingImage(Image2DCPtr image, Mask2DCPtr mask, size_t nTimes, size_t nFreq);
		};
};

void StatisticsCollectionTest::TestConstructor::operator()()
{
	StatisticsCollection *collection = new StatisticsCollection();
	collection->SetPolarizationCount(1);
	AssertEquals(collection->PolarizationCount(), 1u, "Polarization count after init");
	DefaultStatistics statistics(1);
	collection->GetGlobalCrossBaselineStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalCrossBaselineStatistics() is zero after construct A");
	delete collection;
	
	collection = new StatisticsCollection(1);
	AssertEquals(collection->PolarizationCount(), 1u, "Polarization count after construct");
	collection->GetGlobalCrossBaselineStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalCrossBaselineStatistics() is zero after construct B");
	
	StatisticsCollection *copy = new StatisticsCollection(*collection);
	AssertEquals(collection->PolarizationCount(), 1u, "Polarization count after copy construct");
	copy->GetGlobalCrossBaselineStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalCrossBaselineStatistics() is zero after copy");
	delete copy;
	delete collection;
}

void StatisticsCollectionTest::TestStatisticsCollecting::operator()()
{
	StatisticsCollection collection(1);
	double frequencies[3] = {100, 101, 102};
	collection.InitializeBand(0, frequencies, 3);
	float
		reals[3] = { 1.0, 2.0, 3.0 },
		imags[3] = { 4.0, 6.0, 8.0 };
	bool isRFI[3] = { false, false, false };
	bool isPreFlagged[3] = { false, false, false };
	collection.Add(0, 0, 0.0, 0, 0, reals, imags, isRFI, isPreFlagged, 3, 1, 1, 1);
	
	DefaultStatistics statistics(1);
	collection.GetGlobalCrossBaselineStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalCrossBaselineStatistics() is zero");
	
	collection.GetGlobalFrequencyStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalFrequencyStatistics() is zero");
	
	collection.GetGlobalTimeStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalTimeStatistics() is zero");
	
	collection.GetGlobalAutoBaselineStatistics(statistics);
	AssertBasicExample(statistics, *this, "GetGlobalAutoBaselineStatistics()");
	
	collection.Add(0, 1, 0.0, 0, 0, reals, imags, isRFI, isPreFlagged, 3, 1, 1, 1);
	collection.GetGlobalCrossBaselineStatistics(statistics);
	AssertBasicExample(statistics, *this, "GetGlobalCrossBaselineStatistics()");
	
	collection.GetGlobalTimeStatistics(statistics);
	AssertBasicExample(statistics, *this, "GetGlobalTimeStatistics()");
	
	collection.GetGlobalFrequencyStatistics(statistics);
	AssertEquals(statistics.count[0], 3ul, "count");
	AssertEquals(statistics.sum->real(), 6.0, "real sum");
	AssertEquals(statistics.sum->imag(), 18.0, "imag sum");
	AssertEquals(statistics.sumP2->real(), 14.0, "real sum^2");
	AssertEquals(statistics.sumP2->imag(), 116.0, "imag sum^2");
	AssertEquals(statistics.dCount[0], 4ul, "dCount");
	AssertAlmostEqual(statistics.dSum->real(), 4.0 * M_SQRT1_2, "real dSum");
	AssertAlmostEqual(statistics.dSum->imag(), 8.0 * M_SQRT1_2, "imag dSum");
	AssertAlmostEqual(statistics.dSumP2->real(), 2.0, "real dSum^2");
	AssertAlmostEqual(statistics.dSumP2->imag(), 8.0, "imag dSum^2");
	AssertEquals(statistics.rfiCount[0], 0ul, "rfi count");
	
	bool flagged[3] = { true, true, true };
	collection.Add(0, 1, 0.0, 0, 0, reals, imags, flagged, isPreFlagged, 3, 1, 1, 1);
	collection.GetGlobalTimeStatistics(statistics);
	AssertEquals(statistics.count[0], 3ul, "count");
	AssertEquals(statistics.rfiCount[0], 3ul, "rfi count");
	AssertEquals(statistics.sum->real(), 6.0, "real sum");

	collection.Add(0, 1, 0.0, 0, 0, reals, imags, flagged, flagged, 3, 1, 1, 1);
	collection.GetGlobalTimeStatistics(statistics);
	AssertEquals(statistics.count[0], 3ul, "count");
	AssertEquals(statistics.rfiCount[0], 3ul, "rfi count");
	AssertEquals(statistics.sum->real(), 6.0, "real sum");
}

void StatisticsCollectionTest::TestImageCollecting::operator()()
{
	StatisticsCollection collection(1);
	double frequencies[3] = {100, 101, 102};
	collection.InitializeBand(0, frequencies, 3);
	Image2DPtr
		realImg = Image2D::CreateUnsetImagePtr(1, 3),
		imagImg = Image2D::CreateUnsetImagePtr(1, 3);
	realImg->SetValue(0, 0, 1.0); imagImg->SetValue(0, 0, 4.0);
	realImg->SetValue(0, 1, 2.0); imagImg->SetValue(0, 1, 6.0);
	realImg->SetValue(0, 2, 3.0); imagImg->SetValue(0, 2, 8.0);
	Mask2DPtr
		rfiMask = Mask2D::CreateSetMaskPtr<false>(1, 3),
		preFlaggedMask = Mask2D::CreateSetMaskPtr<false>(1, 3);
	const double times[1] = { 0.0 };
	
	collection.AddImage(0, 0, times, 0, 0, realImg, imagImg, rfiMask, preFlaggedMask);
	
	DefaultStatistics statistics(1);
	collection.GetGlobalCrossBaselineStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalCrossBaselineStatistics() is zero");
	
	collection.GetGlobalFrequencyStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalFrequencyStatistics() is zero");
	
	collection.GetGlobalTimeStatistics(statistics);
	AssertZero(statistics, *this, "GetGlobalTimeStatistics() is zero");
	
	collection.GetGlobalAutoBaselineStatistics(statistics);
	AssertBasicExample(statistics, *this, "GetGlobalAutoBaselineStatistics()");
	
	collection.AddImage(0, 1, times, 0, 0, realImg, imagImg, rfiMask, preFlaggedMask);
	collection.GetGlobalCrossBaselineStatistics(statistics);
	AssertBasicExample(statistics, *this, "GetGlobalCrossBaselineStatistics()");
	
	collection.GetGlobalTimeStatistics(statistics);
	AssertBasicExample(statistics, *this, "GetGlobalTimeStatistics()");
	
	collection.GetGlobalFrequencyStatistics(statistics);
	AssertEquals(statistics.count[0], 3ul, "count");
	AssertEquals(statistics.sum->real(), 6.0, "real sum");
	AssertEquals(statistics.sum->imag(), 18.0, "imag sum");
	AssertEquals(statistics.sumP2->real(), 14.0, "real sum^2");
	AssertEquals(statistics.sumP2->imag(), 116.0, "imag sum^2");
	AssertEquals(statistics.dCount[0], 4ul, "dCount");
	AssertAlmostEqual(statistics.dSum->real(), 4.0 * M_SQRT1_2, "real dSum");
	AssertAlmostEqual(statistics.dSum->imag(), 8.0 * M_SQRT1_2, "imag dSum");
	AssertAlmostEqual(statistics.dSumP2->real(), 2.0, "real dSum^2");
	AssertAlmostEqual(statistics.dSumP2->imag(), 8.0, "imag dSum^2");
	AssertEquals(statistics.rfiCount[0], 0ul, "rfi count");
	
	rfiMask->SetAll<true>();
	collection.AddImage(0, 1, times, 0, 0, realImg, imagImg, rfiMask, preFlaggedMask);
	collection.GetGlobalTimeStatistics(statistics);
	AssertEquals(statistics.count[0], 3ul, "count");
	AssertEquals(statistics.rfiCount[0], 3ul, "rfi count");
	AssertEquals(statistics.sum->real(), 6.0, "real sum");

	collection.AddImage(0, 1, times, 0, 0, realImg, imagImg, rfiMask, rfiMask);
	collection.GetGlobalTimeStatistics(statistics);
	AssertEquals(statistics.count[0], 3ul, "count");
	AssertEquals(statistics.rfiCount[0], 3ul, "rfi count");
	AssertEquals(statistics.sum->real(), 6.0, "real sum");
}

template<bool SpeedTest>
void StatisticsCollectionTest::TestComparison<SpeedTest>::operator()()
{
	size_t nFreq = 500, nTimes = 2000;
	
	Image2DPtr image = Image2D::CreateZeroImagePtr(nTimes, nFreq);
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(nTimes, nFreq);
	
	testCollectingImage(image, mask, nTimes, nFreq);
	
	image->SetAll(1.0);
	testCollectingImage(image, mask, nTimes, nFreq);
	
	image->SetAll(1.0);
	testCollectingImage(image, mask, nTimes, nFreq);
	
	for(size_t y=0; y!=nFreq; ++y)
	{
		for(size_t x=0; x!=nTimes; ++x)
			mask->SetValue(x, y, (x + y)%2 == 0);
	}
	testCollectingImage(image, mask, nTimes, nFreq);
	
	mask->SetAll<false>();
	for(size_t i=0; i<nFreq*nTimes; i+=17)
	{
		size_t x = i/nFreq, y = i%nFreq;
		image->SetValue(x, y, i%3);
		if((i%5)==0)
			mask->SetValue(x, y, true);
	}
	
	testCollectingImage(image, mask, nTimes, nFreq);
}

template<bool SpeedTest>
void StatisticsCollectionTest::TestComparison<SpeedTest>::testCollectingImage(Image2DCPtr image, Mask2DCPtr mask, size_t nTimes, size_t nFreq)
{
	size_t iterations = SpeedTest ? 10 : 1, startAntenna = SpeedTest ? 0 : 1;
	StatisticsCollection collectionA(1), collectionB(1);
	std::vector<double> frequencies(nFreq), times(nTimes);
	for(size_t i=0; i!=nFreq; ++i)
		frequencies[i] = i+1;
	for(size_t i=0; i!=nTimes; ++i)
		times[i] = i+1;

	collectionA.InitializeBand(0, &frequencies[0], nFreq);
	collectionB.InitializeBand(0, &frequencies[0], nFreq);
	
	Stopwatch watchA(true);
	for(size_t a=startAntenna; a!=iterations+startAntenna; ++a)
	{
		for(size_t t=0; t<image->Width(); ++t) 
		{
			collectionA.Add(0, a, times[t], 0, 0,
					image->Data()+t, image->Data()+t,
					mask->Data()+t, mask->Data()+t,
					nFreq, image->Stride(),
					mask->Stride(), mask->Stride());
		}
	}
	if(SpeedTest)
		std::cout << iterations << " x nTimes x Add() took " << watchA.ToString() << '\n';

	Stopwatch watchB(true);
	for(size_t a=startAntenna; a!=iterations+startAntenna; ++a)
		collectionB.AddImage(0, a, &times[0], 0, 0, image, image, mask, mask);
	if(SpeedTest)
		std::cout << iterations << " x AddImage() took " << watchB.ToString() << '\n';
	
	DefaultStatistics statA(1), statB(1);
	collectionA.GetGlobalCrossBaselineStatistics(statA);
	collectionB.GetGlobalCrossBaselineStatistics(statB);
	if(statA != statB)
	{
		AssertEquals(statA.count[0], statB.count[0], "Statistic count made by Add() and AddImage()");
		AssertAlmostEqual((double) statA.sum[0].real(), (double) statB.sum[0].real(), "Statistic sum made by Add() and AddImage()");
		AssertAlmostEqual((double) statA.sumP2[0].real() , (double) statB.sumP2[0].real(), "Statistic sumP2 made by Add() and AddImage()");
		AssertEquals(statA.dCount[0], statB.dCount[0], "Statistic dCount made by Add() and AddImage()");
		AssertAlmostEqual((double) statA.dSum[0].real(), (double) statB.dSum[0].real(), "Statistic dSum made by Add() and AddImage()");
		AssertAlmostEqual((double) statA.dSumP2[0].real(), (double) statB.dSumP2[0].real(), "Statistic dSumP2 made by Add() and AddImage()");
	}
}

#endif

