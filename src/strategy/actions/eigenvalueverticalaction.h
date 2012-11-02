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
#ifndef EIGENVALUEVERTICALACTION_H
#define EIGENVALUEVERTICALACTION_H

#include "../../msio/timefrequencydata.h"

#include "action.h"

#include "../algorithms/vertevd.h"

#include "../control/artifactset.h"

namespace rfiStrategy {
	
	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class EigenValueVerticalAction : public Action {
		public:
			EigenValueVerticalAction() : _timeIntegrated(true)
			{
			}
			~EigenValueVerticalAction()
			{
			}
			virtual std::string Description()
			{
				return "Eigen value decomposition (vertical)";
			}

			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData &data = artifacts.ContaminatedData();
				if(data.PolarisationCount()!=1)
				{
					throw std::runtime_error("Eigen value decompisition requires one polarization");
				}
				VertEVD::Perform(data, _timeIntegrated);
			}

			virtual ActionType Type() const { return EigenValueVerticalActionType; }
		private:
			bool _timeIntegrated;
	};

}
	
#endif // EIGENVALUEVERTICALACTION_H
