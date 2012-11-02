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
#ifndef AOFLAGGER_STATISTICSDERIVATORTEST_H
#define AOFLAGGER_STATISTICSDERIVATORTEST_H

#include <cmath>

#include "../testingtools/asserter.h"
#include "../testingtools/unittest.h"

#include "../../quality/statisticsderivator.h"
#include "../../quality/defaultstatistics.h"

class StatisticsDerivatorTest : public UnitTest {
	public:
    StatisticsDerivatorTest() : UnitTest("Statistics derivator")
		{
			AddTest(TestStatistics(), "Deriving statistics");
		}
	private:
		struct TestStatistics : public Asserter
		{
			void operator()();
		};
};

void StatisticsDerivatorTest::TestStatistics::operator()()
{
	// Our sequence: 0.0 0.0 0.0
	std::complex<long double> val = StatisticsDerivator::Variance(3, 0.0, 0.0);
	AssertEquals(val, std::complex<long double>(0.0, 0.0));
	
	// Our sequence: 1.0 2.0 3.0
	val = StatisticsDerivator::Variance(3, 6.0, 14.0);
	AssertAlmostEqual(val.real(), 1.0); // (1 + 0 + 1) / (n-1) = 1.0
	
	// Our sequence: 1.0+4.0i 2.0+6.0i 3.0+8.0i
	DefaultStatistics statistics(1);
	statistics.count[0] = 3;
	statistics.sum[0] = std::complex<long double>(6.0, 18.0);
	statistics.sumP2[0] = std::complex<long double>(14.0, 116.0);
	val = StatisticsDerivator::GetComplexStatistic(QualityTablesFormatter::CountStatistic, statistics, 0);
	AssertEquals(val.real(), 3.0);
	val = StatisticsDerivator::GetComplexStatistic(QualityTablesFormatter::SumStatistic, statistics, 0);
	AssertEquals(val.real(), 6.0);
	AssertEquals(val.imag(), 18.0);
	val = StatisticsDerivator::GetComplexStatistic(QualityTablesFormatter::SumP2Statistic, statistics, 0);
	AssertEquals(val.real(), 14.0);
	AssertEquals(val.imag(), 116.0);
	val = StatisticsDerivator::GetComplexStatistic(QualityTablesFormatter::MeanStatistic, statistics, 0);
	AssertEquals(val.real(), 6.0 / 3.0);
	AssertEquals(val.imag(), 18.0 / 3.0);
	val = StatisticsDerivator::GetComplexStatistic(QualityTablesFormatter::VarianceStatistic, statistics, 0);
	AssertEquals(val.real(), 1.0); // (1 + 0 + 1) / (n-1) = 1.0
	AssertEquals(val.imag(), 4.0); // (4 + 0 + 4) / (n-1) = 4.0
	val = StatisticsDerivator::GetComplexStatistic(QualityTablesFormatter::StandardDeviationStatistic, statistics, 0);
	AssertEquals(val.real(), sqrt(2.0/3.0)); // sqrt((1 + 0 + 1) / n) = sqrt(2/3)
	AssertEquals(val.imag(), sqrt(8.0/3.0)); // sqrt((4 + 0 + 4) / n) = sqrt(8/3)
	
	statistics.dCount[0] = statistics.count[0];
	statistics.dSum[0] = statistics.sum[0];
	statistics.dSumP2[0] = statistics.sumP2[0];
	val = StatisticsDerivator::GetComplexStatistic(QualityTablesFormatter::SignalToNoiseStatistic, statistics, 0);
	AssertAlmostEqual(val.real(), 2.0 / sqrt(2.0/3.0), "Real SNR");
	AssertAlmostEqual(val.imag(), 6.0 / sqrt(8.0/3.0), "Imag SNR");
}

#endif

