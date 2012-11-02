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
#ifndef CLEANER_H
#define CLEANER_H

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Cleaner{
	public:
		Cleaner();
		~Cleaner();
		void Init(const class Image2D &real, const Image2D &imaginary, const Image2D &weights, long double gain);
		const Image2D &RealResidual() const throw() { return *_residualReal; }
		const Image2D &ImaginaryResidual() const throw() { return *_residualImaginary; }
		const Image2D &RealDeconvolved() const throw() { return *_deconvolvedReal; }
		const Image2D &ImaginaryDeconvolved() const throw() { return *_deconvolvedImaginary; }
		const Image2D &RealPSF() const throw() { return *_psfReal; }
		const Image2D &ImaginaryPSF() const throw() { return *_psfImaginary; }
		const Image2D &RealConvolvedResidual() const throw() { return *_convolvedReal; }
		const Image2D &ImaginaryConvolvedResidual() const throw() { return *_convolvedImaginary; }
		void Iteration();
	private:
		static void FindPeak(const Image2D &real, const Image2D &imaginary, unsigned &x, unsigned &y);
		void Subtract(Image2D &original, const Image2D &rightHand, unsigned x, unsigned y, long double gain);
		long double MaxAllowedGain(const Image2D &image, const Image2D &subtract, unsigned x, unsigned y);

		const Image2D *_real, *_imaginary, *_weightsReal, *_weightsImaginary;
		Image2D *_residualReal, *_residualImaginary, *_psfReal, *_psfImaginary;
		Image2D *_deconvolvedReal, *_deconvolvedImaginary;
		Image2D *_convolvedReal, *_convolvedImaginary; 
		long double _gain, _psfPeak;
		long double _psfSum;
};

#endif
