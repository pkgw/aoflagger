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
#ifndef PLOT2D_H
#define PLOT2D_H

#include <gtkmm/drawingarea.h>

#include <stdexcept>
#include <string>

#include "plotable.h"
#include "plot2dpointset.h"
#include "system.h"
#include "horizontalplotscale.h"
#include "verticalplotscale.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Plot2D : public Plotable {
	public:
		enum RangeDetermination { MinMaxRange, WinsorizedRange, SpecifiedRange };
		
		Plot2D();
		~Plot2D();

		void Clear();
		Plot2DPointSet &StartLine(const std::string &label, const std::string &xDesc = "x", const std::string &yDesc = "y", bool xIsTime = false, enum Plot2DPointSet::DrawingStyle drawingStyle = Plot2DPointSet::DrawLines)
		{
			Plot2DPointSet *newSet = new Plot2DPointSet();
			newSet->SetLabel(label);
			newSet->SetXIsTime(xIsTime);
			newSet->SetXDesc(xDesc);
			newSet->SetYDesc(yDesc);
			newSet->SetDrawingStyle(drawingStyle);
			_pointSets.push_back(newSet);
			return *newSet;
		}
		Plot2DPointSet &StartLine(const std::string &label, enum Plot2DPointSet::DrawingStyle drawingStyle)
		{
			return StartLine(label, "x", "y", false, drawingStyle);
		}
		Plot2DPointSet &StartLine()
		{
			return StartLine("", "x", "y", false, Plot2DPointSet::DrawLines);
		}
		void PushDataPoint(double x, double y)
		{
			if(_pointSets.size() > 0)
				(*_pointSets.rbegin())->PushDataPoint(x,y);
			else
				throw std::runtime_error("Trying to push a data point into a plot without point sets (call StartLine first).");
		}
		size_t PointSetCount() const { return _pointSets.size(); }
		Plot2DPointSet &GetPointSet(size_t index) { return *_pointSets[index]; }
		const Plot2DPointSet &GetPointSet(size_t index) const { return *_pointSets[index]; }
		virtual void Render(Gtk::DrawingArea &drawingArea);
		void SetIncludeZeroYAxis(bool includeZeroAxis)
		{
			_system.SetIncludeZeroYAxis(includeZeroAxis);
			if(includeZeroAxis)
				_logarithmicYAxis = false;
		}
		void SetLogarithmicYAxis(bool logarithmicYAxis)
		{
			_logarithmicYAxis = logarithmicYAxis;
			if(_logarithmicYAxis)
				_system.SetIncludeZeroYAxis(false);
		}
		bool LogarithmicYAxis() const
		{
			return _logarithmicYAxis;
		}
		void SetVRangeDetermination(enum RangeDetermination range) {
			_vRangeDetermination = range;
		}
		enum RangeDetermination VRangeDetermination() const
		{
			return _vRangeDetermination;
		}
		void SetMaxY(double maxY)
		{
			_vRangeDetermination = SpecifiedRange;
			_specifiedMaxY = maxY;
		}
		double MaxY() const
		{
			if(_vRangeDetermination == SpecifiedRange)
				return _specifiedMaxY;
			else if(_pointSets.empty())
				return 1.0;
			else
				return _system.YRangeMax(**_pointSets.begin());
		}
		double MaxPositiveY() const
		{
			if(_vRangeDetermination == SpecifiedRange)
				return _specifiedMaxY;
			else if(_pointSets.empty())
				return 1.0;
			else
				return _system.YRangePositiveMax(**_pointSets.begin());
		}
		void SetMinY(double minY)
		{
			_vRangeDetermination = SpecifiedRange;
			_specifiedMinY = minY;
		}
		double MinY() const
		{
			if(_vRangeDetermination == SpecifiedRange)
				return _specifiedMinY;
			else if(_pointSets.empty())
				return -1.0;
			else
				return _system.YRangeMin(**_pointSets.begin());
		}
		double MinPositiveY() const
		{
			if(_vRangeDetermination == SpecifiedRange)
				return _specifiedMinY;
			else if(_pointSets.empty())
				return 0.1;
			else
				return _system.YRangePositiveMin(**_pointSets.begin());
		}
		void SetShowAxes(bool showAxes) {
			_showAxes = showAxes;
		}
		bool ShowAxes() const {
			return _showAxes;
		}
		void SetShowAxisDescriptions(bool showAxisDescriptions) {
			_showAxisDescriptions = showAxisDescriptions;
		}
		bool ShowAxisDescriptions() const {
			return _showAxisDescriptions;
		}
		void SetTitle(const std::string &title) { }
		void SavePdf(const std::string &filename);
		void SaveSvg(const std::string &filename);
		void SavePng(const std::string &filename);
	private:
		void render(Cairo::RefPtr<Cairo::Context> cr);
		void render(Cairo::RefPtr<Cairo::Context> cr, Plot2DPointSet &pointSet);

		HorizontalPlotScale _horizontalScale;
		VerticalPlotScale _verticalScale;
		std::vector<Plot2DPointSet*> _pointSets;
		int _width, _height;
		double _topMargin;
		System _system;
		bool _logarithmicYAxis, _showAxes, _showAxisDescriptions;
		double _specifiedMinY, _specifiedMaxY;
		enum RangeDetermination _vRangeDetermination;
};

#endif
