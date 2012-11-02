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
#include <AOFlagger/strategy/algorithms/imagetile.h>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/fitsfile.h>
#include <AOFlagger/msio/pngfile.h>

#include <AOFlagger/util/rng.h>

#include <AOFlagger/strategy/algorithms/thresholdmitigater.h>

#include <iostream>
#include <math.h>

void ImageTile::InitializeData(unsigned channelCount, unsigned scanCount, unsigned channelOffset, unsigned scanOffset, const Image2D &image, int polynomialTimeOrder, int polynomialFequencyOrder) {
	_channelCount = channelCount;
	_scanCount = scanCount;
	_channelOffset = channelOffset;
	_scanOffset = scanOffset;
	_image = &image;
	_timeOrder = polynomialTimeOrder;
	_freqOrder = polynomialFequencyOrder;
	_useMPF = false;
	_isWindowed = new bool*[_channelCount];
	for(unsigned j=0;j<_channelCount;++j) {
		_isWindowed[j] = new bool[_scanCount];
		for(unsigned i=0;i<_scanCount;++i)	
			_isWindowed[j][i] = false;
	}
	_baselineConsts = new long double[polynomialTimeOrder + polynomialFequencyOrder + 1];
	for(int i = 0;i < polynomialTimeOrder + polynomialFequencyOrder + 1;++i)
		_baselineConsts[i] = 1e-5;
	_trigger = 5.0;
}

void ImageTile::WindowSquare(unsigned scanIndex, unsigned frequencyIndex)
{
	int sstart = scanIndex - 2;
	int send = scanIndex + 2;
	int fstart = frequencyIndex - 2;
	int fend = frequencyIndex + 2;
	if(sstart < 0) sstart = 0;
	if(send >= (int) _scanCount) send = _scanCount-1;
	if(fstart < 0) fstart = 0;
	if(fend >= (int) _channelCount) fend = _channelCount-1;
	for(int f = fstart; f <= fend; ++f) {
		for(int s = sstart; s <= send; ++s) {
			_isWindowed[f][s] = true;
		}
	}
}

void ImageTile::ConvolveWindows()
{
	bool **oldWindows = _isWindowed;

	_isWindowed = new bool*[_channelCount];
	for(unsigned j=0;j<_channelCount;++j) {
		_isWindowed[j] = new bool[_scanCount];
		for(unsigned i=0;i<_scanCount;++i)	
			_isWindowed[j][i] = false;
	}

	for(size_t f=0;f<_channelCount;++f) {
		for(size_t t=0;t<_scanCount;++t) {
			if(oldWindows[f][t])
				WindowSquare(t, f);
		}
	}

	for(size_t f=0;f<_channelCount;++f) {
		delete oldWindows[f];
	}
	delete oldWindows;
}

unsigned ImageTile::WindowCount() const {
	unsigned count = 0;
	for(unsigned channel=0; channel<_channelCount; ++channel) {
		for(unsigned scan=0; scan<_scanCount; ++scan) {
			if(_isWindowed[channel][scan]) count++;
		}
	}
	return count;
}

void ImageTile::FitBackground()
{

	// Chose to use the Levenberg-Marquardt solver with scaling
	const gsl_multifit_fdfsolver_type * T = gsl_multifit_fdfsolver_lmsder;

	// Construct solver
	gsl_multifit_fdfsolver *solver = gsl_multifit_fdfsolver_alloc (T, _channelCount * _scanCount, _timeOrder + _freqOrder + 1);
	if(solver == 0) throw std::exception();

	// Initialize function information structure
	gsl_multifit_function_fdf functionInfo;

	/*if(_useMPF) {
		functionInfo.f = &BaselineFunctionMPF;
		functionInfo.df = &BaselineDerivativeMPF;
		functionInfo.fdf = &BaselineCombinedMPF;
		// chose 256 bits precision for intermediate values in the evaluation of the function and its derivative
		mpf_set_default_prec (256); 
	} else {*/
		functionInfo.f = &BaselineFunction;
		functionInfo.df = &BaselineDerivative;
		functionInfo.fdf = &BaselineCombined;
	//}
	functionInfo.n = _channelCount * _scanCount;
	functionInfo.p = _timeOrder + _freqOrder + 1;
	functionInfo.params = this;

	// Initialize initial value of parameters
	//gsl_vector x;
	double x_init[_timeOrder + _freqOrder + 1];
	for(int i = 0;i < _timeOrder + _freqOrder + 1;++i)
		x_init[i] = _baselineConsts[i];
	gsl_vector_view x_view = gsl_vector_view_array (x_init, _timeOrder + _freqOrder + 1);
	
	gsl_multifit_fdfsolver_set (solver, &functionInfo, &x_view.vector);

	// Start iterating
	int status, iter=0;
	do {
		iter++;
		status = gsl_multifit_fdfsolver_iterate(solver);
		//PrintState(iter, solver);

		if (status && status != GSL_CONTINUE) {
			// std::cout << "Error: status = " << gsl_strerror (status) << std::endl;
			break;
		}

		status = gsl_multifit_test_delta(solver->dx, solver->x, 0, 0);
	} while (status == GSL_CONTINUE && iter < 250);
	
	// Save coefficients
	for(int i = 0;i<_freqOrder + _timeOrder + 1;++i)
		this->_baselineConsts[i] = gsl_vector_get(solver->x, i);

	//PrintState(iter, solver);

	// Clean up
	gsl_multifit_fdfsolver_free(solver);
}

