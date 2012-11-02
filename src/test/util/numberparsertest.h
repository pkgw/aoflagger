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
#ifndef AOFLAGGER_NUMBERPARSERTEST_H
#define AOFLAGGER_NUMBERPARSERTEST_H

#include "../testingtools/asserter.h"
#include "../testingtools/unittest.h"

#include "../../util/numberparser.h"

class NumberParserTest : public UnitTest {
	public:
		NumberParserTest() : UnitTest("Number parser")
		{
			AddTest(TestToDouble(), "Parsing double");
		}
		
	private:
		struct TestToDouble : public Asserter
		{
			void operator()();
		};
};

inline void NumberParserTest::TestToDouble::operator()()
{
	AssertEquals<double>(NumberParser::ToDouble("1"), 1.0);
	AssertEquals<double>(NumberParser::ToDouble("1."), 1.0);
	AssertEquals<double>(NumberParser::ToDouble("1.000000"), 1.0);
	AssertEquals<double>(NumberParser::ToDouble("-1"), -1.0);
	AssertEquals<double>(NumberParser::ToDouble("-1.00000"), -1.0);
	
	AssertEquals<double>(NumberParser::ToDouble("3.14159265"), 3.14159265);
	AssertEquals<double>(NumberParser::ToDouble("0.00002"), 0.00002);
	AssertEquals<double>(NumberParser::ToDouble("234567"), 234567.0);
	AssertEquals<double>(NumberParser::ToDouble("234.567"), 234.567);
	
	AssertEquals<double>(NumberParser::ToDouble("0"), 0.0);
	AssertEquals<double>(NumberParser::ToDouble("0.0"), 0.0);
	AssertEquals<double>(NumberParser::ToDouble("-0.0"), 0.0);
	
	AssertEquals<double>(NumberParser::ToDouble("0.0e5"), 0.0);
	AssertEquals<double>(NumberParser::ToDouble("0.0e100"), 0.0);
	AssertEquals<double>(NumberParser::ToDouble("0.0e-100"), 0.0);
	AssertEquals<double>(NumberParser::ToDouble("0.0E5"), 0.0);
	AssertEquals<double>(NumberParser::ToDouble("0.0E100"), 0.0);
	AssertEquals<double>(NumberParser::ToDouble("0.0E-100"), 0.0);
	
	AssertAlmostEqual(NumberParser::ToDouble("1.0e5"), 1.0e5);
	AssertAlmostEqual(NumberParser::ToDouble("1.0e-5"), 1.0e-5);
	AssertAlmostEqual(NumberParser::ToDouble("0.3e0"), 0.3);
	AssertAlmostEqual(NumberParser::ToDouble("642.135e8"), 642.135e8);
}

#endif
