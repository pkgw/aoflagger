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
#ifndef RFISTRATEGYACTIONFACTORY_H
#define RFISTRATEGYACTIONFACTORY_H

#include <string>
#include <vector>

#include "../actions/action.h"

namespace rfiStrategy {

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class ActionFactory {
		public:
			static const std::vector<std::string> GetActionList();
			static class Action *CreateAction(const std::string &action);
			static const char *GetDescription(ActionType actionType);
		private:
			ActionFactory();
			~ActionFactory();
	};
}

#endif // RFISTRATEGYACTIONFACTORY_H
