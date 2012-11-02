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
#include "localfitmethod.h"

#include "../../msio/image2d.h"
#include "../../msio/pngfile.h"

#include "../../util/rng.h"

#include "thresholdtools.h"

#include <cmath>
#include <iostream>

LocalFitMethod::LocalFitMethod() : _background(0), _weights(0)
{
}

LocalFitMethod::~LocalFitMethod()
{
	if(_background!=0)
		delete _background;
	ClearWeights();
}

void LocalFitMethod::Initialize(const TimeFrequencyData &input)
{
	ClearWeights();
	_original = input.GetSingleImage();
	_background2D = Image2D::CreateZeroImagePtr(_original->Width(), _original->Height());
	if(_background!=0)
		delete _background;
	_background = new TimeFrequencyData(input.PhaseRepresentation(), input.Polarisation(), _background2D);
	_mask = input.GetSingleMask();
	if(_hSquareSize * 2 > _original->Width())
		_hSquareSize = _original->Width()/2;
	if(_vSquareSize * 2 > _original->Height())
		_vSquareSize = _original->Height()/2;
	switch(_method) {
		case None:
		case Average:
		case Median:
		case Minimum:
			break;
		case GaussianWeightedAverage:
		case FastGaussianWeightedAverage:
			InitializeGaussianWeights();
			break;
	}
}

void LocalFitMethod::ClearWeights()
{
	if(_weights) {
		for(unsigned y = 0 ; y < _vSquareSize*2+1 ; ++y)
			delete[] _weights[y];
		delete[] _weights;
		_weights = 0;
	}
}

void LocalFitMethod::InitializeGaussianWeights()
{
	_weights = new num_t*[_vSquareSize*2+1];
	for(int y = 0 ; y < (int) (_vSquareSize*2+1) ; ++y) {
		_weights[y] = new num_t[_hSquareSize*2+1];
		for(int x = 0 ; x < (int) (_hSquareSize*2+1) ; ++x) {
			_weights[y][x] = RNG::EvaluateGaussian2D(x-(int)_hSquareSize, y-(int)_vSquareSize, _hKernelSize, _vKernelSize);
		}
	}
}

unsigned LocalFitMethod::TaskCount()
{
	if(_method == FastGaussianWeightedAverage)
		return 1;
	else
		return _original->Height();
}

void LocalFitMethod::PerformFit(unsigned taskNumber)
{
	if(_mask == 0)
		throw BadUsageException("Mask has not been set!");
	if(_method == FastGaussianWeightedAverage) {
		CalculateWeightedAverageFast();
	} else {
		unsigned y = taskNumber;
		for(unsigned x=0;x<_original->Width();++x)
			_background2D->SetValue(x, y, CalculateBackgroundValue(x, y));
	}
}

long double LocalFitMethod::CalculateBackgroundValue(unsigned x, unsigned y)
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
		case FastGaussianWeightedAverage:
			return 0.0;
		case Median:
			return CalculateMedian(x, y, local);
		case Minimum:
			return CalculateMinimum(x, y, local);
		case Average:
			return CalculateAverage(x, y, local);
		case GaussianWeightedAverage:
			return CalculateWeightedAverage(x, y, local);
		default:
			throw ConfigurationException("The LocalFitMethod was not initialized before a fit was executed.");
	}
}

long double LocalFitMethod::CalculateAverage(unsigned x, unsigned y, ThreadLocal &local)
{
	long double sum = 0.0;
	unsigned long count = 0;
	for(unsigned yi=local.startY;yi<=local.endY;++yi) {
		for(unsigned xi=local.startX;xi<=local.endX;++xi) {
			if(!_mask->Value(xi, yi) && std::isfinite(_original->Value(xi, yi))) {
				sum += _original->Value(xi, yi);
				++count;
			}
		}
	}
	if(count != 0)
		return sum / (long double) count;
	else
		return _original->Value(x, y);
}

long double LocalFitMethod::CalculateMedian(unsigned x, unsigned y, ThreadLocal &local)
{
	//unsigned maxSize = (local.endY-local.startY)*(local.endX-local.startX);
	std::vector<long double> orderList;
	unsigned long count = 0;
	for(unsigned yi=local.startY;yi<=local.endY;++yi) {
		for(unsigned xi=local.startX;xi<=local.endX;++xi) {
			if(!_mask->Value(xi, yi) && std::isfinite(_original->Value(xi, yi))) {
				orderList.push_back(_original->Value(xi, yi));
				++count;
			}
		}
	}
	if(count==0)
	{
		local.emptyWindows++;
		return _original->Value(x, y);
	}
	else if(count%2==1) {
	 	std::nth_element(orderList.begin(), orderList.begin()+count/2, orderList.end());
		long double mOdd = orderList[count/2];
		return mOdd;
	} else {
	 	std::nth_element(orderList.begin(), orderList.begin()+count/2, orderList.end());
		long double mOdd = orderList[count/2];
	 	std::nth_element(orderList.begin(), orderList.begin()+(count/2-1), orderList.end());
		long double mEven = orderList[count/2-1];
		return (mOdd + mEven)*0.5L;
	}
}

