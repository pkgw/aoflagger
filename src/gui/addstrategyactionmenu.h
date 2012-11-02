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
#ifndef ADDSTRATEGYACTIONMENU_H
#define ADDSTRATEGYACTIONMENU_H

#include <gtkmm/menu.h>

#include "../strategy/control/actionfactory.h"

#include "editstrategywindow.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AddStrategyActionMenu : public Gtk::Menu {
	public:
		AddStrategyActionMenu(class EditStrategyWindow &editStrategyWindow) :
		_editStrategyWindow(editStrategyWindow)
		{
			std::vector<std::string> actions = rfiStrategy::ActionFactory::GetActionList();

			for(std::vector<std::string>::const_iterator i=actions.begin();i!=actions.end();++i)
			{
				Gtk::MenuItem *item = new Gtk::MenuItem(*i);
				append(*item);
				item->signal_activate().connect(sigc::bind<const std::string>(sigc::mem_fun(*this, &AddStrategyActionMenu::onActionSelected), *i));
				item->show();

				_items.push_back(item);
			}
		}
		
		~AddStrategyActionMenu()
		{
			for(std::vector<Gtk::MenuItem *>::const_iterator i=_items.begin();i!=_items.end();++i)
				delete *i;
		}
		
	private:
		void onActionSelected(const std::string str)
		{
			_editStrategyWindow.AddAction(rfiStrategy::ActionFactory::CreateAction(str));
		}

		class EditStrategyWindow &_editStrategyWindow;
		std::vector<Gtk::MenuItem *> _items;
};

#endif
