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
#ifndef AOFLAGGER_QUALITYTABLESFORMATTERTEST_H
#define AOFLAGGER_QUALITYTABLESFORMATTERTEST_H

#include "../testingtools/asserter.h"
#include "../testingtools/unittest.h"

#include "../../quality/qualitytablesformatter.h"
#include "../../quality/statisticalvalue.h"

#include <tables/Tables/Table.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>

class QualityTablesFormatterTest : public UnitTest {
	public:
    QualityTablesFormatterTest() : UnitTest("Quality data")
		{
			createTable();
			AddTest(TestConstructor(), "Class constructor");
			AddTest(TestTableExists(), "Query table existance");
			AddTest(TestTableInitialization(), "Initialize tables");
			AddTest(TestKindOperations(), "Statistic kind operations");
			AddTest(TestKindNames(), "Statistic kind names");
			AddTest(TestStoreStatistics(), "Storing statistics");
		}
    virtual ~QualityTablesFormatterTest()
		{
			removeTable();
		}
	private:
		void createTable()
		{
			casa::TableDesc tableDesc("MAIN_TABLE", "1.0", casa::TableDesc::Scratch);
			tableDesc.addColumn(casa::ScalarColumnDesc<int>("TEST"));
			casa::SetupNewTable mainTableSetup("QualityTest.MS", tableDesc, casa::Table::New);
			casa::Table mainOutputTable(mainTableSetup);
		}
		void removeTable()
		{
			casa::Table::deleteTable("QualityTest.MS");
		}
		struct TestConstructor : public Asserter
		{
			void operator()();
		};
		struct TestTableExists : public Asserter
		{
			void operator()();
		};
		struct TestTableInitialization : public Asserter
		{
			void operator()();
		};
		struct TestKindOperations : public Asserter
		{
			void operator()();
		};
		struct TestKindNames : public Asserter
		{
			void operator()();
		};
		struct TestStoreStatistics : public Asserter
		{
			void operator()();
		};
};

void QualityTablesFormatterTest::TestConstructor::operator()()
{
	QualityTablesFormatter *qd = new QualityTablesFormatter("QualityTest.MS");
	delete qd;
}

void QualityTablesFormatterTest::TestTableExists::operator()()
{
	QualityTablesFormatter qd("QualityTest.MS");
	// undefined answer, but should not crash.
	qd.TableExists(QualityTablesFormatter::KindNameTable);
	qd.TableExists(QualityTablesFormatter::TimeStatisticTable);
	qd.TableExists(QualityTablesFormatter::FrequencyStatisticTable);
	qd.TableExists(QualityTablesFormatter::BaselineStatisticTable);
	qd.TableExists(QualityTablesFormatter::BaselineTimeStatisticTable);
	
	qd.RemoveAllQualityTables();
	AssertFalse(qd.TableExists(QualityTablesFormatter::KindNameTable));
	AssertFalse(qd.TableExists(QualityTablesFormatter::TimeStatisticTable));
	AssertFalse(qd.TableExists(QualityTablesFormatter::FrequencyStatisticTable));
	AssertFalse(qd.TableExists(QualityTablesFormatter::BaselineStatisticTable));
	AssertFalse(qd.TableExists(QualityTablesFormatter::BaselineTimeStatisticTable));
}

void QualityTablesFormatterTest::TestTableInitialization::operator()()
{
	QualityTablesFormatter qd("QualityTest.MS");
	
	qd.RemoveAllQualityTables();
	
	enum QualityTablesFormatter::QualityTable tables[5] =
	{
		QualityTablesFormatter::KindNameTable,
		QualityTablesFormatter::TimeStatisticTable,
		QualityTablesFormatter::FrequencyStatisticTable,
		QualityTablesFormatter::BaselineStatisticTable,
		QualityTablesFormatter::BaselineTimeStatisticTable
	};
	for(unsigned i=0;i<5;++i)
	{
		qd.InitializeEmptyTable(tables[i], 8);
		AssertTrue(qd.TableExists(tables[i]), "Table exists after initialization");
	}
	
	for(unsigned i=0;i<5;++i)
	{
		qd.RemoveTable(tables[i]);
		for(unsigned j=0;j<=i;++j)
			AssertFalse(qd.TableExists(tables[j]), "Table removed and no longer exists");
		for(unsigned j=i+1;j<5;++j)
			AssertTrue(qd.TableExists(tables[j]), "Table initialized and not yet removed, thus exists");
	}
}

