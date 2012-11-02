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
#include "../../util/stopwatch.h"

#include "svdmitigater.h"

#ifdef HAVE_GTKMM
 #include "../../gui/plot/plot2d.h"
#endif

extern "C" {
  int zgesvd_(char *jobu, char *jobvt, integer *m, integer *n, 
	      doublecomplex *a, integer *lda, doublereal *s, doublecomplex *u, 
	      integer *ldu, doublecomplex *vt, integer *ldvt, doublecomplex *work, 
	      integer *lwork, doublereal *rwork, integer *info);
}

SVDMitigater::SVDMitigater() : _background(0), _singularValues(0), _leftSingularVectors(0), _rightSingularVectors(0), _iteration(0), _removeCount(10),  _verbose(false)
{
}


SVDMitigater::~SVDMitigater()
{
	Clear();
}

void SVDMitigater::Clear()
{
	if(IsDecomposed()) {
		delete[] _singularValues;
		delete[] _leftSingularVectors;
		delete[] _rightSingularVectors;
		_singularValues = 0;
		_leftSingularVectors = 0;
		_rightSingularVectors = 0;
	}
	if(_background != 0)
		delete _background;
}

// lda = leading dimension

/*int cgebrd_(integer *m, integer *n, complex *a, integer *lda,
         real *d__, real *e, complex *tauq, complex *taup, complex *work,
        integer *lwork, integer *info);*/

void SVDMitigater::Decompose()
{
	if(_verbose)
		std::cout << "Decomposing..." << std::endl;
	Stopwatch watch;
	watch.Start();
	Clear();

	// Remember that the axes have to be turned; in 'a', time is along the vertical axis.
	_m = _data.ImageHeight();
	_n = _data.ImageWidth();
	int minmn = _m<_n ? _m : _n;
	char rowsOfU = 'A'; // all rows of u
	char rowsOfVT = 'A'; // all rows of VT
	doublecomplex *a = new doublecomplex[_m * _n];
	Image2DCPtr
		real = _data.GetRealPart(),
		imaginary = _data.GetImaginaryPart();
	for(int t=0;t<_n;++t) {
		for(int f=0;f<_m; ++f) {
			a[t*_m + f].r = real->Value(t, f);
			a[t*_m + f].i = imaginary->Value(t, f);
		}
	}
	long int lda = _m;
	_singularValues = new double[minmn];
	for(int i=0;i<minmn;++i)
		_singularValues[i] = 0.0;
	_leftSingularVectors = new doublecomplex[_m*_m];
	for(int i=0;i<_m*_m;++i) {
		_leftSingularVectors[i].r = 0.0;
		_leftSingularVectors[i].i = 0.0;
	}
	_rightSingularVectors = new doublecomplex[_n*_n];
	for(int i=0;i<_n*_n;++i) {
		_rightSingularVectors[i].r = 0.0;
		_rightSingularVectors[i].i = 0.0;
	}
	long int info = 0;
	doublecomplex complexWorkAreaSize;
	long int workAreaSize = -1;
	double *workArea2 = new double[5 * minmn];

	// Determine optimal workareasize
	zgesvd_(&rowsOfU, &rowsOfVT, &_m, &_n, a, &lda, _singularValues, _leftSingularVectors, &_m, _rightSingularVectors, &_n, &complexWorkAreaSize, &workAreaSize, workArea2, &info);

	if(info == 0)
	{
		if(_verbose)
			std::cout << "zgesvd_..." << std::endl;
		workAreaSize = (int) complexWorkAreaSize.r;
		doublecomplex *workArea1 = new doublecomplex[workAreaSize];
		zgesvd_(&rowsOfU, &rowsOfVT, &_m, &_n, a, &lda, _singularValues, _leftSingularVectors, &_m, _rightSingularVectors, &_n, workArea1, &workAreaSize, workArea2, &info);

		delete[] workArea1;
	}
	delete[] workArea2;
	delete[] a;

	if(_verbose) {
		for(int i=0;i<minmn;++i)
			std::cout << _singularValues[i] << ",";
		std::cout << std::endl;
		std::cout << watch.ToString() << std::endl;
	}
}

void SVDMitigater::Compose()
{
	if(_verbose)
		std::cout << "Composing..." << std::endl;
	Stopwatch watch;
	watch.Start();
	Image2DPtr real = Image2D::CreateUnsetImagePtr(_data.ImageWidth(), _data.ImageHeight());
	Image2DPtr imaginary = Image2D::CreateUnsetImagePtr(_data.ImageWidth(), _data.ImageHeight());
	int minmn = _m<_n ? _m : _n;
	for(int t=0;t<_n;++t) {
		for(int f=0;f<_m; ++f) {
			double a_tf_r = 0.0;
			double a_tf_i = 0.0;
			// A = U S V^T , so:
			// a_tf = \sum_{g=0}^{minmn} U_{gf} S_{gg} V^T_{tg}
			// Note that _rightSingularVectors=V^T, thus is already stored rowwise
			for(int g=0;g<minmn;++g) {
				double u_r = _leftSingularVectors[g*_m + f].r;
				double u_i = _leftSingularVectors[g*_m + f].i;
				double s = _singularValues[g];
				double v_r = _rightSingularVectors[t*_n + g].r;
				double v_i = _rightSingularVectors[t*_n + g].i;
				a_tf_r += s * (u_r * v_r - u_i * v_i);
				a_tf_i += s * (u_r * v_i + u_i * v_r);
			}
			real->SetValue(t, f, a_tf_r);
			imaginary->SetValue(t, f, a_tf_i);
		}
	}
	if(_background != 0)
		delete _background;
	_background = new TimeFrequencyData(SinglePolarisation, real, imaginary);
	if(_verbose)
		std::cout << watch.ToString() << std::endl;
}


#ifdef HAVE_GTKMM

void SVDMitigater::CreateSingularValueGraph(const TimeFrequencyData &data, Plot2D &plot)
{
	size_t polarisationCount = data.PolarisationCount();
	plot.SetTitle("Distribution of singular values");
	plot.SetLogarithmicYAxis(true);
	for(size_t i=0;i<polarisationCount;++i)
	{
		TimeFrequencyData *polarizationData = data.CreateTFDataFromPolarisationIndex(i);
		SVDMitigater svd;
		svd.Initialize(*polarizationData);
		svd.Decompose();
		size_t minmn = svd._m<svd._n ? svd._m : svd._n;
		
		Plot2DPointSet &pointSet = plot.StartLine(polarizationData->Description());
		pointSet.SetXDesc("Singular value index");
		pointSet.SetYDesc("Singular value");
		
		for(size_t i=0;i<minmn;++i)
			plot.PushDataPoint(i, svd.SingularValue(i));
	}
}

#else
void SVDMitigater::CreateSingularValueGraph(const TimeFrequencyData &, Plot2D &)
{}
#endif
