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
#ifndef AOFLAGGER_CONVOLUTIONSTEST_H
#define AOFLAGGER_CONVOLUTIONSTEST_H

#include "../../testingtools/asserter.h"
#include "../../testingtools/unittest.h"

#include "../../../strategy/algorithms/convolutions.h"

#include <cmath>

class ConvolutionsTest : public UnitTest {
	public:
		ConvolutionsTest() : UnitTest("Convolutions")
		{
			AddTest(TestOneDimensionalConvolution(), "One dimensional convolution");
			AddTest(TestOneDimensionalSincConvolution(), "One dimensional sinc convolution");
		}
		
	private:
		struct TestOneDimensionalConvolution : public Asserter
		{
			void operator()();
		};
		
		struct TestOneDimensionalSincConvolution : public Asserter
		{
			void operator()();
		};
		
		static std::string ToString(const num_t *values, unsigned count)
		{
			std::stringstream s;
			s << '{' << values[0];
			for(unsigned i=1;i<count;++i)
			{
				s << ", " << values[i];
			}
			s << '}';
			return s.str();
		}
		
		static void AssertValues(Asserter *asserter, const num_t *values, const num_t *expected, unsigned count)
		{
			asserter->AssertEquals(ToString(values, count), ToString(expected, count));
			for(unsigned i=0;i<count;++i)
			{
				std::stringstream s;
				s << "Value " << i;
				asserter->AssertAlmostEqual(values[i], expected[i], s.str());
			}
		}
};

inline void ConvolutionsTest::TestOneDimensionalConvolution::operator()()
{
	// Remember that OneDimensionalConvolutionBorderInterp assumes that sumover kernel == 1,
	// otherwise we have to multiply the output with sumover kernel.
	num_t data1[3] = { 0.0, 1.0, 2.0 };
	num_t kernel1[1] = { 1.0 };
	Convolutions::OneDimensionalConvolutionBorderInterp(data1, 3, kernel1, 1);
	AssertAlmostEqual(data1[0], 0.0);
	AssertAlmostEqual(data1[1], 1.0);
	AssertAlmostEqual(data1[2], 2.0);
	
	num_t kernel2[1] = { 0.0 };
	Convolutions::OneDimensionalConvolutionBorderInterp(data1, 3, kernel2, 1);

	num_t data3[4] = { 0.0, 1.0, 2.0, 3.0 };
	num_t kernel3[1] = { 2.0 };
	Convolutions::OneDimensionalConvolutionBorderInterp(data3, 4, kernel3, 1);
	AssertAlmostEqual(data3[0], 0.0);
	AssertAlmostEqual(data3[1], 1.0);
	AssertAlmostEqual(data3[2], 2.0);
	AssertAlmostEqual(data3[3], 3.0);
	
	num_t kernel4[2] = {0.0, 1.0};
	Convolutions::OneDimensionalConvolutionBorderInterp(data3, 4, kernel4, 2);
	AssertAlmostEqual(data3[0], 0.0);
	AssertAlmostEqual(data3[1], 1.0);
	AssertAlmostEqual(data3[2], 2.0);
	AssertAlmostEqual(data3[3], 3.0);
	
	num_t kernel5[2] = {1.0, 1.0};
	Convolutions::OneDimensionalConvolutionBorderInterp(data3, 4, kernel5, 2);
	AssertAlmostEqual(data3[0], 0.0);
	AssertAlmostEqual(data3[1], 0.5);
	AssertAlmostEqual(data3[2], 1.5);
	AssertAlmostEqual(data3[3], 2.5);
	
	num_t data6[4] = { 0.0, 1.0, 2.0, 3.0 };
	num_t kernel6[3] = {1.0, 1.0, 1.0};
	Convolutions::OneDimensionalConvolutionBorderInterp(data6, 4, kernel6, 3);
	num_t expected6[4] = {0.5, 1.0, 2.0, 2.5};
	AssertValues(this, data6, expected6, 4);

	num_t data7[6] = { 0.0, 0.0, 1.0, 2.0, 3.0, 0.0 };
	num_t kernel7[3] = {1.0, 1.0, 1.0};
	Convolutions::OneDimensionalConvolutionBorderInterp(data7, 6, kernel7, 3);
	num_t expected7[6] = {0.0, 1.0/3.0, 1.0, 2.0, 5.0/3.0, 1.5};
	AssertValues(this, data7, expected7, 6);
}

