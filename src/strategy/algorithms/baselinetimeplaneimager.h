#ifndef BASELINETIMEPLANEIMAGER_H
#define BASELINETIMEPLANEIMAGER_H

#include <complex>

#include "../../msio/image2d.h"

template<typename NumType>
class BaselineTimePlaneImager
{
	public:
		void Image(NumType uTimesLambda, NumType vTimesLambda, NumType wTimesLambda, NumType lowestFrequency, NumType frequencyStep, size_t channelCount, const std::complex<NumType> *data, Image2D &output);
		
	private:
		template<typename T>
		static T frequencyToWavelength(const T frequency)
		{
			return speedOfLight() / frequency; 
		}
		static long double speedOfLight()
		{
			return 299792458.0L;
		}
};

#endif
