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

#include "../../gui/quality/datawindow.h"
#include "../../gui/plot/plot2d.h"

#include <sstream>
#include <iomanip>

void DataWindow::SetData(const Plot2D &plot)
{
	_plot = &plot;
	int selectedIndex = _comboBox.get_active_row_number();
	if(selectedIndex < 0)
		selectedIndex = 0;
	_comboListStore->clear();
	for(size_t i=0;i<plot.PointSetCount();++i)
	{
		std::stringstream str;
		str << (i+1) << ". " << plot.GetPointSet(i).Label();
		Gtk::TreeModel::Row row = *_comboListStore->append();
		row[_comboColumnRecord._comboListNameColumn] = str.str();
	}
	if(selectedIndex < (int) plot.PointSetCount())
		_comboBox.set_active(selectedIndex);
	else if(plot.PointSetCount() > 0)
		_comboBox.set_active(0);
	onComboChange();
}

void DataWindow::onComboChange()
{
	int active = _comboBox.get_active_row_number();
	if(active >= 0)
		loadData(active);
	else
		loadData(_plot->PointSetCount());
}

void DataWindow::loadData(size_t plotSetIndex)
{
	std::stringstream _dataStream;
	_dataStream << std::setprecision(14);
	if(_plot->PointSetCount() > plotSetIndex)
	{
		const Plot2DPointSet &pointSet = _plot->GetPointSet(plotSetIndex);
		const size_t valueCount = pointSet.Size();
		for(size_t i=0; i<valueCount; ++i)
		{
			const double
				x = pointSet.GetX(i),
				y = pointSet.GetY(i);
			if(pointSet.HasTickLabels())
			{
				std::string label = pointSet.TickLabels()[i];
				_dataStream << i << '\t' << label << '\t' << y << '\n';
			}
			else
				_dataStream << i << '\t' << x << '\t' << y << '\n';
		}
	}
	SetData(_dataStream.str());
}

