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
#include <AOFlagger/strategy/algorithms/polfitmethod.h>

PolFitMethod::PolFitMethod() : _background(0), _previousCoefficients(0)
{
}

PolFitMethod::~PolFitMethod()
{
	if(_background!=0)
		delete _background;
}

void PolFitMethod::Initialize(const TimeFrequencyData &input)
{
	_original = input.GetSingleImage();
	_background2D = Image2D::CreateEmptyImagePtr(_original->Width(), _original->Height());
	if(_background!=0)
		delete _background;
	_background = new TimeFrequencyData(input.PhaseRepresentation(), input.Polarisation(), _background2D);
	_mask = input.GetSingleMask();
	if(_hSquareSize * 2 > _original->Width())
		_hSquareSize = _original->Width()/2;
	if(_vSquareSize * 2 > _original->Height())
		_vSquareSize = _original->Height()/2;
}

unsigned PolFitMethod::TaskCount()
{
	return _original->Height();
}

void PolFitMethod::PerformFit(unsigned taskNumber)
{
	if(_mask == 0)
		throw BadUsageException("Mask has not been set!");
	unsigned y = taskNumber;
	for(unsigned x=0;x<_original->Width();++x)
		_background2D->SetValue(x, y, CalculateBackgroundValue(x, y));
}

long double PolFitMethod::CalculateBackgroundValue(unsigned x, unsigned y)
{
	ThreadLocal local;
	local.image = this;
	local.currentX = x;
	local.currentY = y;

	if(local.currentY >= _vSquareSize)
		local.startY = local.currentY-_vSquareSize;
	else
		local.startY = 0;
	local.endY = local.currentY + _vSquareSize;
	if(local.endY >= _original->Height())
		local.endY = _original->Height()-1;

	if(local.currentX >= _hSquareSize)
		local.startX = local.currentX - _hSquareSize;
	else
		local.startX = 0;
	local.endX = local.currentX + _hSquareSize;
		if(local.endX >= _original->Width())
			local.endX = _original->Width()-1;
	local.emptyWindows = 0;

	switch(_method) {
		case None:
			return 0.0;
		case LeastSquare:
		case LeastAbs:
		case FastGaussianWeightedAverage:
			return FitBackground(x, y, local);
		default:
			throw ConfigurationException("The PolFitMethod was not initialized before a fit was executed.");
	}
}

int PolFitMethod::SquareError(const gsl_vector * coefs, void *data, gsl_vector * f)
{
	// f(x,y) = ( a * x^2 + b * xy + c * y^2 + d x + e y + l  -  image[x,y] )^2
	ThreadLocal &local = *(ThreadLocal *) data;
	double a = gsl_vector_get(coefs, 0);
	double b = gsl_vector_get(coefs, 1);
	double c = gsl_vector_get(coefs, 2);
	double d = gsl_vector_get(coefs, 3);
	double e = gsl_vector_get(coefs, 4);
	double l = gsl_vector_get(coefs, 5);

	unsigned index = 0;

	for(unsigned y=local.startY; y <= local.endY; ++y) {
		double yf = y;
		for(unsigned x=local.startX; x <= local.endX; ++x) {
			if(!local.image->_mask->Value(x, y)) {
				double xf = x;
				double g_xy = a * xf*xf + b * xf*yf + c * yf*yf + d * xf + e * yf + l - local.image->_original->Value(x, y);
				gsl_vector_set(f, index, (g_xy*g_xy));
			} else {
				gsl_vector_set(f, index, 0.0);
			}
			index++;
		}
	}
	return GSL_SUCCESS;
}

