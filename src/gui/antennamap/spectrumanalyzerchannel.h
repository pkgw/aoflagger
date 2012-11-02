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
#ifndef ANTENNAMAP_SPECTRUMANALYZERCHANNEL_H
#define ANTENNAMAP_SPECTRUMANALYZER_H

#include <gtkmm/drawingarea.h>

namespace antennaMap
{
	
/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class SpectrumAnalyzerChannel {
	public:
		SpectrumAnalyzerChannel() : _value(0.0), _runningMaxValue(1.0)
		{
		}
		
		~SpectrumAnalyzerChannel()
		{
		}
		
		double Value() const
		{
			return _value;
		}
		
		void SetValue(double value, double timeInFrameSeconds)
		{
			runTo(timeInFrameSeconds);
			if(value > 1.0)
				value = 1.0;
			_value = value;
			if(value > _runningMaxValue)
				_runningMaxValue = value;
		}
		
		void Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX, double offsetY, double width, double height, double timeInFrameSeconds)
		{
			runTo(timeInFrameSeconds);
			
			cairo->set_line_width(1.0);
			
			// The outer circle
			cairo->rectangle(offsetX, offsetY + height * (1.0 - _value), width, height * _value);
			cairo->set_source_rgba(0.0, 0.0, 1.0, 1.0);
			cairo->fill_preserve();
			cairo->set_source_rgba(0.0, 0.0, 0.0, 1.0);
			cairo->stroke();
		}
		
	private:
		double _value, _runningMaxValue;
		double _lastFrameTime;
		
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
};

}

#endif
