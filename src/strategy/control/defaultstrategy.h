#ifndef DEFAULTSTRATEGYSET_H
#define DEFAULTSTRATEGYSET_H

#include <string>

#include "../../interface/aoflagger.h"

namespace rfiStrategy {
	
	class Strategy;
	class ActionBlock;
	
	class DefaultStrategy
	{
		public:
		/**
		* The contents of this enum needs to be equal to aoflagger::StrategyId
		* defined in interfaces/aoflagger.h
		*/
		enum DefaultStrategyId {
			DEFAULT_STRATEGY = aoflagger::GENERIC_TELESCOPE,
			LOFAR_STRATEGY = aoflagger::LOFAR_TELESCOPE,
			MWA_STRATEGY = aoflagger::MWA_TELESCOPE,
			WSRT_STRATEGY = aoflagger::WSRT_TELESCOPE
		};

		/**
		* These flags need to be equal to aoflagger::StrategyFlags
		* defined in interfaces/aoflagger.h
		*/
		static const unsigned
			FLAG_NONE,
			FLAG_LOW_FREQUENCY,
			FLAG_HIGH_FREQUENCY,
			FLAG_TRANSIENTS,
			FLAG_ROBUST,
			FLAG_FAST,
			FLAG_OFF_AXIS_SOURCES,
			FLAG_UNSENSITIVE,
			FLAG_SENSITIVE,
			FLAG_GUI_FRIENDLY,
			FLAG_CLEAR_FLAGS;
				
		static rfiStrategy::Strategy *CreateStrategy(enum DefaultStrategyId strategyId, unsigned flags, double frequency=0.0, double timeRes=0.0, double frequencyRes=0.0);
		
		static void LoadDefaultFullStrategy(ActionBlock &destination, bool pedantic = false, bool pulsar = false);
		static void LoadDefaultSingleStrategy(ActionBlock &destination, bool pedantic = false, bool pulsar = false);

		private:
	};


}

#endif
