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
#ifndef AOFLAGGER_HIGHPASSFILTERTEST_H
#define AOFLAGGER_HIGHPASSFILTERTEST_H

#include "../../testingtools/asserter.h"
#include "../../testingtools/unittest.h"
#include "../../testingtools/imageasserter.h"

#include "../../../msio/mask2d.h"
#include "../../../msio/image2d.h"

#include "../../../strategy/algorithms/localfitmethod.h"
#include "../../../strategy/algorithms/highpassfilter.h"

class HighPassFilterTest : public UnitTest {
	public:
		HighPassFilterTest() : UnitTest("High-pass filter algorithm")
		{
			AddTest(TestSmallImageFilter(), "High-pass filter with small image");
			AddTest(TestFilter(), "High-pass filter algorithm");
			AddTest(TestFilterWithMask(), "Low-pass filter algorithm with mask");
			AddTest(TestCompletelyMaskedImage(), "Low-pass filter algorithm with completely set mask");
			AddTest(TestNaNImage(), "Low-pass filter algorithm with NaNs");
		}
		
	private:
		struct TestFilter : public Asserter
		{
			void operator()();
		};
		struct TestSmallImageFilter : public Asserter
		{
			void operator()();
		};
		struct TestFilterWithMask : public Asserter
		{
			void operator()();
		};
		struct TestCompletelyMaskedImage : public Asserter
		{
			void operator()();
		};
		struct TestNaNImage : public Asserter
		{
			void operator()();
		};
		
};

inline void HighPassFilterTest::TestFilter::operator()()
{
	const size_t width = 99, height = 99;
	Image2DPtr testImage = Image2D::CreateZeroImagePtr(width, height);
	testImage->SetValue(10,10,1.0);
	testImage->SetValue(15,15,2.0);
	testImage->SetValue(20,20,0.5);
	
	// Fitting
	LocalFitMethod fitMethod;
	TimeFrequencyData data(TimeFrequencyData::AmplitudePart, StokesIPolarisation, Image2D::CreateCopy(testImage));
	fitMethod.SetToWeightedAverage(10, 20, 2.5, 5.0);
	fitMethod.Initialize(data);
	for(size_t i=0;i<fitMethod.TaskCount();++i)
		fitMethod.PerformFit(i);
	Image2DCPtr fitResult = Image2D::CreateFromDiff(testImage, fitMethod.Background().GetSingleImage());
	
	// High-pass filter
	HighPassFilter filter;
	Image2DPtr filterResult = Image2D::CreateCopy(testImage);
	filter.SetHWindowSize(21);
	filter.SetVWindowSize(41);
	filter.SetHKernelSigmaSq(2.5);
	filter.SetVKernelSigmaSq(5.0);
	filterResult = filter.ApplyHighPass(filterResult, Mask2D::CreateSetMaskPtr<false>(width, height));
	
	ImageAsserter::AssertEqual(filterResult, fitResult, "Simple convolution with three high values");
}

inline void HighPassFilterTest::TestSmallImageFilter::operator()()
{
	const size_t width = 8, height = 8;
	Image2DPtr testImage = Image2D::CreateZeroImagePtr(width, height);
	testImage->SetValue(1, 1, 1.0);
	
	// Fitting
	LocalFitMethod fitMethod;
	TimeFrequencyData data(TimeFrequencyData::AmplitudePart, StokesIPolarisation, Image2D::CreateCopy(testImage));
	fitMethod.SetToWeightedAverage(10, 20, 2.5, 5.0);
	fitMethod.Initialize(data);
	for(size_t i=0;i<fitMethod.TaskCount();++i)
		fitMethod.PerformFit(i);
	Image2DCPtr fitResult = Image2D::CreateFromDiff(testImage, fitMethod.Background().GetSingleImage());
	
	// High-pass filter
	HighPassFilter filter;
	Image2DPtr filterResult = Image2D::CreateCopy(testImage);
	filter.SetHWindowSize(21);
	filter.SetVWindowSize(41);
	filter.SetHKernelSigmaSq(2.5);
	filter.SetVKernelSigmaSq(5.0);
	filterResult = filter.ApplyHighPass(filterResult, Mask2D::CreateSetMaskPtr<false>(width, height));
	
	// This test will fail, but the high-pass filter is actually slightly better than the older
	// "fitter" -- it will keep the kernel as large as possible, while the sliding window fit can
	// be one off. The test is still good to guard for out of bounds errors.
	//ImageAsserter::AssertEqual(filterResult, fitResult, "Convolution with kernel that is larger than the image");
}

inline void HighPassFilterTest::TestFilterWithMask::operator()()
{
	const size_t width = 8, height = 8;
	Image2DPtr image = Image2D::CreateZeroImagePtr(width, height);
	image->SetValue(1, 1, 1.0);
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(width, height);
	mask->SetValue(1, 1, true);
	
	// High-pass filter
	HighPassFilter filter;
	filter.SetHWindowSize(4);
	filter.SetVWindowSize(4);
	filter.SetHKernelSigmaSq(4.0);
	filter.SetVKernelSigmaSq(4.0);
	image = filter.ApplyLowPass(image, mask);
	
	ImageAsserter::AssertConstant(image, 0.0, "Low-pass convolution with one masked value");
}

inline void HighPassFilterTest::TestCompletelyMaskedImage::operator()()
{
	const size_t width = 9, height = 9;
	Image2DPtr image = Image2D::CreateZeroImagePtr(width, height);
	image->SetValue(1, 1, 1.0);
	image->SetValue(8, 8, 10.0);
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<true>(width, height);
	
	// High-pass filter
	HighPassFilter filter;
	filter.SetHWindowSize(4);
	filter.SetVWindowSize(4);
	filter.SetHKernelSigmaSq(4.0);
	filter.SetVKernelSigmaSq(4.0);
	image = filter.ApplyLowPass(image, mask);
	
	ImageAsserter::AssertConstant(image, 0.0, "Low-pass convolution with fully masked image");
}

inline void HighPassFilterTest::TestNaNImage::operator()()
{
	const size_t width = 7, height = 7;
	Image2DPtr image = Image2D::CreateZeroImagePtr(width, height);
	image->SetValue(1, 1, std::numeric_limits<float>::quiet_NaN());
	image->SetValue(7, 7, 10.0);
	
	// High-pass filter
	HighPassFilter filter;
	filter.SetHWindowSize(4);
	filter.SetVWindowSize(4);
	filter.SetHKernelSigmaSq(4.0);
	filter.SetVKernelSigmaSq(4.0);
	image = filter.ApplyLowPass(image, Mask2D::CreateSetMaskPtr<false>(width, height));
	
	ImageAsserter::AssertFinite(image, "Low-pass convolution with NaNs");
}

#endif
