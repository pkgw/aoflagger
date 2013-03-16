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

#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include <boost/function.hpp>

#include <set>
#include <vector>

#include "plot2d.h"

class PlotManager
{
	public:
		~PlotManager()
		{
			Clear();
		}
		Plot2D &NewPlot2D(const std::string &plotTitle)
		{
			std::string title = plotTitle;
			if(_plotTitles.find(title) != _plotTitles.end())
			{
				char addChar = 'B';
				std::string tryTitle;
				do {
					tryTitle = title + " (" + addChar + ')';
					++addChar;
				} while(_plotTitles.find(tryTitle) != _plotTitles.end() && addChar <= 'Z');
				if(addChar > 'Z')
					tryTitle = title + " (..)";
				title = tryTitle;
			}
			Plot2D *plot = new Plot2D();
			plot->SetTitle(title);
			_plotTitles.insert(title);
			_items.push_back(plot);
			return *plot;
		}
		
		void Update() { _onUpdate(); }
		
		boost::function<void()> &OnUpdate() { return _onUpdate; }
		
		const std::vector<Plot2D*> Items() const { return _items; }
		
		void Clear()
		{
			for(std::vector<Plot2D*>::const_iterator i=_items.begin(); i!=_items.end(); ++i)
				delete *i;
			_items.clear();
			Update();
		}
	private:
		std::vector<Plot2D*> _items;
		std::set<std::string> _plotTitles;
		
		boost::function<void()> _onUpdate;
};

#endif