void QualityTablesFormatterTest::TestKindOperations::operator()()
{
	QualityTablesFormatter qd("QualityTest.MS");
	
	qd.RemoveAllQualityTables();
	qd.InitializeEmptyTable(QualityTablesFormatter::KindNameTable, 4);
	AssertTrue(qd.TableExists(QualityTablesFormatter::KindNameTable));
	
	unsigned kindIndex;
	AssertFalse(qd.QueryKindIndex(QualityTablesFormatter::MeanStatistic, kindIndex), "Empty table contains no index");
	
	unsigned originalKindIndex = qd.StoreKindName(QualityTablesFormatter::MeanStatistic);
	AssertTrue(qd.QueryKindIndex(QualityTablesFormatter::MeanStatistic, kindIndex), "Stored index is queried");
	AssertEquals(kindIndex, originalKindIndex, "Index returned by QueryKindIndex(Statistic, int)");
	AssertEquals(qd.QueryKindIndex(QualityTablesFormatter::MeanStatistic), originalKindIndex, "Index returned by QueryKindIndex(Statistic)");
	
	unsigned secondKindIndex = qd.StoreKindName(QualityTablesFormatter::VarianceStatistic);
	AssertNotEqual(originalKindIndex, secondKindIndex, "Store two kinds");
	AssertEquals(qd.QueryKindIndex(QualityTablesFormatter::MeanStatistic), originalKindIndex);
	AssertEquals(qd.QueryKindIndex(QualityTablesFormatter::VarianceStatistic), secondKindIndex);
	
	qd.InitializeEmptyTable(QualityTablesFormatter::KindNameTable, 4);
	AssertFalse(qd.QueryKindIndex(QualityTablesFormatter::MeanStatistic, kindIndex), "Empty table contains no index after re-init");
}

void QualityTablesFormatterTest::TestStoreStatistics::operator()()
{
	QualityTablesFormatter qd("QualityTest.MS");
	
	qd.RemoveAllQualityTables();
	AssertFalse(qd.IsStatisticAvailable(QualityTablesFormatter::TimeDimension, QualityTablesFormatter::MeanStatistic), "Statistic not available when table not exists");
	
	qd.InitializeEmptyTable(QualityTablesFormatter::KindNameTable, 4);
	AssertFalse(qd.IsStatisticAvailable(QualityTablesFormatter::TimeDimension, QualityTablesFormatter::MeanStatistic), "Statistic not available when only kind-name table exists");
	
	qd.InitializeEmptyTable(QualityTablesFormatter::TimeStatisticTable, 4);
	AssertFalse(qd.IsStatisticAvailable(QualityTablesFormatter::TimeDimension, QualityTablesFormatter::MeanStatistic), "Statistic not available when empty tables exist");
	
	unsigned meanStatIndex = qd.StoreKindName(QualityTablesFormatter::MeanStatistic);
	AssertFalse(qd.IsStatisticAvailable(QualityTablesFormatter::TimeDimension, QualityTablesFormatter::MeanStatistic), "Statistic not available when no entries in stat table");
	AssertEquals(qd.QueryStatisticEntryCount(QualityTablesFormatter::TimeDimension, meanStatIndex), 0u, "QueryStatisticEntryCount with zero entries");

	StatisticalValue value(4);
	value.SetKindIndex(meanStatIndex);
	value.SetValue(0, std::complex<float>(0.0, 1.0));
	value.SetValue(1, std::complex<float>(2.0, -2.0));
	value.SetValue(2, std::complex<float>(-3.0, 3.0));
	value.SetValue(3, std::complex<float>(-4.0, -4.0));
	qd.StoreTimeValue(60.0, 107000000.0, value);
	AssertTrue(qd.IsStatisticAvailable(QualityTablesFormatter::TimeDimension, QualityTablesFormatter::MeanStatistic), "Statistic available");
	AssertEquals(qd.QueryStatisticEntryCount(QualityTablesFormatter::TimeDimension, meanStatIndex), 1u, "QueryStatisticEntryCount with one entries");
	
	std::vector<std::pair<QualityTablesFormatter::TimePosition, StatisticalValue> > entries;
	qd.QueryTimeStatistic(meanStatIndex, entries);
	AssertEquals(entries.size(), (size_t) 1, "entries.size()");
	std::pair<QualityTablesFormatter::TimePosition, StatisticalValue> entry = entries[0];
	AssertEquals(entry.first.frequency, 107000000.0f, "frequency");
	AssertEquals(entry.first.time, 60.0f, "time");
	AssertEquals(entry.second.PolarizationCount(), 4u, "PolarizationCount()");
	AssertEquals(entry.second.KindIndex(), meanStatIndex, "KindIndex()");
	AssertEquals(entry.second.Value(0), std::complex<float>(0.0, 1.0), "Value(0)");
	AssertEquals(entry.second.Value(1), std::complex<float>(2.0, -2.0), "Value(1)");
	AssertEquals(entry.second.Value(2), std::complex<float>(-3.0, 3.0), "Value(2)");
	AssertEquals(entry.second.Value(3), std::complex<float>(-4.0, -4.0), "Value(3)");
	
	qd.RemoveTable(QualityTablesFormatter::KindNameTable);
	qd.RemoveTable(QualityTablesFormatter::TimeStatisticTable);
}

void QualityTablesFormatterTest::TestKindNames::operator()()
{
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::MeanStatistic), "Mean");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::VarianceStatistic), "Variance");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::SumStatistic), "Sum");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::SumP2Statistic), "SumP2");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::DMeanStatistic), "DMean");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::DVarianceStatistic), "DVariance");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::DSumStatistic), "DSum");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::DSumP2Statistic), "DSumP2");
	AssertEquals(QualityTablesFormatter::KindToName(QualityTablesFormatter::FTSumP2Statistic), "FTSumP2");
}

#endif
