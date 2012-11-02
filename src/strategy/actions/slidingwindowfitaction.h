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

#ifndef RFISLIDINGWINDOWFITACTION_H
#define RFISLIDINGWINDOWFITACTION_H 

#include "action.h"

#include "slidingwindowfitparameters.h"

namespace rfiStrategy {

	class SlidingWindowFitAction : public Action
	{
		public:
			SlidingWindowFitAction() { LoadDefaults(); }
			virtual ~SlidingWindowFitAction() { }
			virtual std::string Description()
			{
				return "Sliding window fit";
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);
			virtual ActionType Type() const { return SlidingWindowFitActionType; }

			const SlidingWindowFitParameters &Parameters() const throw() { return _parameters; }
			SlidingWindowFitParameters &Parameters() throw() { return _parameters; }

			void LoadDefaults();
		private:
			SlidingWindowFitParameters _parameters;
	};

}

#endif // RFISLIDINGWINDOWFITACTION_H
