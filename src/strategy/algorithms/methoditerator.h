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
#ifndef METHODTESTER_H
#define METHODTESTER_H

#include <string>

#include "../../msio/image2d.h"
#include "../../msio/mask2d.h"
#include "../../msio/timefrequencydata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class MethodIterator{
	private:
		MethodIterator();
		~MethodIterator();
	
		void ExecuteBackgroundFit(class SurfaceFitMethod &method, unsigned threadCount);
	
		void IterateBackgroundFit(class SurfaceFitMethod &method, unsigned threadCount, const std::string &name, unsigned iterationCount=10);
	
		void OutputMethodDetails(const class SurfaceFitMethod &method, const class Stopwatch &watch, class StatWriter &statWriter);
	
		static void SaveFlaggingToPng(Image2DCPtr image, Mask2DCPtr mask, const std::string &filename, const class BandInfo *bandInfo, bool showFlagging=true);
		static void SaveFlaggingToPng(Image2DCPtr image, Mask2DCPtr mask, const std::string &filename, const class BandInfo *bandInfo, bool showFlagging, bool winsorizedStretch, bool useColor);

		void SetInput(const class TimeFrequencyData &input) { _input = input; }
		void SetOriginalFlag(Mask2DCPtr flagMask) { _originalFlagging = flagMask; }
		void SetThresholdConfig(class ThresholdConfig &config) { _thresholdConfig = &config; }
		ThresholdConfig &Config() const throw() { return *_thresholdConfig; }
		Mask2DCPtr GetMask() const throw() { return _mask; }
		unsigned GetMaskOverlap(Mask2DCPtr mask1, Mask2DCPtr mask2);

		void SetWriteStats(bool newValue) { _writeStats = newValue; }
		void SetSavePNGs(bool newValue) { _savePNGs = newValue; }
		void SetVerbose(bool newValue) { _verbose = newValue; }
		void SetBandInfo(const BandInfo *bandInfo) { _bandInfo = bandInfo; }
	private:
		void ExecuteThreshold(Image2DCPtr image, Image2DCPtr background, int imageIndex, long double sensitivity);
		void OutputStatistics(Image2DCPtr image, Mask2DCPtr mask);

		class TimeFrequencyData _input;
		Mask2DCPtr _originalFlagging;
		Mask2DPtr _mask;
		bool _writeStats, _savePNGs;
		class ThresholdConfig *_thresholdConfig;
		bool _verbose;
		const class BandInfo *_bandInfo;
		StatWriter *_statWriter;
};

#endif
