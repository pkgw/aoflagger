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
#ifndef AOFLAGGER_EIGENVALUETEST_H
#define AOFLAGGER_EIGENVALUETEST_H

#include "../../testingtools/asserter.h"
#include "../../testingtools/unittest.h"

#include "../../../strategy/algorithms/eigenvalue.h"

class EigenvalueTest : public UnitTest {
	public:
    EigenvalueTest() : UnitTest("Eigenvalue")
		{
			AddTest(TestRemove(), "Remove");
		}
		
	private:
		struct TestRemove : public Asserter
		{
			void operator()();
		};
		
	static std::string ToString(Image2DCPtr real, Image2DCPtr imag)
	{
		std::stringstream s;
		s << "{";
		for(unsigned y=0;y<real->Height();++y)
		{
			for(unsigned x=0;x<real->Width();++x)
			{
				if(x!=0 || y!=0) s << ",";
				s << real->Value(x, y);
			}
		}
		s << "} {";
		for(unsigned y=0;y<imag->Height();++y)
		{
			for(unsigned x=0;x<imag->Width();++x)
			{
				if(x!=0 || y!=0) s << ",";
				s << imag->Value(x, y);
			}
		}
		s << "}";
		return s.str();
	}
};

void EigenvalueTest::TestRemove::operator()()
{
	Image2DPtr
		real1 = Image2D::CreateZeroImagePtr(2, 2),
		imag1 = Image2D::CreateZeroImagePtr(2, 2);
	
	Eigenvalue::Remove(real1, imag1);

	const std::string real1Str("Real values of zero image");
	AssertAlmostEqual(real1->Value(0,0), 0.0, real1Str);
	AssertAlmostEqual(real1->Value(0,1), 0.0, real1Str);
	AssertAlmostEqual(real1->Value(1,0), 0.0, real1Str);
	AssertAlmostEqual(real1->Value(1,1), 0.0, real1Str);

	const std::string imag1Str("Imaginary values of zero image");
	AssertAlmostEqual(imag1->Value(0,0), 0.0, imag1Str);
	AssertAlmostEqual(imag1->Value(0,1), 0.0, imag1Str);
	AssertAlmostEqual(imag1->Value(1,0), 0.0, imag1Str);
	AssertAlmostEqual(imag1->Value(1,1), 0.0, imag1Str);

	real1->SetValue(0, 0, 13.0);
	real1->SetValue(0, 1, -4.0);
	real1->SetValue(1, 0, -4.0);
	real1->SetValue(1, 1, 7.0);
	Eigenvalue::Remove(real1, imag1);
	AssertEquals(ToString(real1, imag1), "{12,-6,-6,3} {0,0,0,0}");
}

#endif
