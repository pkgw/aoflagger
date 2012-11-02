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
#ifndef TILEDIMAGE_H
#define TILEDIMAGE_H

#include <AOFlagger/strategy/algorithms/imagetile.h>
#include <AOFlagger/strategy/algorithms/surfacefitmethod.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class TiledImage : public SurfaceFitMethod {
	public:
		TiledImage();
		~TiledImage();
		void SetParameters(int polynomialTimeOrder, int polynomialFequencyOrder, size_t scanTileSize, size_t frequencyTileSize);
		virtual void Initialize(const TimeFrequencyData &data);
		virtual unsigned TaskCount() { return _vTileCount; }
		virtual void FitBackground(unsigned taskNumber);
		TimeFrequencyData Background();
		Mask2DPtr CreateFlagImage();
		//void SaveDistribution(const std::string &filename);
		virtual enum TimeFrequencyData::PhaseRepresentation PhaseRepresentation() const
		{
			return TimeFrequencyData::AmplitudePart;
		}
	private:
		void SetMask(Mask2DCPtr mask);
		size_t StartScan(size_t hWindowIndex) const;
		size_t EndScan(size_t hWindowIndex) const { return StartScan(hWindowIndex+1); }
		size_t StartFrequency(size_t vWindowIndex) const;
		size_t EndFrequency(size_t vWindowIndex) const { return StartFrequency(vWindowIndex+1); }
		size_t HWindowIndex(size_t scan);
		size_t VWindowIndex(size_t frequency);

		ImageTile ***_tiles;
		Image2DCPtr _original;
		TimeFrequencyData _data;
		Image2DPtr _background;
		size_t _hTileCount, _vTileCount;
		size_t _scanTileSize, _frequencyTileSize;
		long double _mean, _variance;
		bool _backgroundInitialized;
		int _polynomialTimeOrder, _polynomialFequencyOrder;
};

#endif
