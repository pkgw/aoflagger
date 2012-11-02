
#include "eigenvalue.h"

#include <stdexcept>

// Needs to be included LAST
#include "../../f2c.h"
#include "../../util/aologger.h"

extern "C" {
	void zheevx_(char *jobz, char *range, char *uplo, integer *n,
							doublecomplex *a, integer *lda, double *vl, double *vu, integer *il,
							integer *iu, double *abtol, integer *nfound, double  *w,
							doublecomplex *z, integer *ldz, doublecomplex *work,
							integer *lwork, doublereal *rwork, integer *iwork, integer *ifail, integer *info);

	void zheev_(char *jobz, char *uplo, integer *n,
							doublecomplex *a, integer *lda, double  *w,
							doublecomplex *work,
							integer *lwork, doublereal *rwork, integer *info);
}

double Eigenvalue::Compute(Image2DCPtr real, Image2DCPtr imaginary)
{
	if(real->Width() != imaginary->Width() || real->Height() != imaginary->Height())
		throw std::runtime_error("Size of real and imaginary don't match in eigen value decomposition");
	if(real->Width() != real->Height())
		throw std::runtime_error("Not a square image given in eigen value decomposition");
	
	char jobz[] = "N";  // compute eigenvalues only
	char range[] = "I"; // the IL-th through IU-th eigenvalues will be found.
	char uplo[] = "U";  // Upper triangle of A is stored
	long int n = real->Width();
	long int lda = n;
	double vl = 0, vu = 0;
	//long int il = 1, iu = 1; // search for first eigenvalue
	long int il = n, iu = n; // search for nth eigenvalue
	double abtol = 0.0;
	long int nfound = 0;
	double w[n];
	doublecomplex z; // for eigenvectors, not used
	long int ldz = 1; // for eigenvectors, not used
	long int ifail = 0;
	long int info = 0;
	
	doublecomplex *a = new doublecomplex[n * n];
	for(int y=0;y<n;++y) {
		for(int x=0;x<n; ++x) {
			a[y + x*n].r = real->Value(x, y);
			a[y + x*n].i = imaginary->Value(x, y);
		}
	}
	
	doublecomplex complexWorkAreaSize;
	long int workAreaSize = -1;
	doublereal *rwork = new doublereal[7*n];
	integer *iwork = new integer[5*n];

	// Determine optimal workareasize
	zheevx_(jobz, range, uplo, &n, a, &lda, &vl, &vu, &il, &iu, &abtol, &nfound, w, &z, &ldz, &complexWorkAreaSize, &workAreaSize, rwork, iwork, &ifail, &info);
	
	if(info != 0)
	{
		delete[] a;
		delete[] rwork;
		delete[] iwork;
		throw std::runtime_error("Can not determine workareasize, zheevx returned an error.");
	}
	
	workAreaSize = (int) complexWorkAreaSize.r;
	doublecomplex *work = new doublecomplex[workAreaSize];
	zheevx_(jobz, range, uplo, &n, a, &lda, &vl, &vu, &il, &iu, &abtol, &nfound, w, &z, &ldz, work, &workAreaSize, rwork, iwork, &ifail, &info);
	
	delete[] work;
	delete[] a;
	delete[] rwork;
	
	if(info != 0)
		throw std::runtime_error("zheevx failed");
	
	return w[0];
}

void Eigenvalue::Remove(Image2DPtr real, Image2DPtr imaginary, bool debug)
{
	if(real->Width() != imaginary->Width() || real->Height() != imaginary->Height())
		throw std::runtime_error("Size of real and imaginary don't match in eigen value decomposition");
	if(real->Width() != real->Height())
		throw std::runtime_error("Not a square image given in eigen value decomposition");
	
	char jobz[] = "V";  // compute eigenvalues and eigenvectors
	char uplo[] = "U";  // Upper triangle of A is stored
	long int n = real->Width();
	long int lda = n;
	double w[n];
	long int info = 0;
	
	doublecomplex *a = new doublecomplex[n * n];
	for(int y=0;y<n;++y) {
		for(int x=0;x<n; ++x) {
			a[y + x*n].r = real->Value(x, y);
			a[y + x*n].i = imaginary->Value(x, y);
		}
	}
	
	doublecomplex complexWorkAreaSize;
	long int workAreaSize = -1;
	doublereal *rwork = new doublereal[7*n];

	// Determine optimal workareasize
	zheev_(jobz, uplo, &n, a, &lda, w, &complexWorkAreaSize, &workAreaSize, rwork, &info);
	
	if(info != 0)
	{
		delete[] a;
		delete[] rwork;
		throw std::runtime_error("Can not determine workareasize, zheev returned an error.");
	}
	
	workAreaSize = (int) complexWorkAreaSize.r;
	doublecomplex *work = new doublecomplex[workAreaSize];
	zheev_(jobz, uplo, &n, a, &lda, w, work, &workAreaSize, rwork, &info);
	
	delete[] work;
	delete[] rwork;
	
	if(info != 0)
		throw std::runtime_error("zheev failed");
		
	if(debug) 
	{
		AOLogger::Debug << "Eigenvalues: ";
		for(unsigned i=0;i<n;++i)
			AOLogger::Debug << w[i] << ' ';
		AOLogger::Debug << '\n';
	}
	for(int y=0;y<n;++y)
	{
		for(int x=0;x<n;++x)
		{
			double a_xy_r = 0.0;
			double a_xy_i = 0.0;
			// A = U S U^T , so:
			// a_xy = \sum_{i=0}^{n} U_{iy} S_{ii} U_{ix}
			// The eigenvalues are sorted from small (or negative) to large (positive)
			for(int i=n-1;i<n;++i) {
				double u_r = a[y + i*n].r;
				double u_i = a[y + i*n].i;
				double s = w[i];
				double ut_r = a[x + i*n].r;
				double ut_i = a[x + i*n].i;
				a_xy_r += s * (u_r * ut_r - u_i * ut_i);
				a_xy_i += s * (u_r * ut_i + u_i * ut_r);
			}
			real->SetValue(x, y, /*real->Value(x, y) - */ a_xy_r);
			imaginary->SetValue(x, y, /*imaginary->Value(x, y) -*/ a_xy_i);
		}
	}
	
	delete[] a;
}
