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
#ifndef RFI_FOR_EACH_SIMULATED_BASELINE_ACTION
#define RFI_FOR_EACH_SIMULATED_BASELINE_ACTION

#include "../../imaging/defaultmodels.h"
#include "../../imaging/observatorium.h"
#include "../../imaging/model.h"

#include "action.h"

#include "../control/artifactset.h"
#include "../control/actionblock.h"

namespace rfiStrategy {

	class ForEachSimulatedBaselineAction : public ActionBlock
	{
		public:
			ForEachSimulatedBaselineAction() : ActionBlock()
			{
			}
			virtual std::string Description()
			{
				return "For each sim. baseline";
			}
			virtual ActionType Type() const { return ForEachSimulatedBaselineActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				/*
				double dec = 0.5*M_PI + 0.12800;
				double ra = -0.03000;
				double factor = 1.0;

				struct Observatorium *observatorium = new WSRTObservatorium(16);
				class Model *model = new Model();
				model->loadUrsaMajor(dec, ra, factor);
				model->loadUrsaMajorDistortingSource(dec, ra, factor);
				
				if(observatorium != 0 && model != 0)
				{*/
				ArtifactSet localArtifacts(artifacts);
				size_t antennaCount = 14; //observatorium->AntennaCount();
				size_t taskNr = 0;
				for(size_t a1=0;a1<antennaCount;++a1)
				{
					for(size_t a2=a1+1;a2<antennaCount;++a2)
					{
						listener.OnStartTask(*this, taskNr, antennaCount*(antennaCount-1)/2, "Simulating baseline");
						++taskNr;
						
						std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> pair = DefaultModels::LoadSet(DefaultModels::EmptySet, DefaultModels::ConstantDistortion, 0.0, 64, 2500000.0*4.0, a1, a2);
						TimeFrequencyData data = pair.first;
						TimeFrequencyMetaDataCPtr metaData = pair.second;

						localArtifacts.SetOriginalData(data);
						localArtifacts.SetContaminatedData(data);
						localArtifacts.SetMetaData(metaData);
						
						ActionBlock::Perform(localArtifacts, listener);
						
						listener.OnEndTask(*this);
					}
				}
			}
		private:
	};

} // namespace

#endif // RFI_FOR_EACH_SIMULATED_BASELINE_ACTION
