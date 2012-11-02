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
#ifndef FOR_EACH_COMPLEX_COMPONENT_ACTION_H
#define FOR_EACH_COMPLEX_COMPONENT_ACTION_H

#include "action.h"

#include "../../util/progresslistener.h"

#include "../control/artifactset.h"
#include "../control/actionblock.h"

namespace rfiStrategy {

	class ForEachComplexComponentAction : public ActionBlock
	{
		public:
			ForEachComplexComponentAction() : ActionBlock(), _onAmplitude(false), _onPhase(false), _onReal(true), _onImaginary(true), _restoreFromAmplitude(false)
			{
			}
			virtual std::string Description()
			{
				if(IterationCount() == 1)
				{
					if(_onAmplitude) {
						if(_restoreFromAmplitude)
							return "On amplitude (restore)";
						else
							return "On amplitude";
					}
					if(_onPhase) return "On phase";
					if(_onReal) return "On real";
					if(_onImaginary) return "On imaginary";
				}
				return "For each complex component";
			}
			virtual ActionType Type() const { return ForEachComplexComponentActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				size_t taskCount = 0;
				if(_onAmplitude) ++taskCount;
				if(_onPhase) ++taskCount;
				if(_onReal) ++taskCount;
				if(_onImaginary) ++taskCount;
				
				size_t taskIndex = 0;
				
				if(_onAmplitude) {
					listener.OnStartTask(*this, taskIndex, taskCount, "On amplitude");
					performOnAmplitude(artifacts, listener);
					listener.OnEndTask(*this);
					++taskIndex;
				}
				if(_onPhase) {
					listener.OnStartTask(*this, taskIndex, taskCount, "On phase");
					performOnPhaseRepresentation(artifacts, listener, TimeFrequencyData::PhasePart);
					listener.OnEndTask(*this);
					++taskIndex;
				}
				if(_onReal) {
					listener.OnStartTask(*this, taskIndex, taskCount, "On real");
					performOnPhaseRepresentation(artifacts, listener, TimeFrequencyData::RealPart);
					listener.OnEndTask(*this);
					++taskIndex;
				}
				if(_onImaginary) {
					listener.OnStartTask(*this, taskIndex, taskCount, "On imaginary");
					performOnPhaseRepresentation(artifacts, listener, TimeFrequencyData::ImaginaryPart);
					listener.OnEndTask(*this);
					++taskIndex;
				}
			}
			unsigned IterationCount() const {
				unsigned count = 0;
				if(_onAmplitude) ++count;
				if(_onPhase) ++count;
				if(_onReal) ++count;
				if(_onImaginary) ++count;
				return count;
			}
			void SetRestoreFromAmplitude(bool restoreFromAmplitude)
			{
				_restoreFromAmplitude = restoreFromAmplitude;
			}
			bool RestoreFromAmplitude() const
			{
				return _restoreFromAmplitude;
			}
			void SetOnAmplitude(bool onAmplitude)
			{
				_onAmplitude = onAmplitude;
			}
			bool OnAmplitude() const
			{
				return _onAmplitude;
			}
			void SetOnPhase(bool onPhase)
			{
				_onPhase = onPhase;
			}
			bool OnPhase() const
			{
				return _onPhase;
			}
			void SetOnReal(bool onReal)
			{
				_onReal = onReal;
			}
			bool OnReal() const
			{
				return _onReal;
			}
			void SetOnImaginary(bool onImaginary)
			{
				_onImaginary = onImaginary;
			}
			bool OnImaginary() const
			{
				return _onImaginary;
			}
		private:
			void performOnAmplitude(ArtifactSet &artifacts, class ProgressListener &listener)
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

				if(_restoreFromAmplitude)
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
			
			void performOnPhaseRepresentation(ArtifactSet &artifacts, class ProgressListener &listener, enum TimeFrequencyData::PhaseRepresentation phaseRepresentation)
			{
				enum TimeFrequencyData::PhaseRepresentation
					contaminatedPhase = artifacts.ContaminatedData().PhaseRepresentation(),
					revisedPhase = artifacts.RevisedData().PhaseRepresentation(),
					originalPhase = artifacts.OriginalData().PhaseRepresentation();
					
				TimeFrequencyData
					prevContaminated = artifacts.ContaminatedData(),
					prevRevised = artifacts.RevisedData(),
					prevOriginal = artifacts.OriginalData();
					
				bool phaseRepresentationIsAvailable = false;

				if(contaminatedPhase == TimeFrequencyData::ComplexRepresentation || contaminatedPhase == phaseRepresentation)
				{
					TimeFrequencyData *newContaminatedData =
						artifacts.ContaminatedData().CreateTFData(phaseRepresentation);
					artifacts.SetContaminatedData(*newContaminatedData);
					delete newContaminatedData;
					phaseRepresentationIsAvailable = true;
				}
				if(revisedPhase == TimeFrequencyData::ComplexRepresentation || revisedPhase == phaseRepresentation)
				{
					TimeFrequencyData *newRevisedData =
						artifacts.RevisedData().CreateTFData(phaseRepresentation);
					artifacts.SetRevisedData(*newRevisedData);
					delete newRevisedData;
					phaseRepresentationIsAvailable = true;
				}
				if(originalPhase == TimeFrequencyData::ComplexRepresentation || originalPhase == phaseRepresentation)
				{
					TimeFrequencyData *newOriginalData =
						artifacts.OriginalData().CreateTFData(phaseRepresentation);
					artifacts.SetOriginalData(*newOriginalData);
					delete newOriginalData;
					phaseRepresentationIsAvailable = true;
				}

				if(phaseRepresentationIsAvailable)
				{
					ActionBlock::Perform(artifacts, listener);

					if(phaseRepresentation != TimeFrequencyData::PhasePart)
					{
						if(contaminatedPhase == TimeFrequencyData::ComplexRepresentation)
							setPart(artifacts.ContaminatedData(), prevContaminated);
						if(revisedPhase == TimeFrequencyData::ComplexRepresentation)
							setPart(artifacts.RevisedData(), prevRevised);
						if(originalPhase == TimeFrequencyData::ComplexRepresentation)
							setPart(artifacts.OriginalData(), prevOriginal);
					}
				}
			}
			void setPart(TimeFrequencyData &changedData, TimeFrequencyData &prevData)
			{
				TimeFrequencyData *newData, *otherPart;
				switch(changedData.PhaseRepresentation())
				{
					default:
					case TimeFrequencyData::RealPart:
						otherPart = prevData.CreateTFData(TimeFrequencyData::ImaginaryPart);
						newData = TimeFrequencyData::CreateTFDataFromComplexCombination(changedData, *otherPart);
						break;
					case TimeFrequencyData::ImaginaryPart:
						otherPart = prevData.CreateTFData(TimeFrequencyData::RealPart);
						newData = TimeFrequencyData::CreateTFDataFromComplexCombination(*otherPart, changedData);
						break;
				}
				changedData = *newData;
				changedData.SetMask(prevData);
				delete newData;
				delete otherPart;
			}
			
			bool _onAmplitude, _onPhase, _onReal, _onImaginary;
			bool _restoreFromAmplitude;
		};

} // namespace

#endif // FOR_EACH_COMPLEX_COMPONENT_ACTION_H