void ImageTile::FirstWindowGuess(long double mean, long double variance)
{
	// Window everything higher than trigger * sigma
	/*for(unsigned channel = 0;channel<_channelCount;++channel) {
		for(unsigned scan = 0;scan<_scanCount;++scan) {
			// Since we have no baseline yet, we just use the mean as a first guess
			if(fabs(GetValueAt(channel, scan) - mean) > _trigger * variance && IsValueSet(channel, scan)) {
				Window(channel, scan);
			}
		}
	}
	ConvolveWindows();*/
	LineThreshold(false, mean, variance, true);

	for(unsigned channel = 0;channel<_channelCount;++channel) {
		for(unsigned scan = 0;scan<_scanCount;++scan) {
			_isWindowed[channel][scan] = true;
		}
	}
}

/*
long double ImageTile::EvaluateBaselineFunction(unsigned scan, unsigned channel) const 
{
	mpf_t term, tmpA, tmpB;
	mpf_init(term);
	mpf_init(tmpA);
	mpf_init(tmpB);
	mpf_set_d(term, _baselineConsts[0]);
	for(int j=1;j <= _freqOrder;++j) {
		// term += const[j] * pow(channel, j);
		mpf_set_ui(tmpA, channel);
		mpf_pow_ui(tmpA, tmpA, j);
		mpf_set_d(tmpB, _baselineConsts[j]);
		mpf_mul(tmpA, tmpA, tmpB);
		mpf_add(term, term, tmpA);
	}
	for(int j=1;j <= _timeOrder;++j) {
		// term += const[j] * pow(scan, j);
		mpf_set_ui(tmpA, scan);
		mpf_pow_ui(tmpA, tmpA, j);
		mpf_set_d(tmpB, _baselineConsts[j + _freqOrder]);
		mpf_mul(tmpA, tmpA, tmpB);
		mpf_add(term, term, tmpA);
	}
	double result = mpf_get_d(term);
	mpf_clear(term);
	mpf_clear(tmpA);
	mpf_clear(tmpB);
	return result;
}*/

long double ImageTile::EvaluateBaselineFunction(unsigned, unsigned) const 
{
	return 0.0; // TODO
}

void ImageTile::AddBaseline(Image2D &dest, double sign)
{
	// Add or subtract baseline
	for(unsigned channel = 0;channel<_channelCount;++channel) {
		for(unsigned scan = 0;scan<_scanCount;++scan) {
			long double val = dest.Value(scan+_scanOffset, channel+_channelOffset);
			val += sign * EvaluateBaselineFunction(scan, channel);
			SetValueAt(dest, channel, scan, val);
		}
	}
}

