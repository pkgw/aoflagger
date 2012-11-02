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
#ifndef GUI_QUALITY__BASELINEPLOTPAGE_H
#define GUI_QUALITY__BASELINEPLOTPAGE_H

#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>

#include "../imagewidget.h"

#include "../../quality/qualitytablesformatter.h"

#include "grayscaleplotpage.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class BaselinePlotPage : public GrayScalePlotPage {
	public:
		BaselinePlotPage();
    virtual ~BaselinePlotPage();
		
		sigc::signal<void, const std::string &> SignalStatusChange() { return _signalStatusChange; }
		
		void SetStatistics(class StatisticsCollection *statCollection, const std::vector<class AntennaInfo> &antennas)
		{
			_statCollection = statCollection;
			_antennas = &antennas;
			UpdateImage();
		}
		void CloseStatistics()
		{
			_statCollection = 0;
			_antennas = 0;
		}
		bool HasStatistics() const
		{
			return _statCollection != 0;
		}
	protected:
		virtual std::pair<TimeFrequencyData, TimeFrequencyMetaDataCPtr> ConstructImage();
	private:
		void onMouseMoved(size_t x, size_t y);
		
		class StatisticsCollection *_statCollection;
		const std::vector<class AntennaInfo> *_antennas;
		
		sigc::signal<void, const std::string &> _signalStatusChange;
};

#endif
