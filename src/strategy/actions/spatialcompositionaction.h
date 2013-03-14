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
#ifndef SPATIALCOMPOSITIONACTION_H
#define SPATIALCOMPOSITIONACTION_H

#include "action.h"

#include "../control/artifactset.h"

namespace rfiStrategy {

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class SpatialCompositionAction : public Action {
		public:
			enum Operation { SumCrossCorrelationsOperation, SumAutoCorrelationsOperation, EigenvalueDecompositionOperation, EigenvalueRemovalOperation } ;

			SpatialCompositionAction() : _operation(EigenvalueDecompositionOperation)
			{
			}
			virtual ~SpatialCompositionAction()
			{
			}
			virtual std::string Description()
			{
				switch(_operation)
				{
					default:
					case SumCrossCorrelationsOperation:
						return "Spatial composition (cross)";
					case SumAutoCorrelationsOperation:
						return "Spatial composition (auto)";
					case EigenvalueDecompositionOperation:
						return "Spatial composition (eigenvalue)";
					case EigenvalueRemovalOperation:
						return "Spatial composition (remove eigenvalue)";
				}
			}
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress);

			virtual ActionType Type() const { return SpatialCompositionActionType; }

			enum Operation Operation() const { return _operation; }
			void SetOperation(enum Operation operation) { _operation = operation; }
		private:
			enum Operation _operation;

			num_t sumCrossCorrelations(Image2DCPtr image) const;
			num_t sumAutoCorrelations(Image2DCPtr image) const;
			num_t eigenvalue(Image2DCPtr real, Image2DCPtr imaginary) const;
			std::pair<num_t, num_t> removeEigenvalue(Image2DCPtr real, Image2DCPtr imaginary) const;
	};

}

#endif // SPATIALCOMPOSITIONACTION_H