void ImageTile::SetWindows(long double variance, bool convolve) {
	bool methodA = false;
	bool methodB = false;
	bool methodC = true;

	// Unwindow everything
	for(unsigned channel = 0;channel<_channelCount;++channel) {
		for(unsigned scan = 0;scan<_scanCount;++scan)
			_isWindowed[channel][scan] = false;
	}

	if(methodA) {
		// Window everything higher than trigger * sigma
		for(unsigned channel = 0;channel<_channelCount;++channel) {
			for(unsigned scan = 0;scan<_scanCount;++scan) {
				if(fabsl(GetValueAt(channel, scan) - EvaluateBaselineFunction(scan, channel)) > _trigger * variance) {
					Window(scan, channel);
				}
			}
		}
	}
	if(methodB) {
		for(unsigned channel = 0;channel<_channelCount;++channel) {
			for(unsigned scan = 0;scan<_scanCount;++scan) {
				bool triggered =
					TriggeredRaise(channel, scan, channel-1, scan-1, variance) ||
					TriggeredRaise(channel, scan, channel-1, scan+1, variance) ||
					TriggeredRaise(channel, scan, channel+1, scan-1, variance) ||
					TriggeredRaise(channel, scan, channel+1, scan+1, variance);
				if(triggered)
					Window(scan, channel);
			}
		}
	}
	if(convolve)
		ConvolveWindows();
	if(methodC) {
		LineThreshold(true, 0.0, variance, convolve);
	}
}

void ImageTile::LineThreshold(bool evaluateBaseline, long double mean, long double variance, bool convolve)
{
	Image2DPtr input = Image2D::CreateEmptyImagePtr(_scanCount, _channelCount);
	Mask2DPtr output = Mask2D::CreateSetMaskPtr<false>(_scanCount, _channelCount);
	if(evaluateBaseline) {
		for(unsigned channel = 0;channel<_channelCount;++channel)
			for(unsigned scan = 0;scan<_scanCount;++scan)
				input->SetValue(scan, channel, GetValueAt(channel, scan) - EvaluateBaselineFunction(scan, channel));
	} else {
		for(unsigned channel = 0;channel<_channelCount;++channel)
			for(unsigned scan = 0;scan<_scanCount;++scan)
				input->SetValue(scan, channel, GetValueAt(channel, scan) - mean);
	}
	ThresholdMitigater::SumThreshold(input, output, 1, _trigger * variance);
	ThresholdMitigater::SumThreshold(input, output, 2, _trigger * variance * 1.6);
	ThresholdMitigater::SumThreshold(input, output, 3, _trigger * variance * 2.2);
	ThresholdMitigater::SumThreshold(input, output, 5, _trigger * variance * 3.0);
	ThresholdMitigater::SumThreshold(input, output, 10, _trigger * variance * 5.0);
	unsigned count = 0;
	for(unsigned channel = 0;channel<_channelCount;++channel) {
		for(unsigned scan = 0;scan<_scanCount;++scan) {
			if(output->Value(scan, channel)) {
				Window(scan, channel);
				count++;
			}
		}
	}
	while(count*2 > _channelCount*_scanCount) {
		size_t x = (size_t) (RNG::Uniform()*_scanCount);
		size_t y = (size_t) (RNG::Uniform()*_channelCount);
		if(_isWindowed[y][x]) {
			count--;
			_isWindowed[y][x]=false;
		}
	}
	if(convolve)
		ConvolveWindows();
}

bool ImageTile::TriggeredRaise(unsigned channelA, unsigned scanA, unsigned channelB, unsigned scanB, long double variance) const
{
	if(channelB >= _channelCount) return false;
	if(scanB >= _scanCount) return false;
	long double value = GetValueAt(channelA, scanA) - GetValueAt(channelB, scanB);
	return value > _trigger * variance * 2.0;
}

void ImageTile::SaveBackgroundToPng(const std::string &filename) {
	ColdHotMap coldHotMap;
	PosLogMap logMap(coldHotMap);
	PngFile file(filename, _scanCount, _channelCount);
	file.BeginWrite();
	file.Clear(0, 0, 0, 255);
	long double norm = _image->GetMaxMinNormalizationFactor();
	for(unsigned channel=0;channel < _channelCount; ++channel) {
		for(unsigned scan=0;scan < _scanCount; ++scan) {
			double val = norm * EvaluateBaselineFunction(scan, channel);
			file.PlotDatapoint(scan, channel, logMap.ValueToColorR(val), logMap.ValueToColorG(val), logMap.ValueToColorB(val), logMap.ValueToColorA(val));
		}
	}
	file.Close();
}

long double ImageTile::GetValueAt(unsigned frequencyIndex, unsigned scanIndex) const {
	return _image->Value(_scanOffset + scanIndex, _channelOffset + frequencyIndex);
}

void ImageTile::SetValueAt(Image2D &dest, unsigned frequencyIndex, unsigned scanIndex, long double newValue) {
	return dest.SetValue(_scanOffset + scanIndex, _channelOffset + frequencyIndex, newValue);
}
