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
#include "changeresolutionaction.h"

#include "../control/artifactset.h"

#include <stdexcept>
#include "../algorithms/thresholdtools.h"

namespace rfiStrategy {
	
	void ChangeResolutionAction::Perform(class ArtifactSet &artifacts, class ProgressListener &listener)
	{
		if(_timeDecreaseFactor != 1)
		{
			ArtifactSet artifactsCopy(artifacts);
			artifactsCopy.SetNoImageSet();
	
			TimeFrequencyData oldContaminated = artifacts.ContaminatedData();

			DecreaseTime(artifactsCopy.OriginalData());
			DecreaseTime(artifactsCopy.ContaminatedData());
			DecreaseTime(artifactsCopy.RevisedData());
	
			PerformFrequencyChange(artifactsCopy, listener);
	
			IncreaseTime(artifacts.OriginalData(), artifactsCopy.OriginalData(), false, false);
			IncreaseTime(artifacts.ContaminatedData(), artifactsCopy.ContaminatedData(), _restoreContaminated, _restoreMasks);
			IncreaseTime(artifacts.RevisedData(), artifactsCopy.RevisedData(), _restoreRevised, _restoreMasks);

			if(_restoreRevised && !_restoreContaminated)
			{
				//TimeFrequencyData *contaminatedData =
				//	TimeFrequencyData::CreateTFDataFromDiff(oldContaminated, artifacts.RevisedData());
				//contaminatedData->SetMask(oldContaminated);
				//artifacts.SetContaminatedData(*contaminatedData);
				//delete contaminatedData;
				
				oldContaminated.Subtract(artifacts.RevisedData());
				artifacts.SetContaminatedData(oldContaminated);
			}
		} else {
			PerformFrequencyChange(artifacts, listener);
		}
	}

	void ChangeResolutionAction::PerformFrequencyChange(class ArtifactSet &artifacts, class ProgressListener &listener)
	{
		if(_frequencyDecreaseFactor != 1)
		{
			ArtifactSet artifactsCopy(artifacts);
			artifactsCopy.SetNoImageSet();
	
			TimeFrequencyData oldContaminated = artifacts.ContaminatedData();

			DecreaseFrequency(artifactsCopy.OriginalData());
			DecreaseFrequency(artifactsCopy.ContaminatedData());
			DecreaseFrequency(artifactsCopy.RevisedData());
	
			ActionBlock::Perform(artifactsCopy, listener);
	
			IncreaseFrequency(artifacts.OriginalData(), artifactsCopy.OriginalData(), false, false);
			IncreaseFrequency(artifacts.ContaminatedData(), artifactsCopy.ContaminatedData(), _restoreContaminated, _restoreMasks);
			IncreaseFrequency(artifacts.RevisedData(), artifactsCopy.RevisedData(), _restoreRevised, _restoreMasks);

			if(_restoreRevised)
			{
				TimeFrequencyData *contaminatedData =
					TimeFrequencyData::CreateTFDataFromDiff(oldContaminated, artifacts.RevisedData());
				contaminatedData->SetMask(oldContaminated);
				artifacts.SetContaminatedData(*contaminatedData);
				delete contaminatedData;
			}
		} else {
			ActionBlock::Perform(artifacts, listener);
		}
	}

	void ChangeResolutionAction::DecreaseTime(TimeFrequencyData &timeFrequencyData)
	{
		if(_useMaskInAveraging)
		{
			DecreaseTimeWithMask(timeFrequencyData);
		}
		else {
			size_t imageCount = timeFrequencyData.ImageCount();
			for(size_t i=0;i<imageCount;++i)
			{
				Image2DCPtr image = timeFrequencyData.GetImage(i);
				Image2DPtr newImage = image->ShrinkHorizontally(_timeDecreaseFactor);
				timeFrequencyData.SetImage(i, newImage);
			}
			size_t maskCount = timeFrequencyData.MaskCount();
			for(size_t i=0;i<maskCount;++i)
			{
				Mask2DCPtr mask = timeFrequencyData.GetMask(i);
				Mask2DPtr newMask = mask->ShrinkHorizontally(_timeDecreaseFactor);
				timeFrequencyData.SetMask(i, newMask);
			}
		}
	}
	
