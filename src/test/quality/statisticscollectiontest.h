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
			AddTest(TestStatisticsCollectingSpeed(), "Speed of collecting");
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
		struct TestStatisticsCollectingSpeed : public Asserter
		{
			void operator()();
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

void StatisticsCollectionTest::TestStatisticsCollectingSpeed::operator()()
{
	size_t nFreq = 500, nTimes = 2000;
	
	StatisticsCollection collectionA(1), collectionB(1);
	std::vector<double> frequencies(nFreq), times(nTimes);
	for(size_t i=0; i!=nFreq; ++i)
		frequencies[i] = i+1;
	for(size_t i=0; i!=nTimes; ++i)
		times[i] = i+1;
	collectionA.InitializeBand(0, &frequencies[0], nFreq);
	collectionB.InitializeBand(0, &frequencies[0], nFreq);

	Image2DPtr image = Image2D::CreateZeroImagePtr(nTimes, nFreq);
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(nTimes, nFreq);
	
	Stopwatch watchA(true);
	for(size_t a=0; a!=10; ++a)
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
	std::cout << "10 x nTimes x Add() took " << watchA.ToString() << '\n';

	Stopwatch watchB(true);
	for(size_t a=0; a!=10; ++a)
		collectionB.AddImage(0, a, &times[0], 0, 0, image, image, mask, mask);
	std::cout << "10 x AddImage() took " << watchB.ToString() << '\n';
}

#endif

