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
#include "plot2d.h"

#include <iostream>

Plot2D::Plot2D() :
	_logarithmicYAxis(false),
	_showAxes(true),
	_showAxisDescriptions(true),
	_vRangeDetermination(MinMaxRange)
{
}

Plot2D::~Plot2D()
{
	Clear();
}

void Plot2D::Clear()
{
	for(std::vector<Plot2DPointSet*>::iterator i=_pointSets.begin();i!=_pointSets.end();++i)
		delete *i;
	_pointSets.clear();
	_system.Clear();
}

void Plot2D::Render(Gtk::DrawingArea &drawingArea)
{
	_system.Clear();
	for(std::vector<Plot2DPointSet*>::iterator i=_pointSets.begin();i!=_pointSets.end();++i)
		_system.AddToSystem(**i);

	Glib::RefPtr<Gdk::Window> window = drawingArea.get_window();
	if(window != 0)
	{
		Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

		Gtk::Allocation allocation = drawingArea.get_allocation();
		_width = allocation.get_width();
		_height = allocation.get_height();
		render(cr);
	}
}

void Plot2D::SavePdf(const std::string &filename)
{
	Cairo::RefPtr<Cairo::PdfSurface> surface = Cairo::PdfSurface::create(filename, _width, _height);
	Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(surface);
	AOLogger::Debug << "Saving PDF of " << _width << " x " << _height << "\n";
	render(cairo);
	cairo->show_page();
	surface->finish();
}

void Plot2D::SaveSvg(const std::string &filename)
{
	Cairo::RefPtr<Cairo::SvgSurface> surface = Cairo::SvgSurface::create(filename, _width, _height);
	Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(surface);
	AOLogger::Debug << "Saving SVG of " << _width << " x " << _height << "\n";
	render(cairo);
	cairo->show_page();
	surface->finish();
}

void Plot2D::SavePng(const std::string &filename)
{
	Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, _width, _height);
	Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(surface);
	AOLogger::Debug << "Saving PNG of " << _width << " x " << _height << "\n";
	render(cairo);
	surface->write_to_png(filename);
}


void Plot2D::render(Cairo::RefPtr<Cairo::Context> cr)
{
	cr->set_line_width(2);

	cr->set_source_rgba(1, 1, 1, 1);
	cr->paint();
	cr->fill();

	size_t c = 0;
	
	if(!_pointSets.empty())
	{
		Plot2DPointSet &refPointSet = **_pointSets.begin();
		
		double verticalScaleWidth, horiScaleHeight;
	
		if(_showAxes)
		{
			_horizontalScale.SetDrawWithDescription(_showAxisDescriptions);
			_horizontalScale.SetRotateUnits(refPointSet.RotateUnits());
			if(refPointSet.HasTickLabels())
				_horizontalScale.InitializeTextTicks(refPointSet.TickLabels());
			else if(refPointSet.XIsTime())
				_horizontalScale.InitializeTimeTicks(_system.XRangeMin(refPointSet), _system.XRangeMax(refPointSet));
			else
				_horizontalScale.InitializeNumericTicks(_system.XRangeMin(refPointSet), _system.XRangeMax(refPointSet));
			_horizontalScale.SetUnitsCaption(refPointSet.XUnits());
			_topMargin = 10.0;
			_horizontalScale.SetPlotDimensions(_width, _height, _topMargin, 0.0);
			horiScaleHeight = _horizontalScale.GetHeight(cr);
			
			double rightMargin = _horizontalScale.GetRightMargin(cr);
			_verticalScale.SetDrawWithDescription(_showAxisDescriptions);
			if(_logarithmicYAxis)
				_verticalScale.InitializeLogarithmicTicks(MinY(), MaxY());
			else
				_verticalScale.InitializeNumericTicks(MinY(), MaxY());
			_verticalScale.SetUnitsCaption(refPointSet.YUnits());
			_verticalScale.SetPlotDimensions(_width - rightMargin, _height - horiScaleHeight - _topMargin, _topMargin);

			verticalScaleWidth =  _verticalScale.GetWidth(cr);
			_horizontalScale.SetPlotDimensions(_width - rightMargin, _height - horiScaleHeight, 0.0, verticalScaleWidth);
		}
		else {
			verticalScaleWidth = 0.0;
			horiScaleHeight = 0.0;
		}
		
		for(std::vector<Plot2DPointSet*>::iterator i=_pointSets.begin();i!=_pointSets.end();++i)
		{
			switch(c)
			{
				case 0: cr->set_source_rgba(1, 0, 0, 1); break;
				case 1: cr->set_source_rgba(0, 1, 0, 1); break;
				case 2: cr->set_source_rgba(0, 0, 1, 1); break;
				case 3: cr->set_source_rgba(0, 0, 0, 1); break;
				case 4: cr->set_source_rgba(1, 1, 0, 1); break;
				case 5: cr->set_source_rgba(1, 0, 1, 1); break;
				case 6: cr->set_source_rgba(0, 1, 1, 1); break;
				case 7: cr->set_source_rgba(0.5, 0.5, 0.5, 1); break;
			}
			c = (c+1)%8;

			render(cr, **i);
		}
		
		double rightMargin;
		if(_showAxes)
		{
			_horizontalScale.Draw(cr);
			_verticalScale.Draw(cr);
			rightMargin = _horizontalScale.GetRightMargin(cr);
		} else {
			rightMargin = 0.0;
		}
		
		cr->set_source_rgb(0.0, 0.0, 0.0);
		cr->rectangle(verticalScaleWidth, _topMargin, _width - verticalScaleWidth - rightMargin, _height - horiScaleHeight - _topMargin);
		cr->stroke();
	}
}

