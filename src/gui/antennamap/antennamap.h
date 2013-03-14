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
#ifndef ANTENNAMAP_ANTENNAMAP_H
#define ANTENNAMAP_ANTENNAMAP_H

#include <fstream>
#include <iostream>

#include <gtkmm/drawingarea.h>

#include "../../msio/image2d.h"
#include "../../msio/measurementset.h"

#include "antenna.h"

namespace antennaMap
{

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AntennaMap {
	public:
		AntennaMap(MeasurementSet &set);
		~AntennaMap();
		void Draw(Cairo::RefPtr<Cairo::Context> cairo, int width, int height, double timeInFrameSeconds);
		
		double UCSToPixelX(double ucsX) const
		{
			return ucsX * _xUCSToPixelsFactor + _xUCSToPixelsOffset;
		}
		double UCSToPixelY(double ucsY) const
		{
			return ucsY * _yUCSToPixelsFactor + _yUCSToPixelsOffset;
		}
		void SetValues(Image2DCPtr correlationMatrix, double timeInFrameSeconds)
		{
			unsigned count = correlationMatrix->Height();
			std::vector<double> values(count);
			for(unsigned i=0;i<count;++i)
				values[i] = 0.0;
			
			for(unsigned x=0;x<count;++x)
			{
				for(unsigned y=x+1;y<count;++y)
				{
					values[x] += correlationMatrix->Value(x, y);
					values[y] += correlationMatrix->Value(x, y);
				}
			}
			
			num_t
				max = fabsn(values[0]),
				min = max;
			for(unsigned i=0;i<count;++i)
			{
				const num_t aValue = fabsn(values[i]);
				values[i] = aValue;
				if(aValue > max)
					max = aValue;
				if(aValue < min)
					min = aValue;
			}
				
			if(min > sqrtn(max))
				min = sqrtn(max);
			double normalizationFactor = (max==min) ? 0.0 : (1.0/(max - min));
			double lastFactorWeight = pow(0.5, timeInFrameSeconds - _lastFactorChangeTime);
			normalizationFactor = normalizationFactor * (1.0 - lastFactorWeight) +
				_lastFactor * lastFactorWeight;
			double normalizationOffset = -min * normalizationFactor;
			_lastFactor = normalizationFactor;
			_lastFactorChangeTime = timeInFrameSeconds;
				
			for(unsigned i=0;i<count;++i)
			{
				values[i] = values[i] * normalizationFactor + normalizationOffset;
				_antennas[i]->SetValue(values[i], timeInFrameSeconds);
			}
		}
		void OpenStatFile(const std::string &statFilename);
		void ReadStatFile(int timeSteps, double timeInFrameSeconds);
		bool HasStatFile() const { return _statFile != 0; }
		
		void OpenAntennaFile(const std::string &antennaFile, double timeInFrameSeconds);
		
		void OpenSpectrumFile(const std::string &spectrumFile, double timeInFrameSeconds);
	private:
		void initMetrics();
		void setValues(Image2DPtr correlationMatrix, Image2DCPtr weightMatrix, double timeInFrameSeconds)
		{
			for(unsigned y=0;y<correlationMatrix->Height();++y)
			{
				for(unsigned x=0;x<correlationMatrix->Width();++x)
				{
					if(weightMatrix->Value(x, y) != 0.0)
						correlationMatrix->SetValue(x, y, correlationMatrix->Value(x, y) / weightMatrix->Value(x, y));
					else
						correlationMatrix->SetValue(x, y, 0.0);
				}
			}
			SetValues(correlationMatrix, timeInFrameSeconds);
		}
		
		int
			_widthInPixels, _heightInPixels;
		double
			_xUCSToPixelsFactor, _yUCSToPixelsFactor,
			_xUCSToPixelsOffset, _yUCSToPixelsOffset;

		std::vector<Antenna*> _antennas;
		class Clock *_clock;
		class SpectrumAnalyzer *_analyzer;
		std::ifstream *_statFile;
		double _lastFactor;
		double _lastFactorChangeTime;
};

}

#endif
