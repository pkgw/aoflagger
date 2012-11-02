#ifndef RFI_EIGENVALUE_H
#define RFI_EIGENVALUE_H

#include "../../msio/image2d.h"

class Eigenvalue
{
	public:
		static double Compute(Image2DCPtr real, Image2DCPtr imaginary);
		static void Remove(Image2DPtr real, Image2DPtr imaginary, bool debug=false);
};

#endif // RFI_EIGENVALUE_H
