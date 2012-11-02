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
#ifndef RFI_ADAPTER
#define RFI_ADAPTER

#include "../control/artifactset.h"
#include "../control/actionblock.h"

namespace rfiStrategy {

	class Adapter : public ActionBlock
	{
		public:
			Adapter() : ActionBlock(), _restoreOriginals(false)
			{
			}
			virtual std::string Description()
			{
				return "On amplitude";
			}
			virtual ActionType Type() const { return AdapterType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				enum TimeFrequencyData::PhaseRepresentation contaminatedPhase = 
					artifacts.ContaminatedData().PhaseRepresentation();
				enum TimeFrequencyData::PhaseRepresentation revisedPhase = 
					artifacts.RevisedData().PhaseRepresentation();
				enum TimeFrequencyData::PhaseRepresentation originalPhase = 
					artifacts.OriginalData().PhaseRepresentation();

				if(contaminatedPhase == TimeFrequencyData::ComplexRepresentation)
				{
					TimeFrequencyData *newContaminatedData =
						artifacts.ContaminatedData().CreateTFData(TimeFrequencyData::AmplitudePart);
					artifacts.SetContaminatedData(*newContaminatedData);
					delete newContaminatedData;
				}
				if(revisedPhase == TimeFrequencyData::ComplexRepresentation)
				{
					TimeFrequencyData *newRevisedData =
						artifacts.RevisedData().CreateTFData(TimeFrequencyData::AmplitudePart);
					artifacts.SetRevisedData(*newRevisedData);
					delete newRevisedData;
				}
				if(originalPhase == TimeFrequencyData::ComplexRepresentation)
				{
					TimeFrequencyData *newOriginalData =
						artifacts.OriginalData().CreateTFData(TimeFrequencyData::AmplitudePart);
					artifacts.SetOriginalData(*newOriginalData);
					delete newOriginalData;
				}

				ActionBlock::Perform(artifacts, listener);

				if(_restoreOriginals)
				{
					if(contaminatedPhase == TimeFrequencyData::ComplexRepresentation)
					{
						TimeFrequencyData *newContaminatedData =
							TimeFrequencyData::CreateTFDataFromComplexCombination(artifacts.ContaminatedData(), artifacts.ContaminatedData());
						newContaminatedData->MultiplyImages(1.0L/M_SQRT2);
						newContaminatedData->SetMask(artifacts.ContaminatedData());
						artifacts.SetContaminatedData(*newContaminatedData);
						delete newContaminatedData;
					}
					if(revisedPhase == TimeFrequencyData::ComplexRepresentation)
					{
						TimeFrequencyData *newRevisedData =
							TimeFrequencyData::CreateTFDataFromComplexCombination(artifacts.RevisedData(), artifacts.RevisedData());
						newRevisedData->MultiplyImages(1.0L/M_SQRT2);
						newRevisedData->SetMask(artifacts.RevisedData());
						artifacts.SetRevisedData(*newRevisedData);
						delete newRevisedData;
					}
					if(originalPhase == TimeFrequencyData::ComplexRepresentation)
					{
						TimeFrequencyData *newOriginalData =
							TimeFrequencyData::CreateTFDataFromComplexCombination(artifacts.OriginalData(), artifacts.OriginalData());
						newOriginalData->MultiplyImages(1.0L/M_SQRT2);
						newOriginalData->SetMask(artifacts.OriginalData());
						artifacts.SetOriginalData(*newOriginalData);
						delete newOriginalData;
					}
				}
			}
			void SetRestoreOriginals(bool restoreOriginals)
			{
				_restoreOriginals = restoreOriginals;
			}
			bool RestoreOriginals() const
			{
				return _restoreOriginals;
			}
		private:
			bool _restoreOriginals;
	};

} // namespace

#endif
