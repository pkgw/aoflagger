#ifndef DEFAULTSTRATEGYSET_H
#define DEFAULTSTRATEGYSET_H

#include <string>

#include <AOFlagger/strategy/actions/strategyaction.h>

namespace aotools {
	
	enum DefaultStrategyIdentifier {
		GUI_DEFAULT_STRATEGY,
		LOFAR_DEFAULT_STRATEGY,
		NDPPP_DEFAULT_STRATEGY,
		VLA_DEFAULT_STRATEGY,
		WSRT_DEFAULT_STRATEGY
	};

	class DefaultStrategySet
	{
		public:
			StrategyAction *CreateStrategy(enum DefaultStrategyIdentifier identifier);
			
			const DefaultStrategyGroup RootGroup() const
			{
				
			}
		private:
	};

	class DefaultStrategyGroupItem
	{
		public:
			DefaultStrategyGroupItem(const::string &name, const std::string &identifierString)
			: _name(name), _identifierString(identifierString)
			{
			}
		private:
			std::string _name;
			std::string _identifierString;
	};

	class DefaultStrategyGroup
	{
		public:
			DefaultStrategyGroup(const std::string &name) : _name(name)
			{
			}
		private:
			const std::string _name;
			std::vector<DefaultStrategyGroupItem> _strategies;
			std::vector<DefaultStrategyGroup> _subGroups;
	};
}

#endif
