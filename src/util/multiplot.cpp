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
#include "../util/multiplot.h"

MultiPlot::MultiPlot(Plot2D &plot, size_t plotCount)
	: _plotCount(plotCount), _plot(plot)
{
	_points = new PointList*[plotCount];
	for(size_t i=0;i<plotCount;++i)
		_points[i] = new PointList();
	_legends = new std::string[plotCount];
}

MultiPlot::~MultiPlot()
{
	for(size_t i=0;i<_plotCount;++i)
		delete _points[i];
	delete[] _points;
	delete[] _legends;
}

void MultiPlot::Finish()
{
	for(size_t i=0;i<_plotCount;++i)
	{
		_plot.StartLine(_legends[i], _xAxisText, _yAxisText, false, Plot2DPointSet::DrawPoints);
		PointList &list = *_points[i];
		for(PointList::const_iterator p=list.begin();p!=list.end();++p)
		{
			_plot.PushDataPoint(p->x, p->y);
		}
		list.clear();
	}
}

