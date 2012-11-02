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
#include "stokesimager.h"

StokesImager::StokesImager() : _stokesI(0), _stokesQ(0), _stokesU(0), _stokesV(0)
{
}

StokesImager::~StokesImager()
{
	if(_stokesI) {
		delete _stokesI;
		delete _stokesQ;
		delete _stokesU;
		delete _stokesV;
	}
}

void StokesImager::Image(const Image2D &realXX, const Image2D &imaginaryXX, const Image2D &realXY, const Image2D &imaginaryXY, const Image2D &realYY, const Image2D &imaginaryYY)
{
	if(_stokesI) {
		delete _stokesI;
		delete _stokesQ;
		delete _stokesU;
		delete _stokesV;
	}
	_stokesI = Image2D::CreateUnsetImage(realXX.Width(), realXX.Height());
	_stokesQ = Image2D::CreateUnsetImage(realXX.Width(), realXX.Height());
	_stokesU = Image2D::CreateUnsetImage(realXX.Width(), realXX.Height());
	_stokesV = Image2D::CreateUnsetImage(realXX.Width(), realXX.Height());

	for(unsigned long y = 0; y < realXX.Height(); ++y) {
		for(unsigned long x = 0; x < realXX.Width(); ++x) {
			double xx_a = sqrt(realXX.Value(x, y)*realXX.Value(x, y) + imaginaryXX.Value(x, y)*imaginaryXX.Value(x, y));
			double yy_a = sqrt(realYY.Value(x, y)*realYY.Value(x, y) + imaginaryYY.Value(x, y)*imaginaryYY.Value(x, y));
		
			double stokesI = xx_a + yy_a;
			double stokesQ = xx_a - yy_a;
			double stokesU = 2.0 * realXY.Value(x, y);
			double stokesV = 2.0 * imaginaryXY.Value(x, y);
			_stokesI->SetValue(x, y, stokesI);
			_stokesQ->SetValue(x, y, stokesQ);
			_stokesU->SetValue(x, y, stokesU);
			_stokesV->SetValue(x, y, stokesV);
		}
	}
} 

Image2DPtr StokesImager::CreateStokesIAmplitude(Image2DCPtr realXX, Image2DCPtr imaginaryXX, Image2DCPtr realYY, Image2DCPtr imaginaryYY)
{
	Image2D *stokesI = Image2D::CreateUnsetImage(realXX->Width(), realXX->Height());

	for(unsigned long y = 0; y < realXX->Height(); ++y) {
		for(unsigned long x = 0; x < realXX->Width(); ++x) {
			double xx_a = sqrt(realXX->Value(x, y)*realXX->Value(x, y) + imaginaryXX->Value(x, y)*imaginaryXX->Value(x, y));
			double yy_a = sqrt(realYY->Value(x, y)*realYY->Value(x, y) + imaginaryYY->Value(x, y)*imaginaryYY->Value(x, y));
		
			stokesI->SetValue(x, y, xx_a + yy_a);
		}
	}
	return Image2DPtr(stokesI);
}

Image2DPtr StokesImager::CreateSum(Image2DCPtr left, Image2DCPtr right)
{
	Image2D *sum = Image2D::CreateUnsetImage(left->Width(), right->Height());

	for(unsigned long y = 0; y < left->Height(); ++y) {
		for(unsigned long x = 0; x < right->Width(); ++x) {
			num_t left_a = left->Value(x, y);
			num_t right_a = right->Value(x, y);
		
			sum->SetValue(x, y, left_a + right_a);
		}
	}
	return Image2DPtr(sum);
}

Image2DPtr StokesImager::CreateNegatedSum(Image2DCPtr left, Image2DCPtr right)
{
	Image2D *sum = Image2D::CreateUnsetImage(left->Width(), right->Height());

	for(unsigned long y = 0; y < left->Height(); ++y) {
		for(unsigned long x = 0; x < right->Width(); ++x) {
			num_t left_a = left->Value(x, y);
			num_t right_a = right->Value(x, y);
		
			sum->SetValue(x, y, -(left_a + right_a));
		}
	}
	return Image2DPtr(sum);
}

Image2DPtr StokesImager::CreateDifference(Image2DCPtr left, Image2DCPtr right)
{
	Image2D *difference = Image2D::CreateUnsetImage(left->Width(), right->Height());

	for(unsigned long y = 0; y < left->Height(); ++y) {
		for(unsigned long x = 0; x < right->Width(); ++x) {
			num_t left_a = left->Value(x, y);
			num_t right_a = right->Value(x, y);
		
			difference->SetValue(x, y, left_a - right_a);
		}
	}
	return Image2DPtr(difference);
}

Image2DPtr StokesImager::CreateAvgPhase(Image2DCPtr xx, Image2DCPtr yy)
{
	Image2D *avgPhase = Image2D::CreateUnsetImage(xx->Width(), xx->Height());

	for(unsigned long y = 0; y < xx->Height(); ++y) {
		for(unsigned long x = 0; x < xx->Width(); ++x) {
			double xx_a = xx->Value(x, y);
			double yy_a = yy->Value(x, y);
		
			avgPhase->SetValue(x, y, fmodn(xx_a + yy_a, 2.0L*M_PIn) );
		}
	}
	return Image2DPtr(avgPhase);
}

