#ifndef VERTEVD_H
#define VERTEVD_H

#include <cmath>
#include <vector>

#include "../../msio/timefrequencydata.h"

#include "../../util/aologger.h"

#include "eigenvalue.h"

/**
 * This class performs a horizontal Eigenvalue Decomposition. Horizon
 */
class VertEVD
{
	public:
		static void Perform(TimeFrequencyData &data, bool timeIntegrated)
		{
			if(data.PolarisationCount() != 1)
				throw std::runtime_error("Can not decompose multipolarization data");
			if(data.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation)
				throw std::runtime_error("Can only decompose complex data");
			
			Image2DCPtr
				real = data.GetRealPart(),
				imaginary = data.GetImaginaryPart();
			Image2DPtr
				outReal = Image2D::CreateZeroImagePtr(real->Width(), real->Height()),
				outImaginary = Image2D::CreateZeroImagePtr(real->Width(), real->Height());
				
			// Since the number of vertical elements e = n(n-1)/2 , solving for e
			// results in:
			// e = n(n-1)/2
			// e = 0.5n^2 - 0.5n
			// 0 = 0.5n^2 - 0.5n - e
			// n = -(-0.5) +- sqrt(0.25 - 4 x 0.5 x -e)
			// n = 0.5 + sqrt(0.25 + 2e)
			const size_t n = floor(0.5 + sqrt(0.25 + 2 * real->Height()));
			const size_t skipped = real->Height()-((n * (n-1)) / 2);
			if(n > 0)
			{
				AOLogger::Warn << "In vertical eigenvalue decomposition: height did not correspond with an exact triangle:\n"
				<< skipped << " values of height " << (real->Height()) << " were skipped to create matrix size " << n << "\n.";
			}
			Image2DPtr
				realMatrix = Image2D::CreateUnsetImagePtr(n, n),
				imaginaryMatrix = Image2D::CreateUnsetImagePtr(n, n);
			
			if(timeIntegrated)
			{
				performIntegrated(n, real, imaginary, realMatrix, imaginaryMatrix, outReal, outImaginary);
			} else {				
				for(unsigned t=0;t<real->Width();++t)
				{
					performOnEachTimestep(n, t, real, imaginary, realMatrix, imaginaryMatrix, outReal, outImaginary);
				}
			}
			
			data.SetImage(0, outReal);
			data.SetImage(1, outImaginary);
		}
	private:
		static void performIntegrated(const unsigned n, Image2DCPtr real, Image2DCPtr imaginary, Image2DPtr realMatrix, Image2DPtr imaginaryMatrix, Image2DPtr outReal, Image2DPtr outImaginary)
		{
			std::vector<double> diagonal(n);
			for(unsigned i=0;i<n;++i)
				diagonal[i] = 0.0;

			for(unsigned diagIteration=0;diagIteration<5;++diagIteration)
			{
				unsigned index = 0;
				for(unsigned y=0;y<n;++y)
				{
					realMatrix->SetValue(y, y, diagonal[y]);
					imaginaryMatrix->SetValue(y, y, 0.0);

					for(unsigned x=y+1;x<n;++x)
					{
						realMatrix->SetValue(x, y, 0.0);
						imaginaryMatrix->SetValue(x, y, 0.0);
						realMatrix->SetValue(y, x, 0.0);
						imaginaryMatrix->SetValue(y, x, 0.0);
						
						for(unsigned t=0;t<real->Width();++t)
						{
							realMatrix->SetValue(x, y, real->Value(t, index) + realMatrix->Value(x, y));
							imaginaryMatrix->SetValue(x, y, imaginary->Value(t, index) + imaginaryMatrix->Value(x, y));

							realMatrix->SetValue(y, x, real->Value(t, index) + realMatrix->Value(y, x));
							imaginaryMatrix->SetValue(y, x, -imaginary->Value(t, index) + imaginaryMatrix->Value(y, x));
						}
						++index;
					}
				}
				AOLogger::Debug << "Input to EVD:\n";
				for(unsigned y=0;y<realMatrix->Height();++y) {
					for(unsigned x=0;x<realMatrix->Width();++x) {
						AOLogger::Debug << realMatrix->Value(x, y) << ' ';
					}
					AOLogger::Debug << '\n';
				}
				
				Eigenvalue::Remove(realMatrix, imaginaryMatrix, true);
				
				AOLogger::Debug << "Output to EVD:\n";
				for(unsigned y=0;y<realMatrix->Height();++y) {
					for(unsigned x=0;x<realMatrix->Width();++x) {
						AOLogger::Debug << realMatrix->Value(x, y) << ' ';
					}
					AOLogger::Debug << '\n';
				}
				
				for(unsigned i=0;i<n;++i)
					diagonal[i] = realMatrix->Value(i, i);
			}						
			
			unsigned index = 0;
			for(unsigned y=0;y<n;++y)
			{
				for(unsigned x=y+1;x<n;++x)
				{
					for(unsigned t=0;t<real->Width();++t)
					{
						outReal->SetValue(t, index, /*real->Value(t, index) -*/ realMatrix->Value(x, y)/(num_t) real->Width());
						outImaginary->SetValue(t, index, /*imaginary->Value(t, index) -*/ imaginaryMatrix->Value(x, y)/(num_t) imaginary->Width());
					}
					++index;
				}
			}
		}
		
