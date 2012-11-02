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

#include "timeconvolutionaction.h"

#include <fftw3.h>

namespace rfiStrategy {

void TimeConvolutionAction::PerformFFTSincOperation(ArtifactSet &artifacts, Image2DPtr real, Image2DPtr imag) const
{
	fftw_complex
		*fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * real->Width()),
		*fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * real->Width());
	
	// FFTW plan routines are not thread safe, so lock.
	boost::mutex::scoped_lock lock(artifacts.IOMutex());
	fftw_plan
		fftPlanForward = fftw_plan_dft_1d(real->Width(), fftIn, fftOut, FFTW_FORWARD, FFTW_MEASURE),
		fftPlanBackward = fftw_plan_dft_1d(real->Width(), fftIn, fftOut, FFTW_BACKWARD, FFTW_MEASURE);
	lock.unlock();
	
	const size_t width = real->Width();

	const BandInfo band = artifacts.MetaData()->Band();
	for(unsigned y=0;y<real->Height();++y)
	{
		const numl_t sincScale = ActualSincScaleInSamples(artifacts, band.channels[y].frequencyHz);
		const numl_t limitFrequency = (numl_t) width / sincScale;
		if(y == real->Height()/2)
		{
			AOLogger::Debug << "Horizontal sinc scale: " << sincScale << " (filter scale: " << Angle::ToString(ActualSincScaleAsRaDecDist(artifacts, band.channels[y].frequencyHz)) << ")\n";
		}
		if(sincScale > 1.0)
		{
			for(unsigned x=0;x<width;++x)
			{
				fftIn[x][0] = real->Value(x, y);
				fftIn[x][1] = imag->Value(x, y);
			}
			
			fftw_execute_dft(fftPlanForward, fftIn, fftOut);
			size_t filterIndexSize = (limitFrequency > 1.0) ? (size_t) ceil(limitFrequency/2.0) : 1;
			// Remove the high frequencies [filterIndexSize : n-filterIndexSize]
			for(size_t f=filterIndexSize;f<width - filterIndexSize;++f)
			{
				fftOut[f][0] = 0.0;
				fftOut[f][1] = 0.0;
			}
			fftw_execute_dft(fftPlanBackward, fftOut, fftIn);
			
			const double n = width;
			for(unsigned x=0;x<width;++x)
			{
				real->SetValue(x, y, fftIn[x][0] / n);
				imag->SetValue(x, y, fftIn[x][1] / n);
			}
		}
	}
	fftw_free(fftIn);
	fftw_free(fftOut);
}

}
