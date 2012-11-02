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
#ifndef ANTENNAMAP_CLOCK_H
#define ANTENNAMAP_CLOCK_H

#include <gtkmm/drawingarea.h>

#include <cmath>

namespace antennaMap
{
	
/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Clock {
	public:
		Clock() : _time(0.0)
		{
		}
		
		~Clock()
		{
		}
		
		void SetTime(double time)
		{
			_time = time;
		}
		
		unsigned Width() const
		{
			return 60;
		}
		
		void Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX, double offsetY)
		{
			cairo->set_line_width(2.0);
			
			double
				radius = 30.0,
				centerX = offsetX + radius + 10.0,
				centerY = offsetY + radius + 10.0;
			
			// The outer circle
			cairo->arc(centerX, centerY, radius, 0.0, 2.0*M_PI);
			cairo->set_source_rgba(0.7, 0.7, 1.0, 1.0);
			cairo->fill_preserve();
			cairo->set_source_rgba(0.0, 0.0, 0.0, 1.0);
			cairo->stroke();
			
			// The minute hand
			double minute = fmod(_time, 60.0*60.0) / (60.0*60.0);
			double minuteAngle = minute * M_PI * 2.0;
			cairo->move_to(centerX, centerY);
			cairo->line_to(centerX + sin(minuteAngle) * radius * 0.8, centerY - cos(minuteAngle) * radius * 0.8);
			
			// The hour hand
			double hour = fmod(_time, 60.0*60.0*12.0) / (60.0*60.0*12.0);
			double hourAngle = hour * M_PI * 2.0;
			cairo->move_to(centerX, centerY);
			cairo->line_to(centerX + sin(hourAngle) * radius * 0.5, centerY - cos(hourAngle) * radius * 0.5);
			
			cairo->stroke();
		}
		
	private:
		double _time;
};

}

#endif
