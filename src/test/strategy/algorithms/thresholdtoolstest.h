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
#ifndef AOFLAGGER_THRESHOLDTOOLSTEST_H
#define AOFLAGGER_THRESHOLDTOOLSTEST_H

#include "../../../msio/mask2d.h"

#include "../../../strategy/algorithms/thresholdtools.h"

#include "../../testingtools/asserter.h"
#include "../../testingtools/unittest.h"

class ThresholdToolsTest : public UnitTest {
	public:
		ThresholdToolsTest() : UnitTest("Threshold tools")
		{
			AddTest(WinsorizedMaskedMeanVar(), "Winsorized, masked mean and variance");
			AddTest(WinsorizedMaskedMode(), "Winsorized, masked mode");
		}
		
	private:
		struct WinsorizedMaskedMeanVar : public Asserter
		{
			void operator()();
		};
		struct WinsorizedMaskedMode : public Asserter
		{
			void operator()();
		};
};

void ThresholdToolsTest::WinsorizedMaskedMeanVar::operator()()
{
	num_t mean, variance;

	Mask2DCPtr fullMask = Mask2D::CreateSetMaskPtr<true>(100, 1);
	Image2DPtr image1 = Image2D::CreateZeroImagePtr(100, 1);
	// we just don't want it to crash
	ThresholdTools::WinsorizedMeanAndStdDev(image1, fullMask, mean, variance);
	
	Mask2DCPtr emptyMask = Mask2D::CreateSetMaskPtr<false>(100, 1);
	
	ThresholdTools::WinsorizedMeanAndStdDev(image1, emptyMask, mean, variance);
	Asserter::AssertAlmostEqual(mean, 0.0, "Mean of empty image");
	Asserter::AssertAlmostEqual(variance, 0.0, "Variance of empty image");
	
	image1->SetValue(0, 0, 1.0);
	ThresholdTools::WinsorizedMeanAndStdDev(image1, emptyMask, mean, variance);
	Asserter::AssertAlmostEqual(mean, 0.0, "Mean of almost empty image");
	Asserter::AssertAlmostEqual(variance, 0.0, "Variance of almost empty image");

	image1->SetValue(1, 0, -2.0);
	ThresholdTools::WinsorizedMeanAndStdDev(image1, emptyMask, mean, variance);
	Asserter::AssertAlmostEqual(mean, 0.0, "Mean of almost empty image");
	Asserter::AssertAlmostEqual(variance, 0.0, "Variance of almost empty image");
	
	Mask2DPtr mask1 = Mask2D::CreateSetMaskPtr<false>(100, 1);
	for(unsigned x=50;x<100;++x)
		mask1->SetValue(x, 0, true);
	ThresholdTools::WinsorizedMeanAndStdDev(image1, mask1, mean, variance);
	Asserter::AssertAlmostEqual(mean, 0.0, "Mean of 50% flagged image");
	Asserter::AssertAlmostEqual(variance, 0.0, "Variance of 50% flagged image");
	
	for(unsigned x=0;x<50;++x)
		image1->SetValue(x, 0, 1.0);
	ThresholdTools::WinsorizedMeanAndStdDev(image1, mask1, mean, variance);
	Asserter::AssertAlmostEqual(mean, 1.0, "Mean of 50% flagged image");
	Asserter::AssertAlmostEqual(variance, 0.0, "Variance of 50% flagged image");

	for(unsigned x=0;x<25;++x)
		image1->SetValue(x, 0, -1.0);
	ThresholdTools::WinsorizedMeanAndStdDev(image1, mask1, mean, variance);
	Asserter::AssertAlmostEqual(mean, 0.0, "Mean of 50% flagged image");
	// Remember: since the distribution is not Gaussian, the variance does not correspond with
	// the Winsorized variance. The sqrtn(1.54) should be the correction term.
	Asserter::AssertAlmostEqual(variance, 1.0 * sqrtn(1.54), "Mean of 50% flagged image");

	for(unsigned x=0;x<100;++x)
		image1->SetValue(x, 0, x+1.0);
	ThresholdTools::WinsorizedMeanAndStdDev(image1, mask1, mean, variance);
	Asserter::AssertAlmostEqual(mean, 25.5, "Mean of 50% flagged sequencial image");
	// Since the distribution is not Gaussian, the variance does not correspond with
	// the Winsorized variance. Therefore, don't test it here. TODO
}


void ThresholdToolsTest::WinsorizedMaskedMode::operator()()
{
	num_t mode;
	Mask2DCPtr fullMask = Mask2D::CreateSetMaskPtr<true>(100, 1);
	Image2DPtr image1 = Image2D::CreateZeroImagePtr(100, 1);
	// we just don't want it to crash
	ThresholdTools::WinsorizedMode(image1, fullMask);

	Mask2DCPtr emptyMask = Mask2D::CreateSetMaskPtr<false>(100, 1);
  mode = ThresholdTools::WinsorizedMode(image1, emptyMask);
	Asserter::AssertAlmostEqual(mode, 0.0, "Mean of zero image");

	image1->SetValue(0, 0, 1.0);
	mode = ThresholdTools::WinsorizedMode(image1, emptyMask);
	Asserter::AssertAlmostEqual(mode, 0.0, "Mode of almost empty image");

	Mask2DPtr mask1 = Mask2D::CreateSetMaskPtr<false>(100, 1);
	for(unsigned x=50;x<100;++x)
		mask1->SetValue(x, 0, true);
	mode = ThresholdTools::WinsorizedMode(image1, mask1);
	Asserter::AssertAlmostEqual(mode, 0.0, "Mode of 50% flagged image with only zeros");
	
	for(unsigned x=0;x<50;++x)
		image1->SetValue(x, 0, 1.0);
	mode = ThresholdTools::WinsorizedMode(image1, mask1);
	Asserter::AssertAlmostEqual(mode, sqrt(0.5) * 1.0541, "Mode of non-zero 50% flagged image");

	//for(unsigned x=0;x<100;++x)
	//	image1->SetValue(x, 0, x+1.0);
	//mode = ThresholdTools::WinsorizedMeanAndStdDev(image1, mask1, mean, variance);
	//Asserter::AssertAlmostEqual(mean, 25.5, "Mean of 50% flagged sequencial image");
	// Since the distribution is not Gaussian, the variance does not correspond with
	// the Winsorized variance. Therefore, don't test it here. TODO
}

#endif
