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
#include <AOFlagger/strategy/algorithms/tiledimage.h>

#include <cmath>
#include <vector>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/pngfile.h>

#include <AOFlagger/strategy/algorithms/mitigationtester.h>
#include <AOFlagger/strategy/algorithms/thresholdtools.h>

TiledImage::TiledImage() : _tiles(0)
{
}

TiledImage::~TiledImage()
{
	if(_tiles != 0) {
		for(size_t j=0;j<_vTileCount;++j) {
			for(size_t i=0;i<_hTileCount;++i) {
				delete[] _tiles[j][i];
			}
			delete[] _tiles[j];
		}
		delete[] _tiles;
	}
}

void TiledImage::SetParameters(int polynomialTimeOrder, int polynomialFequencyOrder, size_t scanTileSize, size_t frequencyTileSize)
{
	_scanTileSize = scanTileSize;
	_frequencyTileSize = frequencyTileSize;
	_polynomialTimeOrder = polynomialTimeOrder;
	_polynomialFequencyOrder = polynomialFequencyOrder;
}

void TiledImage::Initialize(const TimeFrequencyData &data)
{
	_data = data;
	_original = data.GetSingleImage();
	_hTileCount = (size_t) ceil((double) _original->Width() / _frequencyTileSize);
	_vTileCount = (size_t) ceil((double) _original->Height() / _scanTileSize);
	_tiles = new ImageTile**[_vTileCount];
	for(size_t j=0;j<_vTileCount;++j) {
		_tiles[j] = new ImageTile*[_hTileCount];
		size_t startFreq = StartFrequency(j);
		size_t endFreq = EndFrequency(j);
		for(size_t i=0;i<_hTileCount;++i) {
			size_t startScan = StartScan(i);
			size_t endScan = EndScan(i);
			_tiles[j][i] = new ImageTile();
			_tiles[j][i]->InitializeData(endFreq-startFreq, endScan-startScan, startFreq, startScan, *_original, _polynomialTimeOrder, _polynomialFequencyOrder);
		}
	}
	_backgroundInitialized = false;
	SetMask(data.GetSingleMask());
}

void TiledImage::SetMask(Mask2DCPtr mask)
{
	for(unsigned channel = 0;channel<_original->Height();++channel) {
		size_t vIndex = VWindowIndex(channel);
		for(unsigned scan=0;scan < _original->Width(); ++scan) {
			size_t hIndex = HWindowIndex(scan);
			_tiles[vIndex][hIndex]->SetWindowed(scan, channel, mask->Value(scan, channel) != 0.0);
		}
	}
}

void TiledImage::FitBackground(unsigned taskNumber)
{
	size_t j = taskNumber;
	for(size_t i=0;i<_hTileCount;++i) {
		_tiles[j][i]->FitBackground();
	}
	if(j+1 == _vTileCount)
		_backgroundInitialized = true;
}

size_t TiledImage::StartScan(size_t hWindowIndex) const
{
	// Because of rounding, we calculate
	// hWindowIndex * width / hTileCount  -  ((hWindowIndex * width) % hTileCount) / hTileCount
	return (size_t) ((double) hWindowIndex * _original->Width() / _hTileCount);
}

size_t TiledImage::StartFrequency(size_t vWindowIndex) const
{
	return (size_t) ((double) vWindowIndex * _original->Height() / _vTileCount);
}

size_t TiledImage::HWindowIndex(size_t scan)
{
	size_t index = (size_t) ((double) scan * _hTileCount / _original->Width());
	if(EndScan(index) > scan)
		return index;
	else
		return index+1;
}

size_t TiledImage::VWindowIndex(size_t frequency)
{
	size_t index = (size_t) ((double) frequency * _vTileCount / _original->Height());
	if(EndFrequency(index) > frequency)
		return index;
	else
		return index+1;
}

TimeFrequencyData TiledImage::Background()
{
	_background = Image2D::CreateEmptyImagePtr(_original->Width(), _original->Height());
	if(_backgroundInitialized) {
		for(size_t j=0;j<_vTileCount;++j) {
			for(size_t i=0;i<_hTileCount;++i) {
				_tiles[j][i]->AddBaseline(*_background, 1.0);
			}
		}
	} else {
		Mask2DPtr mask = CreateFlagImage();
		num_t mean, stddev;
		ThresholdTools::WinsorizedMeanAndStdDev(_original, mask, mean, stddev);
		mask.reset();

		for(size_t j=0;j<_original->Height();++j) {
			for(size_t i=0;i<_original->Width();++i) {
				_background->SetValue(i, j, mean);
			}
		}
	}
	return TimeFrequencyData(_data.PhaseRepresentation(), _data.Polarisation(), _background);
}

Mask2DPtr TiledImage::CreateFlagImage()
{
	Mask2DPtr mask = Mask2D::CreateUnsetMaskPtr(_original->Width(), _original->Height());
	for(unsigned channel = 0;channel<_original->Height();++channel) {
		size_t vIndex = VWindowIndex(channel);
		for(unsigned scan=0;scan < _original->Width(); ++scan) {
			size_t hIndex = HWindowIndex(scan);
			if(_tiles[vIndex][hIndex]->IsWindowed(scan, channel))
				mask->SetValue(scan, channel, true);
			else
				mask->SetValue(scan, channel, false);
		}
	}
	return mask;
}
