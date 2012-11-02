#ifndef STRATEGY_ITERATOR_H
#define STRATEGY_ITERATOR_H

#include <stack>

#include "actioncontainer.h"

namespace rfiStrategy
{
	class StrategyIterator
	{
		public:
			StrategyIterator(const StrategyIterator &source)
				: _indices(source._indices), _currentAction(source._currentAction)
			{
			}

			StrategyIterator &operator=(const StrategyIterator &source)
			{
				_indices = source._indices;
				_currentAction = source._currentAction;
				return *this;
			}

			StrategyIterator &operator++()
			{
				ActionContainer *container = dynamic_cast<ActionContainer*>(_currentAction);
				if(container != 0 && container->GetChildCount()!=0)
				{
					_currentAction = &container->GetFirstChild();
					_indices.push(0);
				} else {
					// Current action is not a container, so go back levels until a next action is available
					ActionContainer *parent = _currentAction->Parent();
					size_t index = _indices.top();
					_indices.pop();
					++index;

					while(_indices.size() != 0 && index >= parent->GetChildCount())
					{
						index = _indices.top();
						_indices.pop();
						parent = parent->Parent();
						++index;
					}

					_indices.push(index);
					if(parent != 0 && index < parent->GetChildCount())
					{
						_currentAction = &parent->GetChild(index);
					} else {
						while(_currentAction->Parent() != 0)
							_currentAction = _currentAction->Parent();
					}
				}
				return *this;
			}

			Action &operator*() const
			{
				return *_currentAction;
			}

			Action *operator->() const
			{
				return _currentAction;
			}

			bool PastEnd() const
			{
				if(_indices.size() != 1)
				{
					return false;
				} else
				{
					ActionContainer *container = static_cast<ActionContainer*>(_currentAction);
					return _indices.top() >= container->GetChildCount();
				}
			}

			static StrategyIterator NewStartIterator(ActionContainer &action)
			{
				return StrategyIterator(&action, 0);
			}

			static StrategyIterator NewEndIterator(ActionContainer &action)
			{
				return StrategyIterator(&action, action.GetChildCount());
			}

		private:
			StrategyIterator(Action *rootAction, size_t rootIndex)
			: _currentAction(rootAction)
			{
				_indices.push(rootIndex);
			} 

			// _indices contains the path that is followed to reach the _currentAction, starting from the root
			// action. It contains always at least one value. It has one zero value if the iterator is
			// pointing to the root strategy (without children)
			// and contains one value ChildCount of root if the iterator is pointed past the root strategy.
			std::stack<size_t> _indices;

			// _currentAction is the action currently pointed at by the iterator, except if
			// the iterator is pointed past the end. In that case, it points to the root
			// action.
			Action *_currentAction;
	};

}

#endif // STRATEGY_ITERATOR_H
