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
#ifndef GUI_QUALITY__TIMEPLOTPAGE_H
#define GUI_QUALITY__TIMEPLOTPAGE_H

#include "twodimensionalplotpage.h"

#include "../../quality/statisticscollection.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class TimePlotPage : public TwoDimensionalPlotPage {
	protected:
		virtual const std::map<double, class DefaultStatistics> &GetStatistics() const
		{
			return GetStatCollection()->TimeStatistics();
		}
		
		virtual void StartLine(Plot2D &plot, const std::string &name, const std::string &yAxisDesc)
		{
			plot.StartLine(name, "Time", yAxisDesc, true);
		}
};

#endif
