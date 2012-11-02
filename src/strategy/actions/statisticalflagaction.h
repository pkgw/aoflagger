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

#ifndef STATISTICALFLAGACTION_H
#define STATISTICALFLAGACTION_H

#include "../../msio/types.h"

#include "action.h"

namespace rfiStrategy {

	class StatisticalFlagAction : public Action
	{
		public:
			StatisticalFlagAction()
			: _enlargeTimeSize(0), _enlargeFrequencySize(0), _maxContaminatedTimesRatio(0.5), _maxContaminatedFrequenciesRatio(0.5),
			_minimumGoodTimeRatio(0.2), _minimumGoodFrequencyRatio(0.2) { }
			virtual ~StatisticalFlagAction() { }
			virtual std::string Description()
			{
				return "Statistical flagging";
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);
			virtual ActionType Type() const { return StatisticalFlagActionType; }

			size_t EnlargeTimeSize() const throw() { return _enlargeTimeSize; }
			void SetEnlargeTimeSize(size_t size) throw() { _enlargeTimeSize = size; }
			size_t EnlargeFrequencySize() const throw() { return _enlargeFrequencySize; }
			void SetEnlargeFrequencySize(size_t size) throw() { _enlargeFrequencySize = size; }
			
			num_t MaxContaminatedTimesRatio() const throw() { return _maxContaminatedTimesRatio; }
			void SetMaxContaminatedTimesRatio(num_t newValue) throw() { _maxContaminatedTimesRatio = newValue; }
			num_t MaxContaminatedFrequenciesRatio() const throw() { return _maxContaminatedFrequenciesRatio; }
			void SetMaxContaminatedFrequenciesRatio(num_t newValue) throw() { _maxContaminatedFrequenciesRatio = newValue; }
			
			num_t MinimumGoodTimeRatio() const throw() { return _minimumGoodTimeRatio; }
			void SetMinimumGoodTimeRatio(num_t newValue) throw() { _minimumGoodTimeRatio = newValue; }
			num_t MinimumGoodFrequencyRatio() const throw() { return _minimumGoodFrequencyRatio; }
			void SetMinimumGoodFrequencyRatio(num_t newValue) throw() { _minimumGoodFrequencyRatio = newValue; }
		private:
			size_t _enlargeTimeSize;
			size_t _enlargeFrequencySize;
			num_t _maxContaminatedTimesRatio;
			num_t _maxContaminatedFrequenciesRatio;
			num_t _minimumGoodTimeRatio;
			num_t _minimumGoodFrequencyRatio;
	};

}

#endif // STATISTICALFLAGACTION_H