		static void performOnEachTimestep(const unsigned n, const unsigned t, Image2DCPtr real, Image2DCPtr imaginary, Image2DPtr realMatrix, Image2DPtr imaginaryMatrix, Image2DPtr outReal, Image2DPtr outImaginary)
		{
			std::vector<double> diagonal(n);
			for(unsigned i=0;i<n;++i)
				diagonal[i] = 0.0;
			
			for(unsigned diagIteration=0;diagIteration<5;++diagIteration)
			{
				unsigned index = 0;
				for(unsigned y=0;y<n;++y)
				{
					realMatrix->SetValue(y, y, diagonal[y]);
					imaginaryMatrix->SetValue(y, y, 0.0);
					
					for(unsigned x=y+1;x<n;++x)
					{
						realMatrix->SetValue(x, y, real->Value(t, index));
						imaginaryMatrix->SetValue(x, y, imaginary->Value(t, index));

						realMatrix->SetValue(y, x, real->Value(t, index));
						imaginaryMatrix->SetValue(y, x, -imaginary->Value(t, index));

						++index;
					}
				}
				
				if(t == 262)
				{
					AOLogger::Debug << "Input to EVD:\n";
					for(unsigned y=0;y<realMatrix->Height();++y) {
						for(unsigned x=0;x<realMatrix->Width();++x) {
							AOLogger::Debug << realMatrix->Value(x, y) << ' ';
						}
						AOLogger::Debug << '\n';
					}
				}
				
				Eigenvalue::Remove(realMatrix, imaginaryMatrix, t==262);
				
				if(t == 262)
				{
					AOLogger::Debug << "Output to EVD:\n";
					for(unsigned y=0;y<realMatrix->Height();++y) {
						for(unsigned x=0;x<realMatrix->Width();++x) {
							AOLogger::Debug << realMatrix->Value(x, y) << ' ';
						}
						AOLogger::Debug << '\n';
					}
				}
				
				for(unsigned i=0;i<n;++i)
					diagonal[i] = realMatrix->Value(i, i);
			}

			unsigned index = 0;
			for(unsigned y=0;y<n;++y)
			{
				for(unsigned x=y+1;x<n;++x)
				{
					outReal->SetValue(t, index, realMatrix->Value(x, y));
					outImaginary->SetValue(t, index, imaginaryMatrix->Value(x, y));
					++index;
				}
			}
			
			while(index < real->Height())
			{
				outReal->SetValue(t, index, 0.0);
				outImaginary->SetValue(t, index, 0.0);
				++index;
			}
		}
		
		VertEVD() { }
};

#endif
