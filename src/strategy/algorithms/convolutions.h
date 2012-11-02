#ifndef CONVOLUTIONS_H
#define CONVOLUTIONS_H

#include "../../msio/types.h"

#include "../../util/rng.h"

#include <cmath>

class Convolutions
{
	public:
		/**
		* This function will convolve data with the specified kernel. It will
		* place the element at position (kernelSize/2) of the kernel in the
		* centre, i.e. data[0] := sum over i in kS : data[i - kS + _kS/2_] * kernel[i].
		* Therefore, it makes most sense to specify an odd kernelSize if the
		* kernel consists of a peak / symmetric function.
		*
		* This function assumes the function represented by @c data is zero outside
		* its domain [0..dataSize}.
		*
		* This function does not reverse one of the input functions, therefore this function
		* is not adhering to the strict definition of a convolution.
		*
		* This function does not use an FFT to calculate the convolution, and is therefore
		* O(dataSize x kernelSize).
		*
		* @param data The data to be convolved (will also be the output)
		* @param dataSize Number of samples in data
		* @param kernel The kernel to be convolved, central sample = kernelSize/2
		* @param kernelSize Number of samples in the kernel, probably desired to be
		* odd if the kernel is symmetric.
		*/
		static void OneDimensionalConvolutionBorderZero(num_t *data, unsigned dataSize, const num_t *kernel, unsigned kernelSize)
		{
			num_t *tmp = new num_t[dataSize]; 
			for(unsigned i=0;i<dataSize;++i)
			{
				unsigned kStart, kEnd;
				const int offset = i - kernelSize/2;
				// Make sure that kStart >= 0
				if(offset < 0)
					kStart = -offset;
				else
					kStart = 0;
				// Make sure that kEnd + offset <= dataSize
				if(offset + kernelSize > dataSize)
					kEnd = dataSize - offset;
				else
					kEnd = kernelSize;
				num_t sum = 0.0;
				for(unsigned k=kStart;k<kEnd;++k)
					sum += data[k + offset]*kernel[k];
				tmp[i] = sum;
			}
			for(unsigned i=0;i<dataSize;++i)
				data[i] = tmp[i];
			delete[] tmp;
		}
		
		/**
		* This function will convolve data with the specified kernel. It will
		* place the element at position (kernelSize/2) of the kernel in the
		* centre, i.e. data[0] := sum over i in kS : data[i - kS + _kS/2_] * kernel[i].
		* Therefore, it makes most sense to specify an odd kernelSize if the
		* kernel consists of a peak / symmetric function.
		*
		* This function assumes the function represented by @c data is not zero outside
		* its domain [0..dataSize}, but is "unknown". It assumes that the integral
		* over @c kernel is one. If this is not the case, you have to multiply all output
		* values with the kernels integral after convolution.
		*
		* This function does not reverse one of the input functions, therefore this function
		* is not adhering to the strict definition of a convolution.
		*
		* This function does not use an FFT to calculate the convolution, and is therefore
		* O(dataSize x kernelSize).
		*
		* @param data The data to be convolved (will also be the output)
		* @param dataSize Number of samples in data
		* @param kernel The kernel to be convolved, central sample = kernelSize/2
		* @param kernelSize Number of samples in the kernel, probably desired to be
		* odd if the kernel is symmetric.
		*/
		static void OneDimensionalConvolutionBorderInterp(num_t *data, unsigned dataSize, const num_t *kernel, unsigned kernelSize)
		{
			num_t *tmp = new num_t[dataSize]; 
			for(unsigned i=0;i<dataSize;++i)
			{
				unsigned kStart, kEnd;
				const int offset = i - kernelSize/2;
				// Make sure that kStart >= 0
				if(offset < 0)
					kStart = -offset;
				else
					kStart = 0;
				// Make sure that kEnd + offset <= dataSize
				if(offset + kernelSize > dataSize)
					kEnd = dataSize - offset;
				else
					kEnd = kernelSize;
				num_t sum = 0.0;
				num_t weight = 0.0;
				for(unsigned k=kStart;k<kEnd;++k)
				{
					sum += data[k + offset]*kernel[k];
					weight += kernel[k];
				}
				// Weighting is performed to correct for the "missing data" outside the domain of data.
				// The actual correct value should be sum * (sumover kernel / sumover kernel-used ).
				// We however assume "sumover kernel" to be 1. sumover kernel-used is the weight.
				tmp[i] = sum / weight;
			}
			for(unsigned i=0;i<dataSize;++i)
				data[i] = tmp[i];
			delete[] tmp;
		}
		
