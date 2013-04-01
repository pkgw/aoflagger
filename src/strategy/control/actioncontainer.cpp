#include "actioncontainer.h"

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