int PolFitMethod::SquareErrorDiff(const gsl_vector *coefs, void *data, gsl_matrix * J)
{
	ThreadLocal &local = *(ThreadLocal *) data;

	double a = gsl_vector_get(coefs, 0);
	double b = gsl_vector_get(coefs, 1);
	double c = gsl_vector_get(coefs, 2);
	double d = gsl_vector_get(coefs, 3);
	double e = gsl_vector_get(coefs, 4);
	double f = gsl_vector_get(coefs, 5);

	unsigned index = 0;
	for(unsigned y=local.startY; y <= local.endY; ++y) {
		double yf = y;
		for(unsigned x=local.startX; x <= local.endX; ++x) {
			if(!local.image->_mask->Value(x, y)) {
				// f(x,y) = ( a * x^2 + b * xy + c * y^2 + d x + e y + f  -  image[x,y] )^2
				// f(x,y) = g^2(x,y)
				// df(x,y)/dz =  2 * g(x,y) * dg(x,y)/dz
				// We now need to calculate df(x,y)/da, df(x,y)/db, ..... for each x and y
				double xf = x;
				double g_xy = 2.0 * (a * xf*xf + b * xf*yf + c * yf*yf + d * xf + e * yf + f - local.image->_original->Value(x, y));
				gsl_matrix_set(J, index, 0, g_xy * xf*xf); // df/da
				gsl_matrix_set(J, index, 1, g_xy * xf*yf); // df/db
				gsl_matrix_set(J, index, 2, g_xy * yf*yf); // df/dc
				gsl_matrix_set(J, index, 3, g_xy * xf); // df/dd
				gsl_matrix_set(J, index, 4, g_xy * yf); // df/de
				gsl_matrix_set(J, index, 5, g_xy); // df/df
			} else {
				for(unsigned ci=0;ci<6;++ci)
					gsl_matrix_set(J, index, ci, 0.0);
			}
			index++;
		}
	}
	
	return GSL_SUCCESS;
}

int PolFitMethod::LinError(const gsl_vector * coefs, void *data, gsl_vector * f)
{
	// f(x,y) = | a * x^2 + b * xy + c * y^2 + d x + e y + l  -  image[x,y] |
	ThreadLocal &local = *(ThreadLocal *) data;
	double a = gsl_vector_get(coefs, 0);
	double b = gsl_vector_get(coefs, 1);
	double c = gsl_vector_get(coefs, 2);
	double d = gsl_vector_get(coefs, 3);
	double e = gsl_vector_get(coefs, 4);
	double l = gsl_vector_get(coefs, 5);

	unsigned index = 0;

	for(unsigned y=local.startY; y <= local.endY; ++y) {
		double yf = y;
		for(unsigned x=local.startX; x <= local.endX; ++x) {
			if(!local.image->_mask->Value(x, y)) {
				double xf = x;
				double g_xy = a * xf*xf + b * xf*yf + c * yf*yf + d * xf + e * yf + l - local.image->_original->Value(x, y);
				gsl_vector_set(f, index, fabs(g_xy));
			} else {
				gsl_vector_set(f, index, 0.0);
			}
			index++;
		}
	}
	return GSL_SUCCESS;
}

int PolFitMethod::LinErrorDiff(const gsl_vector *coefs, void *data, gsl_matrix * J)
{
	ThreadLocal &local = *(ThreadLocal *) data;

	double a = gsl_vector_get(coefs, 0);
	double b = gsl_vector_get(coefs, 1);
	double c = gsl_vector_get(coefs, 2);
	double d = gsl_vector_get(coefs, 3);
	double e = gsl_vector_get(coefs, 4);
	double f = gsl_vector_get(coefs, 5);

	unsigned index = 0;
	for(unsigned y=local.startY; y <= local.endY; ++y) {
		double yf = y;
		for(unsigned x=local.startX; x <= local.endX; ++x) {
			if(!local.image->_mask->Value(x, y)) {
				// f(x,y) = | a * x^2 + b * xy + c * y^2 + d x + e y + f  -  image[x,y] |
				// f(x,y) = | g(x,y) |
				// df(x,y)/dz =  dg(x,y)/dz * g(x,y) / | g(x,y) |
				// We now need to calculate df(x,y)/da, df(x,y)/db, ..... for each x and y
				double xf = x;
				double g_xy = a * xf*xf + b * xf*yf + c * yf*yf + d * xf + e * yf + f - local.image->_original->Value(x, y);
				double h_xy = g_xy / fabs(g_xy);
				gsl_matrix_set(J, index, 0, h_xy * xf*xf); // df/da
				gsl_matrix_set(J, index, 1, h_xy * xf*yf); // df/db
				gsl_matrix_set(J, index, 2, h_xy * yf*yf); // df/dc
				gsl_matrix_set(J, index, 3, h_xy * xf); // df/dd
				gsl_matrix_set(J, index, 4, h_xy * yf); // df/de
				gsl_matrix_set(J, index, 5, h_xy); // df/df
			} else {
				for(unsigned ci=0;ci<6;++ci)
					gsl_matrix_set(J, index, ci, 0.0);
			}
			index++;
		}
	}
	
	return GSL_SUCCESS;
}

