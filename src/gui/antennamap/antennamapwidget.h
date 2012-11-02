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
#ifndef ANTENNAMAP_ANTENNAMAPWIDGET_H
#define ANTENNAMAP_ANTENNAMAPWIDGET_H

#include <gtkmm/drawingarea.h>

#include "../../util/stopwatch.h"

#include "antennamap.h"

namespace antennaMap
{
	
/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AntennaMapWidget : public Gtk::DrawingArea {
	public:
		AntennaMapWidget() : _map(0), _watch(true)
		{
			signal_expose_event().connect(sigc::mem_fun(*this, &AntennaMapWidget::onExposeEvent) );
		}
		
		~AntennaMapWidget()
		{
			if(_map != 0)
				delete _map;
		}
		
		void SetMeasurementSet(MeasurementSet &set)
		{
			if(_map != 0)
				delete _map;
			_map = new AntennaMap(set);
			_watch.Reset();
			_watch.Start();
		}
		
		void SetValuesFromAntennaFile(const std::string &antennaFile)
		{
			_map->OpenAntennaFile(antennaFile, _watch.Seconds());
		}
		
		void SetValuesFromSpectrumFile(const std::string &spectrumFile)
		{
			_map->OpenSpectrumFile(spectrumFile, _watch.Seconds());
		}
		
		void SetMovieTimeStepsPerFrame(const double stepsPerFrame)
		{
			_movieTimeStepsPerFrame = stepsPerFrame;
		}

		void Update()
		{
			redraw();
		}
		
		AntennaMap &Map()
		{
			return *_map;
		}
	private:
		bool onExposeEvent(GdkEventExpose *)
		{
			redraw();
			return true;
		}

		void redraw()
		{
			if(_map != 0)
			{
				Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();
				Gtk::Allocation allocation = get_allocation();
				int width = allocation.get_width();
				int height = allocation.get_height();
				if(_map->HasStatFile())
					_map->ReadStatFile(_movieTimeStepsPerFrame, _watch.Seconds());
				_map->Draw(cr, width, height, _watch.Seconds());
			}
		}
		
		AntennaMap *_map;
		Stopwatch _watch;
		double _movieTimeStepsPerFrame;
};

}

#endif
