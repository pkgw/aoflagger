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
#ifndef AOFLAGGER_NOISESTATISTICSTEST_H
#define AOFLAGGER_NOISESTATISTICSTEST_H

#include "../../testingtools/asserter.h"
#include "../../testingtools/unittest.h"

#include "../../../msio/mask2d.h"

#include "../../../strategy/algorithms/noisestatistics.h"

class NoiseStatisticsTest : public UnitTest {
	public:
		NoiseStatisticsTest() : UnitTest("Noise statistics")
		{
			AddTest(TestInitialization(), "Initialization");
			AddTest(TestCalculations(), "Calculations");
			AddTest(TestAddValues(), "Adding values");
		}
		
	private:
		struct TestInitialization : public Asserter
		{
			void operator()();
		};
		
		struct TestCalculations : public Asserter
		{
			void operator()();
		};
		
		struct TestAddValues : public Asserter
		{
			void operator()();
		};
		
		static void AssertValues(
			const NoiseStatistics &statistics,
			const Asserter *asserter,
			long unsigned count,
			NoiseStatistics::stat_t sum,
			NoiseStatistics::stat_t sum2,
			NoiseStatistics::stat_t sum3,
			NoiseStatistics::stat_t sum4)
		{
			asserter->AssertEquals(statistics.Count(), count, "Count()");
			asserter->AssertAlmostEqual(statistics.Sum(), sum, "Sum()");
			asserter->AssertAlmostEqual(statistics.Sum2(), sum2, "Sum2()");
			asserter->AssertAlmostEqual(statistics.Sum3(), sum3, "Sum3()");
			asserter->AssertAlmostEqual(statistics.Sum4(), sum4, "Sum4()");
		}

		static void AssertValues(
			const NoiseStatistics &statistics,
			const Asserter *asserter,
			long unsigned count,
			NoiseStatistics::stat_t sum,
			NoiseStatistics::stat_t sum2,
			NoiseStatistics::stat_t sum3,
			NoiseStatistics::stat_t sum4,
			NoiseStatistics::stat_t mean,
			NoiseStatistics::stat_t moment2,
			NoiseStatistics::stat_t moment4,
			NoiseStatistics::stat_t stdDevEst,
			NoiseStatistics::stat_t varianceEst,
			NoiseStatistics::stat_t varianceOfVarianceEst)
		{
			AssertValues(statistics, asserter, count, sum, sum2, sum3, sum4);
			asserter->AssertAlmostEqual(statistics.Mean(), mean, "Mean()");
			asserter->AssertAlmostEqual(statistics.SecondMoment(), moment2, "SecondMoment()");
			asserter->AssertAlmostEqual(statistics.FourthMoment(), moment4, "FourthMoment()");
			asserter->AssertAlmostEqual(statistics.StdDevEstimator(), stdDevEst, "StdDevEstimator()");
			asserter->AssertAlmostEqual(statistics.VarianceEstimator(), varianceEst, "VarianceEstimator()");
			asserter->AssertAlmostEqual(statistics.VarianceOfVarianceEstimator(), varianceOfVarianceEst, "VarianceOfVarianceEstimator()");
		}
		
	static void AssertRunnable(const NoiseStatistics &statistics)
	{
		statistics.Count();
		statistics.Sum();
		statistics.Sum2();
		statistics.Sum3();
		statistics.Sum4();
		statistics.Mean();
		statistics.SecondMoment();
		statistics.FourthMoment();
		statistics.StdDevEstimator();
		statistics.VarianceEstimator();
		statistics.VarianceOfVarianceEstimator();
	}
};

inline void NoiseStatisticsTest::TestInitialization::operator()()
{
	// Test without initialization
	NoiseStatistics statistics;
	AssertValues(statistics, this, 0, 0.0, 0.0, 0.0, 0.0);
	// Some values are undefined, but should not throw an exception:
	AssertRunnable(statistics);
	
	// Test assignment + initialization with an array
	NoiseStatistics::Array array;
	array.push_back(1.0);
	statistics = NoiseStatistics(array);
	AssertValues(statistics, this, 1, 1.0, 1.0, 1.0, 1.0);
	AssertRunnable(statistics);
	
	// Test copy constructor
	NoiseStatistics copy(statistics);
	AssertValues(statistics, this, 1, 1.0, 1.0, 1.0, 1.0);
	AssertRunnable(statistics);
}

