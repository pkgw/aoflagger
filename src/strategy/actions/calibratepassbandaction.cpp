#include "calibratepassbandaction.h"

#include "../algorithms/thresholdtools.h"

#include <xmmintrin.h>

#include <vector>

namespace rfiStrategy {

	void CalibratePassbandAction::calibrate(TimeFrequencyData& data) const
	{
		const size_t height = data.ImageHeight();
		std::vector<num_t> stddev(_steps);
		for(size_t step=0; step!=_steps; ++step)
		{
			const size_t startY = step*height/_steps, endY = (step+1)*height/_steps;
			std::vector<num_t> dataVector((1+endY-startY) * data.ImageWidth() * data.ImageCount());
			std::vector<num_t>::iterator vecIter = dataVector.begin();
			const Mask2DCPtr maskPtr = data.GetSingleMask();
			const Mask2D &mask = *maskPtr;
			for(size_t i=0; i!=data.ImageCount(); ++i)
			{
				const Image2D &image = *data.GetImage(i);
				for(size_t y=startY; y!=endY; ++y)
				{
					const num_t *inputPtr = image.ValuePtr(0, y);
					const bool *maskPtr = mask.ValuePtr(0, y);
					for(size_t x=0; x!=image.Width(); ++x)
					{
						if(!*maskPtr && std::isfinite(*inputPtr))
						{
							*vecIter = *inputPtr;
							++vecIter;
						}
						++inputPtr;
						++maskPtr;
					}
				}
			}
			dataVector.resize(vecIter - dataVector.begin());
			
			num_t mean;
			ThresholdTools::WinsorizedMeanAndStdDev<num_t>(dataVector, mean, stddev[step]);
		}
			
		for(size_t i=0; i!=data.ImageCount(); ++i)
		{
			const Image2D &image = *data.GetImage(i);
			Image2D *destImage = Image2D::CreateUnsetImage(image.Width(), image.Height());
			for(size_t step=0; step!=_steps; ++step)
			{
				const size_t startY = step*height/_steps, endY = (step+1)*height/_steps;
				float correctionFactor;
				if(stddev[step] == 0.0)
					correctionFactor = 0.0;
				else
					correctionFactor = 1.0 / stddev[step];
				const __m128 corrFact4 = _mm_set_ps(correctionFactor, correctionFactor, correctionFactor, correctionFactor);
				
				for(size_t y=startY; y!=endY; ++y)
				{
					const float *inputPtr = image.ValuePtr(0, y);
					float *destPtr = destImage->ValuePtr(0, y);
					
					for(size_t x=0;x<image.Width();x+=4)
					{
						_mm_store_ps(destPtr, _mm_mul_ps(corrFact4, _mm_load_ps(inputPtr)));
						inputPtr += 4;
						destPtr += 4;
					}
				}
			}
			data.SetImage(i, Image2DPtr(destImage));
		}
	}

}