	void ChangeResolutionAction::DecreaseTimeWithMask(TimeFrequencyData &data)
	{
		size_t polCount = data.PolarisationCount();
		for(size_t i=0;i<polCount;++i)
		{
			TimeFrequencyData *polData = data.CreateTFDataFromPolarisationIndex(i);
			Mask2DCPtr mask = polData->GetSingleMask();
			for(unsigned j=0;j<polData->ImageCount();++j)
			{
				Image2DCPtr image = polData->GetImage(j);
				polData->SetImage(j, ThresholdTools::ShrinkHorizontally(_timeDecreaseFactor, image, mask));
			}
			delete polData;
		}
		size_t maskCount = data.MaskCount();
		for(size_t i=0;i<maskCount;++i)
		{
			Mask2DCPtr mask = data.GetMask(i);
			Mask2DPtr newMask = mask->ShrinkHorizontallyForAveraging(_timeDecreaseFactor);
			data.SetMask(i, newMask);
		}
	}

	void ChangeResolutionAction::DecreaseFrequency(TimeFrequencyData &timeFrequencyData)
	{
		size_t imageCount = timeFrequencyData.ImageCount();
		for(size_t i=0;i<imageCount;++i)
		{
			Image2DCPtr image = timeFrequencyData.GetImage(i);
			Image2DPtr newImage = image->ShrinkVertically(_frequencyDecreaseFactor);
			timeFrequencyData.SetImage(i, newImage);
		}
		size_t maskCount = timeFrequencyData.MaskCount();
		for(size_t i=0;i<maskCount;++i)
		{
			Mask2DCPtr mask = timeFrequencyData.GetMask(i);
			Mask2DPtr newMask = mask->ShrinkVertically(_frequencyDecreaseFactor);
			timeFrequencyData.SetMask(i, newMask);
		}
	}

	void ChangeResolutionAction::IncreaseTime(TimeFrequencyData &originalData, TimeFrequencyData &changedData, bool restoreImage, bool restoreMask)
	{
		if(restoreImage)
		{
			size_t imageCount = originalData.ImageCount();
			if(imageCount != changedData.ImageCount())
				throw std::runtime_error("When restoring resolution in change resolution action, original data and changed data do not have the same number of images");
			for(size_t i=0;i<imageCount;++i)
			{
				Image2DCPtr image = changedData.GetImage(i);
				Image2DPtr newImage = image->EnlargeHorizontally(_timeDecreaseFactor, originalData.ImageWidth());
				originalData.SetImage(i, newImage);
			}
		}
		if(restoreMask)
		{
			originalData.SetMask(changedData);
			size_t maskCount = originalData.MaskCount();
			for(size_t i=0;i<maskCount;++i)
			{
				Mask2DCPtr mask = changedData.GetMask(i);
				Mask2DPtr newMask = Mask2D::CreateUnsetMaskPtr(originalData.ImageWidth(), originalData.ImageHeight());
				newMask->EnlargeHorizontallyAndSet(mask, _timeDecreaseFactor);
				originalData.SetMask(i, newMask);
			}
		}
	}

	void ChangeResolutionAction::IncreaseFrequency(TimeFrequencyData &originalData, TimeFrequencyData &changedData, bool restoreImage, bool restoreMask)
	{
		if(restoreImage)
		{
			size_t imageCount = originalData.ImageCount();
			if(imageCount != changedData.ImageCount())
				throw std::runtime_error("When restoring resolution in change resolution action, original data and changed data do not have the same number of images");
			for(size_t i=0;i<imageCount;++i)
			{
				Image2DCPtr image = changedData.GetImage(i);
				Image2DPtr newImage = image->EnlargeVertically(_frequencyDecreaseFactor, originalData.ImageHeight());
				originalData.SetImage(i, newImage);
			}
		}
		if(restoreMask)
		{
			originalData.SetMask(changedData);
			size_t maskCount = originalData.MaskCount();
			for(size_t i=0;i<maskCount;++i)
			{
				Mask2DCPtr mask = changedData.GetMask(i);
				Mask2DPtr newMask = Mask2D::CreateUnsetMaskPtr(originalData.ImageWidth(), originalData.ImageHeight());
				newMask->EnlargeVerticallyAndSet(mask, _frequencyDecreaseFactor);
				originalData.SetMask(i, newMask);
			}
		}
	}

}