long double LocalFitMethod::CalculateMinimum(unsigned x, unsigned y, ThreadLocal &local)
{
	long double minimum = 1e100;
	unsigned long count = 0;
	for(unsigned yi=local.startY;yi<=local.endY;++yi) {
		for(unsigned xi=local.startX;xi<=local.endX;++xi) {
			if(!_mask->Value(xi, yi) && std::isfinite(_original->Value(xi, yi)) && _original->Value(xi, yi) < minimum) {
				minimum = _original->Value(xi, yi);
				++count;
			}
		}
	}
	if(count==0)
		return _original->Value(x, y);
	else
		return minimum;
}

long double LocalFitMethod::CalculateWeightedAverage(unsigned x, unsigned y, ThreadLocal &local)
{
	long double sum = 0.0;
	long double totalWeight = 0.0;
	for(unsigned j=local.startY;j<=local.endY;++j) {
		for(unsigned i=local.startX;i<=local.endX;++i) {
			if(!_mask->Value(i, j) && std::isfinite(_original->Value(i, j))) {
				long double weight = _weights[j - y + _vSquareSize][i - x + _hSquareSize];
				sum += _original->Value(i, j) * weight;
				totalWeight += weight;
			}
		}
	}
	if(totalWeight != 0.0)
		return sum / totalWeight;
	else {
		sum = 0.0;
		totalWeight = 0.0;
		for(unsigned j=local.startY;j<=local.endY;++j) {
			for(unsigned i=local.startX;i<=local.endX;++i) {
				if(std::isfinite(_original->Value(i, j))) {
					long double weight = _weights[j - y + _vSquareSize][i - x + _hSquareSize];
					sum += _original->Value(i, j) * weight;
					totalWeight += weight;
				}
			}
		}
		if(totalWeight != 0.0)
			return sum / totalWeight;
		else {
			sum = 0.0;
			totalWeight = 0.0;
			for(unsigned j=local.startY;j<=local.endY;++j) {
				for(unsigned i=local.startX;i<=local.endX;++i) {
					long double weight = _weights[j - y + _vSquareSize][i - x + _hSquareSize];
					sum += _original->Value(i, j) * weight;
					totalWeight += weight;
				}
			}
			return sum / totalWeight;
		}
	} 
}

void LocalFitMethod::CalculateWeightedAverageFast()
{
	_background2D->SetValues(*_original);
	ThresholdTools::SetFlaggedValuesToZero(_background2D, _mask);
	PerformGaussianConvolution(_background2D);
	Image2DPtr flagWeights = CreateFlagWeightsMatrix();
	PerformGaussianConvolution(flagWeights);
	ElementWiseDivide(_background2D, flagWeights);
}

void LocalFitMethod::ElementWiseDivide(Image2DPtr leftHand, Image2DCPtr rightHand)
{
	for(unsigned y=0;y<leftHand->Height();++y) {
		for(unsigned x=0;x<leftHand->Width();++x) {
			if(rightHand->Value(x, y) == 0.0)
				leftHand->SetValue(x, y, 0.0);
			else
				leftHand->SetValue(x, y, leftHand->Value(x, y) / rightHand->Value(x, y));
		}
	}
}

Image2DPtr LocalFitMethod::CreateFlagWeightsMatrix()
{
	Image2DPtr image = Image2D::CreateUnsetImagePtr(_mask->Width(), _mask->Height());
	for(unsigned y=0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width();++x) {
			if(!_mask->Value(x, y) && std::isfinite(_original->Value(x, y)))
				image->SetValue(x, y, 1.0);
			else
				image->SetValue(x, y, 0.0);
		}
	}
	return image;
}

void LocalFitMethod::PerformGaussianConvolution(Image2DPtr input)
{
	// Guassian convolution can be separated in two 1D convolution
	// because of properties of the 2D Gaussian function.
	Image2DPtr temp = Image2D::CreateZeroImagePtr(input->Width(), input->Height());
	for(int i=-_hSquareSize;i<=(int) _hSquareSize;++i) {
		num_t gaus = _weights[_vSquareSize][i+_hSquareSize];
		for(unsigned y=0;y<input->Height();++y) {
			unsigned xStart = i >= 0 ? 0 : -i;
			unsigned xEnd = i <= 0 ? input->Width() :  input->Width()-i;
			for(unsigned x=xStart;x<xEnd;++x)	{
				if(std::isfinite(input->Value(x+i,y)))
					temp->AddValue(x, y, input->Value(x+i,y)*gaus);
			}
		}
	}

	input->SetAll(0.0);
	for(int j=-_vSquareSize;j<=(int) _vSquareSize;++j) {
		num_t gaus = _weights[j+_vSquareSize][_hSquareSize];
		unsigned yStart = j >= 0 ? 0 : -j;
		unsigned yEnd = j <= 0 ? input->Height() :  input->Height()-j;
		for(unsigned y=yStart;y<yEnd;++y)	{
			for(unsigned x=0;x<input->Width();++x) {
				if(std::isfinite(temp->Value(x,y+j)))
					input->AddValue(x, y, temp->Value(x,y+j)*gaus);
			}
		}
	}
}