long double PolFitMethod::FitBackground(unsigned x, unsigned y, ThreadLocal &local)
{
	long double *coefficients = new long double[6];

	boost::mutex::scoped_lock lock(_mutex);
	if(_previousCoefficients) {
		for(unsigned i=0;i<6;++i)
			coefficients[i] = _previousCoefficients[i];
	} else {
		for(unsigned i=0;i<6;++i)
			coefficients[i] = 1e-4 * (i*i*i);
	}
	lock.unlock();

	// Chose to use the Levenberg-Marquardt solver with scaling
	const gsl_multifit_fdfsolver_type * T = gsl_multifit_fdfsolver_lmsder;

	// Construct solver
	unsigned functionCount = (local.endY - local.startY + 1) * (local.endX - local.startX + 1);
	unsigned coefficientCount = 6;
	gsl_multifit_fdfsolver *solver = gsl_multifit_fdfsolver_alloc (T, functionCount, coefficientCount);
	if(solver == 0) throw BadUsageException("No solver.");

	// Initialize function information structure
	gsl_multifit_function_fdf functionInfo;
	switch(_method) {
		case LeastSquare:
		default:
			functionInfo.f = &SquareError;
			functionInfo.df = &SquareErrorDiff;
			functionInfo.fdf = &SquareErrorComb;
			break;
		case LeastAbs:
			functionInfo.f = &LinError;
			functionInfo.df = &LinErrorDiff;
			functionInfo.fdf = &LinErrorComb;
			break;
	}
	functionInfo.n = functionCount;
	functionInfo.p = coefficientCount;
	functionInfo.params = &local;

	// Initialize initial value of parameters
	//gsl_vector vec;
	double vec_init[coefficientCount];
	for(unsigned i = 0;i < coefficientCount;++i)
		vec_init[i] = coefficients[i];
	gsl_vector_view vec_view = gsl_vector_view_array (vec_init, coefficientCount);
	
	int status = gsl_multifit_fdfsolver_set (solver, &functionInfo, &vec_view.vector);
	if (status && status != GSL_CONTINUE) {
		std::cout << "Error: status = " << gsl_strerror (status) << std::endl;
	}

	// Start iterating
	int iter=0;
	do {
		iter++;
		status = gsl_multifit_fdfsolver_iterate(solver);
		//PrintState(iter, solver);

		if (status && status != GSL_CONTINUE) {
			//std::cout << "Error: status = " << gsl_strerror (status) << std::endl;
			break;
		}

		status = gsl_multifit_test_delta(solver->dx, solver->x, _precision, _precision);
	} while (status == GSL_CONTINUE && iter < 250);
	
	// Save coefficients
	for(unsigned i = 0;i<coefficientCount;++i)
		coefficients[i] = gsl_vector_get(solver->x, i);

	lock.lock();
	if(_previousCoefficients == 0)
		_previousCoefficients = new long double[6];
	for(unsigned i = 0;i<coefficientCount;++i)
		_previousCoefficients[i] = coefficients[i];
	lock.unlock();

	// Clean up
	gsl_multifit_fdfsolver_free(solver);

	long double evaluation = Evaluate(x, y, coefficients);
	delete[] coefficients;
	return evaluation;
}

long double PolFitMethod::Evaluate(unsigned x, unsigned y, long double *coefficients)
{
	// f(x,y) = a * x^2 + b * xy + c * y^2 + d x + e y + f
	double xf = x, yf = y;
	return coefficients[0]*xf*xf + coefficients[1]*xf*yf + coefficients[2]*yf*yf + coefficients[3]*xf +
		coefficients[4]*yf + coefficients[5];
}