void Plot2D::render(Cairo::RefPtr<Cairo::Context> cr, Plot2DPointSet &pointSet)
{
	pointSet.Sort();

	double
		xLeft = _system.XRangeMin(pointSet),
		xRight = _system.XRangeMax(pointSet),
		yMin = _logarithmicYAxis ? MinPositiveY() : MinY(),
		yMax = _logarithmicYAxis ? MaxPositiveY() : MaxY();
	if(!std::isfinite(xLeft) || !std::isfinite(xRight))
	{
		xLeft = -1;
		xRight = 1;
	}
	else if(xLeft == xRight)
	{
		xLeft -= 1;
		xRight += 1;
	}
	if(!std::isfinite(yMin) || !std::isfinite(yMax))
	{
		yMin = -1;
		yMax = 1;
	}
	else if(yMin == yMax)
	{
		yMin -= 1;
		yMax += 1;
	}

	const double rightMargin = _showAxes ? _horizontalScale.GetRightMargin(cr) : 0.0;
	const double bottomMargin = _showAxes ? _horizontalScale.GetHeight(cr) : 0.0;
	const double plotLeftMargin = _showAxes ? _verticalScale.GetWidth(cr) : 0.0;
	
	const double plotWidth = _width - rightMargin - plotLeftMargin;
	const double plotHeight = _height - bottomMargin - _topMargin;
	
	double fx = (double) plotWidth / (xRight - xLeft);
	
	double
		minYLog10 = log10(yMin),
		maxYLog10 = log10(yMax);

	bool hasPrevPoint = false;
	
	unsigned iterationCount = pointSet.Size();
	if(pointSet.DrawingStyle() == Plot2DPointSet::DrawLines && iterationCount!=0)
		--iterationCount;

	for(size_t i=0;i<iterationCount;++i)
	{
		double
			y1Val, y2Val;
		
		if(_logarithmicYAxis)
		{
			if(pointSet.GetY(i) <= 0.0)
				y1Val = 0.0;
			else
				y1Val = (log10(pointSet.GetY(i)) - minYLog10) / (maxYLog10 - minYLog10);
			if(pointSet.GetY(i+1) <= 0.0)
				y2Val = 0.0;
			else
				y2Val = (log10(pointSet.GetY(i+1)) - minYLog10) / (maxYLog10 - minYLog10);
		} else {
			y1Val = (pointSet.GetY(i) - yMin) / (yMax - yMin);
			y2Val = (pointSet.GetY(i+1) - yMin) / (yMax - yMin);
		}
		if(y1Val < 0.0) y1Val = 0.0;
		if(y1Val > 1.0) y1Val = 1.0;
		if(y2Val < 0.0) y2Val = 0.0;
		if(y2Val > 1.0) y2Val = 1.0;
			
		double
			x1 = (pointSet.GetX(i) - xLeft) * fx + plotLeftMargin,
			x2 = (pointSet.GetX(i+1) - xLeft) * fx + plotLeftMargin,
			y1 = (1.0 - y1Val) * plotHeight + _topMargin,
			y2 = (1.0 - y2Val) * plotHeight + _topMargin;

		if(std::isfinite(x1) && std::isfinite(y1))
		{
			switch(pointSet.DrawingStyle())
			{
				case Plot2DPointSet::DrawLines:
					if(std::isfinite(x2) && std::isfinite(y2))
					{
						if(!hasPrevPoint)
							cr->move_to(x1, y1);
						cr->line_to(x2, y2);
						hasPrevPoint = true;
					} else {
						hasPrevPoint = false;
					}
					break;
				case Plot2DPointSet::DrawPoints:
					cr->move_to(x1 + 2.0, y1);
					cr->arc(x1, y1, 2.0, 0.0, 2*M_PI);
					break;
				case Plot2DPointSet::DrawColumns:
					if(y1 <= _topMargin + plotHeight)
					{
						double
							width = 10.0,
							startX = x1 - width*0.5,
							endX = x1 + width*0.5;
						if(startX < plotLeftMargin)
							startX = plotLeftMargin;
						if(endX > plotWidth + plotLeftMargin)
							endX = plotWidth + plotLeftMargin;
						cr->rectangle(startX, y1, endX - startX, _topMargin + plotHeight - y1);
					}
					break;
			}
		} else {
		}
	}
	switch(pointSet.DrawingStyle())
	{
		case Plot2DPointSet::DrawLines:
			cr->stroke();
			break;
		case Plot2DPointSet::DrawPoints:
			cr->fill();
			break;
		case Plot2DPointSet::DrawColumns:
			cr->fill_preserve();
			Cairo::RefPtr<Cairo::Pattern> source = cr->get_source();
			cr->set_source_rgb(0.0, 0.0, 0.0);
			cr->stroke();
			cr->set_source(source);
			break;
	}

	// Draw "zero y" x-axis
	if(yMin <= 0.0 && yMax >= 0.0 && !_logarithmicYAxis)
	{
		cr->set_source_rgba(0.5, 0.5, 0.5, 1);
		cr->move_to(plotLeftMargin, yMax * plotHeight / (yMax - yMin) + _topMargin);
		cr->line_to(plotWidth + plotLeftMargin, yMax * plotHeight / (yMax - yMin) + _topMargin);
		cr->stroke();
	}
}