inline void NoiseStatisticsTest::TestCalculations::operator()()
{
	NoiseStatistics::Array array;
	array.push_back(1.0);
	array.push_back(2.0);
	array.push_back(3.0);
	NoiseStatistics statistics(array);
	AssertValues(statistics, this, 3, 6.0, 14.0, 36.0, 98.0);
	AssertAlmostEqual(statistics.Mean(), 2.0, "Mean()");
	AssertAlmostEqual(statistics.SecondMoment(), 2.0/3.0, "SecondMoment()");
	AssertAlmostEqual(statistics.FourthMoment(), 2.0/3.0, "FourthMoment()");
	AssertAlmostEqual(statistics.StdDevEstimator(), 1.0, "StdDevEstimator()");
	AssertAlmostEqual(statistics.VarianceEstimator(), 1.0, "VarianceEstimator()");
	AssertRunnable(statistics);
	
	array.clear();
	array.push_back(5.0);
	array.push_back(5.0);
	array.push_back(5.0);
	array.push_back(5.0);
	array.push_back(5.0);
	statistics = NoiseStatistics(array);
	AssertValues(statistics, this, 5, 25.0, 125.0, 625.0, 3125.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	array.clear();
	array.push_back(1.0);
	array.push_back(1.0);
	array.push_back(1.0);
	array.push_back(2.0);
	array.push_back(3.0);
	statistics = NoiseStatistics(array);
	AssertValues(statistics, this, 5, 8.0, 16.0, 38.0, 100.0, 1.6, 0.64, 0.8512, sqrt(0.8), 0.8, 0.12928);

	array.clear();
	array.push_back(3.0);
	array.push_back(1.0);
	array.push_back(3.0);
	array.push_back(1.0);
	array.push_back(3.0);
	array.push_back(1.0);
	statistics = NoiseStatistics(array);
	AssertValues(statistics, this, 6, 12.0, 30.0, 84.0, 246.0, 2.0, 1.0, 1.0, sqrt(1.2), 1.2, 2.0/30.0);
}

void NoiseStatisticsTest::TestAddValues::operator()()
{
	NoiseStatistics statistics;

	NoiseStatistics::Array array;
	array.push_back(1.0);
	array.push_back(2.0);
	array.push_back(3.0);
	statistics.Add(array);
	AssertValues(statistics, this, 3, 6.0, 14.0, 36.0, 98.0);
	AssertAlmostEqual(statistics.Mean(), 2.0, "Mean()");
	AssertAlmostEqual(statistics.SecondMoment(), 2.0/3.0, "SecondMoment()");
	AssertAlmostEqual(statistics.FourthMoment(), 2.0/3.0, "FourthMoment()");
	AssertAlmostEqual(statistics.VarianceEstimator(), 1.0, "VarianceEstimator()");
	AssertRunnable(statistics);
	
	array.clear();
	array.push_back(1.0);
	array.push_back(1.0);
	statistics.Add(array);
	AssertValues(statistics, this, 5, 8.0, 16.0, 38.0, 100.0, 1.6, 0.64, 0.8512, sqrt(0.8), 0.8, 0.12928);

	array.clear();
	array.push_back(5.0);
	NoiseStatistics numberFive = NoiseStatistics(array);
	
	statistics = NoiseStatistics();
	statistics.Add(numberFive);
	statistics.Add(numberFive);
	statistics.Add(numberFive);
	statistics.Add(numberFive);
	statistics.Add(numberFive);
	AssertValues(statistics, this, 5, 25.0, 125.0, 625.0, 3125.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	array.clear();
	array.push_back(3.0);
	array.push_back(3.0);
	array.push_back(3.0);
	NoiseStatistics partA = NoiseStatistics(array);
	
	array.clear();
	array.push_back(1.0);
	array.push_back(1.0);
	array.push_back(1.0);
	NoiseStatistics partB = NoiseStatistics(array);
	statistics = NoiseStatistics();
	statistics.Add(partA);
	statistics.Add(partB);
	AssertValues(statistics, this, 6, 12.0, 30.0, 84.0, 246.0, 2.0, 1.0, 1.0, sqrt(1.2), 1.2, 2.0/30.0);
}

#endif
