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
#ifndef MULTIPLOT_H
#define MULTIPLOT_H

#include "plot.h"

#include "../msio/types.h"

#include <vector>

#include "../gui/plot/plotmanager.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class MultiPlot {
	public:
		MultiPlot(Plot2D &plot, size_t plotCount);
		~MultiPlot();

		void AddPoint(size_t plotIndex, num_t x, num_t y)
		{
			_points[plotIndex]->push_back(Point(x, y));
		}
		void SetLegend(int index, const std::string &title)
		{
			_legends[index] = title;
		}
		void Finish();
		Plot2D &Plot() { return _plot; }
		void SetXAxisText(const std::string text)
		{
			_xAxisText = text;
		}
		void SetYAxisText(const std::string text)
		{
			_yAxisText = text;
		}
	private:
		struct Point {
			Point(num_t _x, num_t _y) : x(_x), y(_y) { } 
			num_t x, y;
		};
		typedef std::vector<struct Point> PointList;
		std::string *_legends;
		PointList **_points;
		size_t _plotCount;
		Plot2D &_plot;
		std::string _xAxisText, _yAxisText;
};

#endif