inline void ConvolutionsTest::TestOneDimensionalSincConvolution::operator()()
{
	num_t data1[1] = { 1.0 };
	Convolutions::OneDimensionalSincConvolution(data1, 1, 1.0);
	const num_t expected1[1] = { 1.0 };
	AssertValues(this, data1, expected1, 1);

	Convolutions::OneDimensionalSincConvolution(data1, 0, 1.0);

	num_t data2[2] = { 1.0, 1.0 };
	Convolutions::OneDimensionalSincConvolution(data2, 2, 0.25);
	//const num_t expected2[2] = { 1.0, 1.0 };
	//AssertValues(this, data2, expected2, 2);
	AssertEquals(data2[0], data2[1], "Symmetry test with 2 elements");

	num_t data3[3] = { 1.0, 1.0, 1.0 };
	Convolutions::OneDimensionalSincConvolution(data3, 3, 0.25);
	AssertAlmostEqual(data3[0], data3[2], "Symmetry test with 3 elements");

	num_t data4[4] = { 1.0, 1.0, 1.0, 1.0 };
	Convolutions::OneDimensionalSincConvolution(data4, 4, 0.25);
	AssertAlmostEqual(data4[0], data4[3], "Symmetry test with 4 elements, outer");
	AssertAlmostEqual(data4[1], data4[2], "Symmetry test with 4 elements, inner");

	const num_t sizes5[6] = { 0.01, 0.1, 1.0, 3.14, 10.0/3.0, 100.0 };
	for(unsigned i=0;i<6;++i)
	{
		num_t data5[5] = { 0.0, 0.0, 1.0, 0.0, 0.0 };
		Convolutions::OneDimensionalSincConvolution(data5, 5, sizes5[i]);
		AssertAlmostEqual(data5[0], data5[4], "Symmetry test with 5 elements, outer");
		AssertAlmostEqual(data5[1], data5[3], "Symmetry test with 5 elements, inner");
	}

	num_t data6[100];
	data6[0]=1;
	for(unsigned i=1;i<100;++i)
		data6[i] = 0;
	// Convolution with sinc frequency 0.25 will produce a low-pass filter
	// that filters any frequency > 0.25 Hz. The sinc will therefore have
	// maxima on index 1, 5, 9, ..., minima on index 3, 7, 11, .. and zero's
	// on 2, 4, 6, 8, ... .
	Convolutions::OneDimensionalSincConvolution(data6, 100, 0.25);
	
	// Check whether maxima decrease
	for(unsigned i=1; i<96; i+=4)
	{
		std::stringstream s;
		s << "Decreasing maxima of sinc, element " << i;
		AssertLessThan(data6[i+4], data6[i], s.str());
	}

	// Check whether minima increase. The border value (i=95) is not tested, because of
	// the normalization it is actually larger, which is ok.
	for(unsigned i=3; i<94; i+=4)
	{
		std::stringstream s;
		s << "Increasing minima of sin, element " << i;
		AssertGreaterThan(data6[i+4], data6[i], s.str());
	}
	
	// Check zero points
	for(unsigned i=2; i<100; i+=2)
	{
		AssertAlmostEqual(data6[i], 0.0, "Zero points of sinc");
	}

	// Test whether a low-pass filter attenuates a 10.000 sample
	// high-frequency chirp signal with various levels.
	num_t data7[10000];
	for(unsigned i=0;i<10000;i+=2)
	{
		data7[i] = -1;
		data7[i+1] = 1;
	}
	Convolutions::OneDimensionalSincConvolution(data7, 10000, 0.25);
	for(unsigned i=10;i<9990;++i)
	{
		AssertLessThan(std::abs(data7[i]), 0.1, "10 db attenuation in 99.9% center of filtered data");
	}
	for(unsigned i=100;i<9900;++i)
	{
		AssertLessThan(std::abs(data7[i]), 0.01, "20 db attenuation in 99% center of filtered data");
	}
	for(unsigned i=1000;i<9000;++i)
	{
		AssertLessThan(std::abs(data7[i]), 0.001, "30 db attenuation in 90% center of filtered data");
	}

	// Test whether a low-pass filter does not attenuate a 10.000 sample
	// low-frequency chirp signal with various levels.
	num_t data8[10000];
	for(unsigned i=0;i<10000;++i)
	{
		data8[i] = sin((num_t) i/(10.0 * 2 * M_PIn));
	}
	Convolutions::OneDimensionalSincConvolution(data8, 10000, 0.25);
	for(unsigned i=10;i<9950;++i)
	{
		num_t val = sin((num_t) i/(10.0 * 2 * M_PIn));
		AssertLessThan(std::abs(data8[i] - val), 0.5, "95% consistency in 99.5% center of filtered data");
	}
	for(unsigned i=100;i<9900;++i)
	{
		num_t val = sin((num_t) i/(10.0 * 2 * M_PIn));
		AssertLessThan(std::abs(data8[i] - val), 0.1, "99% consistency in 99% center of filtered data");
	}
	for(unsigned i=1000;i<9000;++i)
	{
		num_t val = sin((num_t) i/(10.0 * 2 * M_PIn));
		AssertLessThan(std::abs(data8[i] - val), 0.01, "99.9% consistency in 90% center of filtered data");
	}
}

#endif
