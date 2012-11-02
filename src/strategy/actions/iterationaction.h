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

#ifndef RFIITERATIONBLOCK_H
#define RFIITERATIONBLOCK_H 

#include <sstream>

#include "action.h"

#include "../control/artifactset.h"
#include "../control/actionblock.h"

#include "../../util/progresslistener.h"

namespace rfiStrategy {

	class IterationBlock : public ActionBlock
	{
		public:
			IterationBlock() : _iterationCount(4), _sensitivityStart(10.0L) { }
			virtual ~IterationBlock() { }

			virtual std::string Description()
			{
				std::stringstream s;
				s << "Iterate " << _iterationCount << " times";
				return s.str();
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener)
			{
				long double oldSensitivity = artifacts.Sensitivity();

				long double sensitivityStep = powl(_sensitivityStart, 1.0L/_iterationCount);
				long double sensitivity = _sensitivityStart;

				for(size_t i=0;i<_iterationCount;++i)
				{
					artifacts.SetSensitivity(sensitivity * oldSensitivity);
					listener.OnStartTask(*this, i, _iterationCount, "Iteration");
					ActionBlock::Perform(artifacts, listener);
					listener.OnEndTask(*this);
					sensitivity /= sensitivityStep;
				}
				artifacts.SetSensitivity(oldSensitivity);
			}
			virtual ActionType Type() const { return IterationBlockType; }
			virtual unsigned int Weight() const { return ActionBlock::Weight() * _iterationCount; }

			size_t IterationCount() const throw() { return _iterationCount; }
			void SetIterationCount(size_t newCount) throw() { _iterationCount = newCount; }
			long double SensitivityStart() const throw() { return _sensitivityStart; }
			void SetSensitivityStart(long double sensitivityStart) throw() { _sensitivityStart = sensitivityStart; }
		private:
			size_t _iterationCount;
			long double _sensitivityStart;
	};

}

#endif // RFIITERATIONBLOCK_H
