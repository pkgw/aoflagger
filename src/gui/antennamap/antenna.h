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
#ifndef ANTENNAMAP_ANTENNA_H
#define ANTENNAMAP_ANTENNA_H

#include <cmath>

#include <gtkmm/drawingarea.h>

#include "../../msio/antennainfo.h"

namespace antennaMap
{
	class AntennaMap;
	
/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Antenna {
	public:
		Antenna(AntennaMap &map, AntennaInfo antennaInfo)
		: _map(map), _info(antennaInfo), _value(0.0), _runningMaxValue(1.0), _lastFrameTime(0.0)
		{
		}
		~Antenna()
		{
		}
		bool Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX, double offsetY, int layer, double timeInFrameSeconds);
		
		double GetXInUCS() const
		{
			return _info.position.y;
		}
		double GetYInUCS() const
		{
			return _info.position.z;
		}
		double Value() const
		{
			return _value;
		}
		void SetValue(double newValue, double timeInFrameSeconds)
		{
			runTo(timeInFrameSeconds);
			if(newValue > 1.0)
				newValue = 1.0;
			_value = newValue;
			if(newValue > _runningMaxValue)
				_runningMaxValue = newValue;
		}
		static double GetMaxWidthInPixels()
		{
			return GetMaxCircleRadius()*2.0;
		}
		static double GetMaxHeightInPixels()
		{
			return GetMaxCircleRadius()*2.0;
		}
		static double GetMaxCircleRadius()
		{
			return 60.0;
		}
	private:
		void runTo(double timeInFrameSeconds)
		{
			if(timeInFrameSeconds < _lastFrameTime)
			{
				_runningMaxValue = 0.0;
			} else {
				double decrease = (timeInFrameSeconds - _lastFrameTime) / 10.0;
				if(_runningMaxValue > decrease)
					_runningMaxValue -= decrease;
				else
					_runningMaxValue = 0.0;
				if(_runningMaxValue < _value)
					_runningMaxValue = _value;
			}
			_lastFrameTime = timeInFrameSeconds;
		}
		
		AntennaMap &_map;
		const AntennaInfo _info;
		double _value;
		double _runningMaxValue;
		double _lastFrameTime;
};

}

#endif

