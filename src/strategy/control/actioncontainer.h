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

#include "../actions/action.h"

#ifndef RFIACTIONCONTAINER_H
#define RFIACTIONCONTAINER_H 

#include <vector>

namespace rfiStrategy {

	class ActionContainer : public Action
	{
		public:
			typedef std::vector<class Action*>::const_iterator const_iterator;
			typedef std::vector<class Action*>::iterator iterator;

			inline virtual ~ActionContainer();
			inline void Add(class Action *newAction);
			inline void RemoveAndDelete(class Action *action);
			inline void RemoveWithoutDelete(class Action *action);
			inline void RemoveAll();
			size_t GetChildCount() const throw() { return _childActions.size(); }
			Action &GetChild(size_t index) const { return *_childActions[index]; }
			Action &GetFirstChild() const { return *_childActions.front(); }
			Action &GetLastChild() const { return *_childActions.back(); }
			void MoveChildUp(size_t childIndex)
			{
				if(childIndex > 0)
				{
					class Action *movedAction = _childActions[childIndex];
					_childActions[childIndex] = _childActions[childIndex-1];
					_childActions[childIndex-1] = movedAction;
				}
			}
			void MoveChildDown(size_t childIndex)
			{
				if(childIndex < _childActions.size()-1)
				{
					class Action *movedAction = _childActions[childIndex];
					_childActions[childIndex] = _childActions[childIndex+1];
					_childActions[childIndex+1] = movedAction;
				}
			}
			inline void InitializeAll();
			inline void FinishAll();

			iterator begin() { return _childActions.begin(); }
			iterator end() { return _childActions.end(); }
			const_iterator begin() const { return _childActions.begin(); }
			const_iterator end() const { return _childActions.end(); }
		private:
			std::vector<class Action*> _childActions;
	};
}

#include "../actions/action.h"

#include "strategyiterator.h"

namespace rfiStrategy {

	ActionContainer::~ActionContainer()
	{
		for(std::vector<class Action*>::iterator i=_childActions.begin();i!=_childActions.end();++i)
			delete *i;
	}

	void ActionContainer::RemoveAll()
	{
		for(std::vector<class Action*>::iterator i=_childActions.begin();i!=_childActions.end();++i)
			delete *i;
		_childActions.clear();
	}

	void ActionContainer::Add(class Action *newAction)
	{
		_childActions.push_back(newAction);
		newAction->_parent = this;
	}

	void ActionContainer::RemoveWithoutDelete(class Action *action)
	{
		for(std::vector<class Action*>::iterator i=_childActions.begin();i!=_childActions.end();++i)
			if(*i == action) {
				_childActions.erase(i);
				break;
			}
	}

	void ActionContainer::RemoveAndDelete(class Action *action)
	{
		for(std::vector<class Action*>::iterator i=_childActions.begin();i!=_childActions.end();++i)
			if(*i == action) {
				_childActions.erase(i);
				delete action;
				break;
			}
	}

	void ActionContainer::InitializeAll()
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(*this);
		while(!i.PastEnd())
		{
			i->Initialize();
			++i;
		}
	}
	
	void ActionContainer::FinishAll()
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(*this);
		while(!i.PastEnd())
		{
			i->Finish();
			++i;
		}
	}
}

#endif // RFIACTIONCONTAINER_H