		static void OneDimensionalGausConvolution(num_t *data, unsigned dataSize, num_t sigma)
		{
			unsigned kernelSize = (unsigned) roundn(sigma*3.0L);
			if(kernelSize%2 == 0) ++kernelSize;
			if(kernelSize > dataSize*2)
			{
				if(dataSize == 0) return;
				kernelSize = dataSize*2 - 1;
			}
			unsigned centreElement = kernelSize/2;
			num_t *kernel = new num_t[kernelSize];
			for(unsigned i=0;i<kernelSize;++i)
			{
				num_t x = ((num_t) i-(num_t) centreElement);
				kernel[i] = RNG::EvaluateGaussian(x, sigma);
			}
			OneDimensionalConvolutionBorderInterp(data, dataSize, kernel, kernelSize);
			delete[] kernel;
		}
		
		/**
		* Perform a sinc convolution. This is equivalent with a low-pass filter,
		* where @c frequency specifies the frequency in index units.
		*
		* With frequency=1, the data will be convolved with the
		* delta function and have no effect. With frequency = 0.25,
		* all fringes faster than 0.25 fringes/index units will be
		* filtered.
		*
		* The function is O(dataSize^2).
		*/
		static void OneDimensionalSincConvolution(num_t *data, unsigned dataSize, num_t frequency)
		{
			if(dataSize == 0) return;
			const unsigned kernelSize = dataSize*2 - 1;
			const unsigned centreElement = kernelSize/2;
			num_t *kernel = new num_t[kernelSize];
			const numl_t factor = 2.0 * frequency * M_PInl;
			numl_t sum = 0.0;
			for(unsigned i=0;i<kernelSize;++i)
			{
				const numl_t x = (((numl_t) i-(numl_t) centreElement) * factor);
				if(x!=0.0)
					kernel[i] = (num_t) (sinnl(x) / x);
				else
					kernel[i] = 1.0;
				sum += kernel[i];
			}
			for(unsigned i=0;i<kernelSize;++i)
			{
				kernel[i] /= sum;
			}
			OneDimensionalConvolutionBorderZero(data, dataSize, kernel, kernelSize);
			delete[] kernel;
		}

		/**
		* Perform a sinc convolution, with a convolution kernel that has been Hamming windowed.
		* Because of the hamming window, the filter has less of a steep cutting edge, but less ripples. 
		*
		* See OneDimensionalSincConvolution() for more info.
		*
		* The function is O(dataSize^2).
		*/
		static void OneDimensionalSincConvolutionHammingWindow(num_t *data, unsigned dataSize, num_t frequency)
		{
			if(dataSize == 0) return;
			const unsigned kernelSize = dataSize*2 - 1;
			const unsigned centreElement = kernelSize/2;
			num_t *kernel = new num_t[kernelSize];
			const numl_t sincFactor = 2.0 * frequency * M_PInl;
			const numl_t hammingFactor = 2.0 * M_PInl * (numl_t) (kernelSize-1);
			numl_t sum = 0.0;
			for(unsigned i=0;i<kernelSize;++i)
			{
				const numl_t hamming = 0.54 - 0.46 * cosnl(hammingFactor * (numl_t) i);
				const numl_t x = (((numl_t) i-(numl_t) centreElement) * sincFactor);
				if(x!=0.0)
					kernel[i] = (num_t) (sinnl(x) / x) * hamming;
				else
					kernel[i] = 1.0;
				sum += kernel[i];
			}
			for(unsigned i=0;i<kernelSize;++i)
			{
				kernel[i] /= sum;
			}
			OneDimensionalConvolutionBorderZero(data, dataSize, kernel, kernelSize);
			delete[] kernel;
		}
};

#endif
