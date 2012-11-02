#include "rayleighfitter.h"

#include <iostream>

#ifdef HAVE_GSL

#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

static int fit_f(const gsl_vector *xvec, void *data, gsl_vector *f)
{
	const double sigma = gsl_vector_get(xvec, 0);
	const double n = gsl_vector_get(xvec, 1);
	
	size_t t = 0;
	RayleighFitter &fitter = *(RayleighFitter *) data;
	const LogHistogram &hist = *fitter._hist;
	const double minVal = fitter._minVal;
	const double maxVal = fitter._maxVal;
  
	for (LogHistogram::const_iterator i=hist.begin(); i!=hist.end(); ++i)
	{
		const double x = i.value();
		if(x >= minVal && x < maxVal && std::isfinite(x))
		{
			const double val = i.normalizedCount();
			//const double logval = log(val);
			//const double weight = logval;
			
			double sigmaP2 = sigma*sigma;
			double Yi = x * exp(-(x*x)/(2*sigmaP2)) * n / sigmaP2;
			if(fitter.FitLogarithmic())
				gsl_vector_set(f, t, log(Yi) - log(val));
			else
				gsl_vector_set(f, t, (Yi - val));
			++t;
		}
	}
	
	return GSL_SUCCESS;
}

int fit_df(const gsl_vector *xvec, void *data, gsl_matrix *J)
{
	const double sigma = gsl_vector_get(xvec, 0);
	const double n = gsl_vector_get(xvec, 1);
	
	size_t t = 0;
	RayleighFitter &fitter = *(RayleighFitter *) data;
	const LogHistogram &hist = *fitter._hist;
	const double minVal = fitter._minVal;
	const double maxVal = fitter._maxVal;
	const double sigmaP2 = sigma*sigma;
  const double sigmaP3 = sigma*sigma*sigma;
	for (LogHistogram::const_iterator i=hist.begin(); i!=hist.end(); ++i)
	{
		const double x = i.value();
		if(x >= minVal && x < maxVal && std::isfinite(x))
		{
			//const double val = i.normalizedCount();
			//const double weight = log(val);

			double dfdsigma, dfdn;
			if(fitter.FitLogarithmic())
			{
				dfdsigma = (x*x-2.0*sigmaP2)/sigmaP3;
				dfdn = 1.0/n;
			} else {
				dfdsigma = -n * 2.0*x*x*x / (sigmaP3 * sigmaP3) * exp(-x*x/(2.0*sigmaP2));
				dfdn = x * exp(-(x*x)/(2*sigmaP2)) / sigmaP2;
			}
			
			// diff to sigma
			gsl_matrix_set (J, t, 0, dfdsigma); 
			// diff to n
			gsl_matrix_set (J, t, 1, dfdn);
			
			++t;
		}
	}	
	return GSL_SUCCESS;
}
     
int fit_fdf(const gsl_vector *x, void *data,
						 gsl_vector *f, gsl_matrix *J)
{
	fit_f (x, data, f);
	fit_df (x, data, J);
  
	return GSL_SUCCESS;
}

void print_state(size_t iter, gsl_multifit_fdfsolver *s)
{
  double sigma = gsl_vector_get (s->x, 0);
  double N = gsl_vector_get (s->x, 1);
	std::cout << "iteration " << iter << ", sigma=" << sigma << ", N=" << N << "\n";
}

void RayleighFitter::Fit(double minVal, double maxVal, const LogHistogram &hist, double &sigma, double &n)
{
	unsigned int iter = 0;
	const size_t nVars = 2;
	_hist = &hist;
	if(minVal > 0)
		_minVal = minVal;
	else
		_minVal = hist.MinPositiveAmplitude();
	_maxVal = maxVal;

	size_t nData = 0;
	for (LogHistogram::iterator i=hist.begin(); i!=hist.end(); ++i)
	{
		const double val = i.value();
		if(val >= minVal && val < maxVal && std::isfinite(val))
			++nData;
	}
	std::cout << "ndata=" << nData << "\n";
    
	double x_init[nVars] = { sigma, n };
	gsl_vector_view x = gsl_vector_view_array (x_init, nVars);
  
	gsl_multifit_function_fdf f;
	f.f = &fit_f;
	f.df = &fit_df;
	f.fdf = &fit_fdf;
	f.n = nData;
	f.p = nVars;
	f.params = this;
  
	const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
	gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc (T, nData, nVars);
	gsl_multifit_fdfsolver_set (s, &f, &x.vector);
  
	print_state (iter, s);
  
	int status;
	do {
		iter++;
		status = gsl_multifit_fdfsolver_iterate (s);
		
		std::cout << "status = " << gsl_strerror(status) << "\n";
		
		print_state (iter, s);
			
		if (status)
			break;
		
		status = gsl_multifit_test_delta (s->dx, s->x, 1e-7, 1e-3);
		
  } while (status == GSL_CONTINUE && iter < 500);
	
  std::cout << "status = " << gsl_strerror (status) << "\n";
  print_state(iter, s);
  sigma = fabs(gsl_vector_get (s->x, 0));
  n = fabs(gsl_vector_get (s->x, 1));
  gsl_multifit_fdfsolver_free (s);
}

#else // No gsl...

void RayleighFitter::Fit(double minVal, double maxVal, const LogHistogram &hist, double &sigma, double &n)
{
	sigma = 1.0;
	n = 1.0;
}

#endif

double RayleighFitter::SigmaEstimate(const LogHistogram &hist)
{
	return hist.AmplitudeWithMaxNormalizedCount();
}

void RayleighFitter::FindFitRangeUnderRFIContamination(double minPositiveAmplitude, double sigmaEstimate, double &minValue, double &maxValue)
{
	minValue = minPositiveAmplitude;
	maxValue = sigmaEstimate * 1.5;
	std::cout << "Found range " << minValue << " -- " << maxValue << "\n";
}

double RayleighFitter::ErrorOfFit(const LogHistogram &histogram, double rangeStart, double rangeEnd, double sigma, double n)
{
	double sum = 0.0;
	size_t count = 0;
	for (LogHistogram::const_iterator i=histogram.begin(); i!=histogram.end(); ++i)
	{
		const double x = i.value();
		if(x >= rangeStart && x < rangeEnd && std::isfinite(x))
		{
			const double val = i.normalizedCount();
			
			double sigmaP2 = sigma*sigma;
			double Yi = x * exp(-(x*x)/(2*sigmaP2)) * n / sigmaP2;
			
			double error = (Yi - val)*(Yi - val);
			sum += error;
			++count;
		}
	}
	return sum / (double) count;
}

double RayleighFitter::NEstimate(const LogHistogram &hist, double rangeStart, double rangeEnd)
{
	double rangeSum = 0.0;
	size_t count = 0;
	for (LogHistogram::const_iterator i=hist.begin(); i!=hist.end(); ++i)
	{
		if(i.value() > rangeStart && i.value() < rangeEnd && std::isfinite(i.value()))
		{
			if(std::isfinite(i.normalizedCount()))
			{
				rangeSum += i.normalizedCount();
				++count;
			}
		}
	}
	return rangeSum / (count * 10.0);
}
