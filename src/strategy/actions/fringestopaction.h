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

#ifndef FRINGESTOPACTION_H
#define FRINGESTOPACTION_H 

#include "../../msio/types.h"

#include "action.h"

namespace rfiStrategy {

	class FringeStopAction : public Action
	{
		public:
			FringeStopAction() : _fringesToConsider(1.0L), _maxWindowSize(128), _fitChannelsIndividually(true), _onlyFringeStop(false), _newPhaseCentreRA(0.0), _newPhaseCentreDec(0.5 * M_PInl) { }
			virtual ~FringeStopAction() { }
			
			virtual std::string Description()
			{
				return "Fringe stop recovery";
			}
			
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);

			long double FringesToConsider() const { return _fringesToConsider; }
			void SetFringesToConsider(long double fringes) { _fringesToConsider = fringes; }

			size_t MaxWindowSize() const { return _maxWindowSize; }
			void SetMaxWindowSize(size_t windowSize) { _maxWindowSize = windowSize; }

			size_t MinWindowSize() const { return _minWindowSize; }
			void SetMinWindowSize(size_t windowSize) { _minWindowSize = windowSize; }

			bool FitChannelsIndividually() const { return _fitChannelsIndividually; }
			void SetFitChannelsIndividually(bool fitChannelsIndividually) throw() { _fitChannelsIndividually = fitChannelsIndividually; }
			
			bool OnlyFringeStop() const { return _onlyFringeStop; }
			void SetOnlyFringeStop(bool onlyFringeStop) throw() {
				_onlyFringeStop = onlyFringeStop; }
				
			virtual ActionType Type() const { return FringeStopActionType; }
			
			long double NewPhaseCentreRA() const { return _newPhaseCentreRA; }
			void SetNewPhaseCentreRA(long double newPhaseCentreRA) { _newPhaseCentreRA = newPhaseCentreRA; }
			
			long double NewPhaseCentreDec() const { return _newPhaseCentreDec; }
			void SetNewPhaseCentreDec(long double newPhaseCentreDec) { _newPhaseCentreDec = newPhaseCentreDec; }
			
		private:
			long double _fringesToConsider;
			size_t _minWindowSize, _maxWindowSize;
			bool _fitChannelsIndividually;
			bool _onlyFringeStop;
			long double _newPhaseCentreRA, _newPhaseCentreDec;
	};

}

#endif // FRINGESTOPACTION_H
