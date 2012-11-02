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
#ifndef PolFitMethod_H
#define PolFitMethod_H

// This file is ORPHAN: it is not used or referenced, because it implements
// a method which is not used, and I wanted to remove the GSL dependence.

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlin.h>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/strategy/algorithms/surfacefitmethod.h>

class PolFitMethod : public SurfaceFitMethod {
	public:
		enum Method { None, LeastSquare, LeastAbs };
		PolFitMethod();
		~PolFitMethod();
		virtual void Initialize(const TimeFrequencyData &input);
		virtual unsigned TaskCount();
		virtual void PerformFit(unsigned taskNumber);
		virtual TimeFrequencyData Background() { return *_background; }
		virtual enum TimeFrequencyData::PhaseRepresentation PhaseRepresentation() const
		{
			return TimeFrequencyData::AmplitudePart;
		}
	private:
		long double Evaluate(unsigned x, unsigned y, long double *coefficients);
		static int SquareError(const gsl_vector * coefs, void *data, gsl_vector * f);
		static int SquareErrorDiff(const gsl_vector * x, void *data, gsl_matrix * J);
		static int SquareErrorComb(const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)
		{
			SquareError(x, data, f);
			SquareErrorDiff(x, data, J);
			return GSL_SUCCESS;
		}
		static int LinError(const gsl_vector * coefs, void *data, gsl_vector * f);
		static int LinErrorDiff(const gsl_vector * x, void *data, gsl_matrix * J);
		static int LinErrorComb(const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)
		{
			LinError(x, data, f);
			LinErrorDiff(x, data, J);
			return GSL_SUCCESS;
		}
		long double CalculateBackgroundValue(unsigned x, unsigned y);
		long double FitBackground(unsigned x, unsigned y, ThreadLocal &local);

		Image2DCPtr _original;
		class TimeFrequencyData *_background;
		Image2DPtr _background2D;
		Mask2DCPtr _mask;
		unsigned _hSquareSize, _vSquareSize;
		long double _precision;
		long double *_previousCoefficients;
		long double _hKernelSize, _vKernelSize;
		enum Method _method;
};

#endif
