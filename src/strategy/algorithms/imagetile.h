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
#ifndef IMAGETILE_H
#define IMAGETILE_H

#include <string>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ImageTile {
	public:
	ImageTile() {	}

	void InitializeData(unsigned channelCount, unsigned scanCount, unsigned channelOffset, unsigned scanOffset, const class Image2D &image, int polynomialTimeOrder, int polynomialFequencyOrder);
	void FirstWindowGuess(long double mean, long double variance);
	void FitBackground();
	void AddBaseline(Image2D &dest, double sign);
	void SetWindows(long double variance, bool convolve);
	long double EvaluateBaselineFunction(unsigned scan, unsigned channel) const;
	void SaveBackgroundToPng(const std::string &filename);
	unsigned WindowCount() const;
	void SetTrigger(long double newTrigger) throw() { _trigger = newTrigger; }
	long double Trigger() const throw() { return _trigger; } 
	bool IsWindowed(unsigned scanIndex, unsigned frequencyIndex) const {
		return _isWindowed[frequencyIndex - _channelOffset][scanIndex - _scanOffset];
	}
	void SetWindowed(unsigned scanIndex, unsigned frequencyIndex, bool newValue) const {
		_isWindowed[frequencyIndex - _channelOffset][scanIndex - _scanOffset] = newValue;
	}


private:
	unsigned _channelCount;
	unsigned _scanCount;
	bool **_isWindowed;
	unsigned _channelOffset;
	unsigned _scanOffset;
	long double *_baselineConsts;
	long double _trigger;
	const class Image2D *_image;
	bool _useMPF;

	int _freqOrder, _timeOrder;

	bool TriggeredRaise(unsigned channelA, unsigned scanA, unsigned channelB, unsigned scanB, long double variance) const;

	long double GetValueAt(unsigned frequencyIndex, unsigned scanIndex) const;
	void SetValueAt(Image2D &dest, unsigned frequencyIndex, unsigned scanIndex, long double newValue);
	void ConvolveWindows();
	void LineThreshold(bool evaluateBaseline, long double mean, long double variance, bool convolve);

	void Window(unsigned scanIndex, unsigned frequencyIndex) {
		_isWindowed[frequencyIndex][scanIndex] = true;
	}
	void WindowSquare(unsigned scanIndex, unsigned frequencyIndex);

	// This function calculates the sum of the squared errors when the parameters in 'x' are used. 
	//static int BaselineFunction(const gsl_vector * x, void *data, gsl_vector * f);

	// This function calculates the sum of the squared errors when the parameters in 'x' are used. 
	//static int BaselineFunctionMPF(const gsl_vector * x, void *data, gsl_vector * f);

	// This function calculates the Jacobian matrix of "BaselineFunction()".
	//static int BaselineDerivative(const gsl_vector * x, void *data, gsl_matrix * J);

	// This function calculates the Jacobian matrix of "BaselineFunction()".
	//static int BaselineDerivativeMPF(const gsl_vector * x, void *data, gsl_matrix * J);

	/*static int BaselineCombined(const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)
	{
		BaselineFunction(x, data, f);
		BaselineDerivative(x, data, J);
		return GSL_SUCCESS;
		}*/

	/8static int BaselineCombinedMPF(const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)
	{
		BaselineFunctionMPF(x, data, f);
		BaselineDerivativeMPF(x, data, J);
		return GSL_SUCCESS;
	}*/

	//void PrintState(unsigned iter, gsl_multifit_fdfsolver *solver);
};

#endif
