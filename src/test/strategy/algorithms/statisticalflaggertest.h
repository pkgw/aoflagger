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
#ifndef AOFLAGGER_STATISTICALFLAGGERTEST_H
#define AOFLAGGER_STATISTICALFLAGGERTEST_H

#include "../../testingtools/asserter.h"
#include "../../testingtools/unittest.h"

#include "../../../msio/mask2d.h"

#include "../../../strategy/algorithms/statisticalflagger.h"

class StatisticalFlaggerTest : public UnitTest {
	public:
		StatisticalFlaggerTest() : UnitTest("Statistical flagger")
		{
			AddTest(TestTimeDilation(), "Time dilation");
			AddTest(TestFrequencyDilation(), "Frequency dilation");
			AddTest(TestTimeDilationSpeed(), "Time dilation speed");
		}
		
	private:
		struct TestTimeDilation : public Asserter
		{
			void operator()();
		};
		struct TestFrequencyDilation : public Asserter
		{
			void operator()();
		};
		struct TestTimeDilationSpeed : public Asserter
		{
			void operator()();
		};
		
		static std::string maskToString(Mask2DCPtr mask)
		{
			std::stringstream s;
			for(unsigned y=0;y<mask->Height();++y)
			{
				for(unsigned x=0;x<mask->Width();++x)
				{
					s << (mask->Value(x, y) ? 'x' : ' ');
				}
			}
			return s.str();
		}
		
		static void setMask(Mask2DPtr mask, const std::string &str)
		{
			std::string::const_iterator i = str.begin();
			for(unsigned y=0;y<mask->Height();++y)
			{
				for(unsigned x=0;x<mask->Width();++x)
				{
					mask->SetValue(x, y, (*i) == 'x');
					++i;
				}
			}
		}
};

inline void StatisticalFlaggerTest::TestTimeDilation::operator()()
{
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(10, 1);
	setMask(mask, "     x    ");
	
	StatisticalFlagger::DensityTimeFlagger(mask, 0.0);
	AssertEquals(maskToString(mask), "     x    ", "Min=0.0, single center flagged, no enlarge");
	
	StatisticalFlagger::DensityTimeFlagger(mask, 0.5);
	AssertEquals(maskToString(mask), "     x    ", "Min=0.5, single center flagged, no enlarge");
	
	StatisticalFlagger::DensityTimeFlagger(mask, 0.6);
	AssertEquals(maskToString(mask), "    xx    ", "Min=0.6, from one to two samples");

	StatisticalFlagger::DensityTimeFlagger(mask, 0.0);
	AssertEquals(maskToString(mask), "    xx    ");
	
	StatisticalFlagger::DensityTimeFlagger(mask, 0.5);
	AssertEquals(maskToString(mask), "   xxxx   ");

	StatisticalFlagger::DensityTimeFlagger(mask, 0.6);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");

	StatisticalFlagger::DensityTimeFlagger(mask, 1.0);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");
	
	setMask(mask, "xx xx     ");

	StatisticalFlagger::DensityTimeFlagger(mask, 0.0);
	AssertEquals(maskToString(mask), "xx xx     ");

	StatisticalFlagger::DensityTimeFlagger(mask, 0.5);
	AssertTrue(mask->Value(2,0), "Fill hole");
	
	mask = Mask2D::CreateSetMaskPtr<false>(40, 1);
	//             0    5    0    5    0    5    0    5    
	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	StatisticalFlagger::DensityTimeFlagger(mask, 0.2);
	AssertEquals(maskToString(mask), "    xxxxxxxxxxxxx x xxxxxxxxxxxx        ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	StatisticalFlagger::DensityTimeFlagger(mask, 0.3);
	AssertEquals(maskToString(mask), "   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	StatisticalFlagger::DensityTimeFlagger(mask, 0.5);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  ");

	setMask(mask, "xxxxxxxxxxxxxxx       xxxxxxxxxxxxxxxxxx");
	StatisticalFlagger::DensityTimeFlagger(mask, 0.3);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

inline void StatisticalFlaggerTest::TestFrequencyDilation::operator()()
{
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(1, 10);
	setMask(mask, "     x    ");
	
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.0);
	AssertEquals(maskToString(mask), "     x    ", "Min=0.0, single center flagged, no enlarge");
	
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.5);
	AssertEquals(maskToString(mask), "     x    ", "Min=0.5, single center flagged, no enlarge");
	
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.6);
	AssertEquals(maskToString(mask), "    xx    ", "Min=0.6, from one to two samples");

	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.0);
	AssertEquals(maskToString(mask), "    xx    ");
	
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.5);
	AssertEquals(maskToString(mask), "   xxxx   ");

	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.6);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");

	StatisticalFlagger::DensityFrequencyFlagger(mask, 1.0);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");
	
	setMask(mask, "xx xx     ");

	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.0);
	AssertEquals(maskToString(mask), "xx xx     ");

	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.5);
	AssertTrue(mask->Value(0,2), "Fill hole");
	
	mask = Mask2D::CreateSetMaskPtr<false>(1, 40);
	//             0    5    0    5    0    5    0    5    
	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.2);
	AssertEquals(maskToString(mask), "    xxxxxxxxxxxxx x xxxxxxxxxxxx        ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.3);
	AssertEquals(maskToString(mask), "   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.5);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  ");

	setMask(mask, "xxxxxxxxxxxxxxx       xxxxxxxxxxxxxxxxxx");
	StatisticalFlagger::DensityFrequencyFlagger(mask, 0.3);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

inline void StatisticalFlaggerTest::TestTimeDilationSpeed::operator()()
{
	const unsigned flagsSize = 10000;
	const unsigned channels = 256;
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(flagsSize, channels);
	for(unsigned y=0;y<channels;++y)
	{
		for(unsigned i=0;i<flagsSize; ++i)
		{
			mask->SetValue(i, 0, (RNG::Uniform() >= 0.2));
		}
	}
	StatisticalFlagger::DensityTimeFlagger(mask, 0.1);
}

#endif
