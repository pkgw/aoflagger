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

#include "baselineselectionaction.h"

#include <iostream>

#include "../../util/plot.h"

#include "../../msio/mask2d.h"
#include "../../msio/baselinereader.h"

#include "strategyaction.h"

#include "../control/artifactset.h"

#include "../imagesets/msimageset.h"

namespace rfiStrategy {
	
	void BaselineSelectionAction::prepare(class ArtifactSet &artifacts, class ProgressListener &)
	{
		if(artifacts.BaselineSelectionInfo() == 0)
			throw BadUsageException("ArtifactSet does not have baseline selection info");
		if(artifacts.MetaData() == 0)
		{
			AOLogger::Warn << "BaselineSelectionAction is used, but ArtifactSet does not have meta data\n";
			return;
		}
		if(!artifacts.MetaData()->HasBaseline())
		{
			AOLogger::Warn << "BaselineSelectionAction is used, but ArtifactSet does not have baseline meta data\n";
			return;
		}

		Mask2DCPtr mask = artifacts.ContaminatedData().GetSingleMask();

		BaselineSelector &info = *artifacts.BaselineSelectionInfo();
		boost::mutex::scoped_lock lock(info.Mutex());
		info.Add(mask, artifacts.MetaData());
	}
	
	void BaselineSelectionAction::mark(class ArtifactSet &artifacts, class ProgressListener &)
	{
		if(artifacts.BaselineSelectionInfo() == 0)
			throw BadUsageException("ArtifactSet does not have baseline selection info");
		BaselineSelector &info = *artifacts.BaselineSelectionInfo();
		if(info.BaselineCount() == 0)
		{
			AOLogger::Warn <<
			"BaselineSelectionAction wrongly used: trying to mark baselines, but baselines have\n"
			"not been prepared previously (you need to add a BaselineSelectionAction within a for\n"
			"each baseline block, that calculates the statistics and prepares selection)\n";
		} else {

			AOLogger::Debug << "Searching for bad baselines...\n";

			Strategy::SyncAll(*GetRoot());

			boost::mutex::scoped_lock lock(info.Mutex());

			BaselineSelector &selector = *artifacts.BaselineSelectionInfo();
			selector.SetAbsThreshold(_absThreshold);
			selector.SetSmoothingSigma(_smoothingSigma);
			selector.SetThreshold(_threshold);
			
			std::vector<BaselineSelector::SingleBaselineInfo> markedBaselines;
			selector.Search(markedBaselines);
			
			if(markedBaselines.size() > 0)
			{
				AOLogger::Info << "Found " << markedBaselines.size() << "/" << (markedBaselines.size()+selector.BaselineCount()) << " bad baselines: ";
				
				std::vector<BaselineSelector::SingleBaselineInfo>::const_iterator badBaselineIter = markedBaselines.begin();
				AOLogger::Info << badBaselineIter->antenna1Name << "x" << badBaselineIter->antenna2Name;
				++badBaselineIter;
				while(badBaselineIter!=markedBaselines.end())
				{
					AOLogger::Info << ", " << badBaselineIter->antenna1Name << "x" << badBaselineIter->antenna2Name;
					++badBaselineIter;
				}
				AOLogger::Info << '\n';
			} else {
				AOLogger::Info << "No bad baselines found.\n";
			}
			
			if(_flagBadBaselines)
			{
				flagBaselines(artifacts, markedBaselines);
			} else {
				if(markedBaselines.size() > 0)
					AOLogger::Info <<
						"Bad baseline finding is still experimental, please check the results.\n"
						"These baselines have therefore NOT been flagged yet. Writing flags to\n"
						"these baselines can be enabled by setting the flag-bad-baselines\n"
						"property of both BaselineSelectionAction's to '1' in your strategy\n"
						"file.\n";
			}
		}
	}

	void BaselineSelectionAction::flagBaselines(ArtifactSet &artifacts, const std::vector<BaselineSelector::SingleBaselineInfo> &baselines)
	{
		boost::mutex::scoped_lock lock(artifacts.IOMutex());

		ImageSet *imageSet = artifacts.ImageSet();
		BaselineReaderPtr reader = dynamic_cast<MSImageSet&>(*imageSet).Reader();

		size_t scans = reader->Set().GetObservationTimesSet().size();

		for(std::vector<BaselineSelector::SingleBaselineInfo>::const_iterator i=baselines.begin();
			i!=baselines.end();++i)
		{
			size_t frequencyCount = reader->Set().FrequencyCount(i->band);
			Mask2DPtr flaggedMask = Mask2D::CreateSetMaskPtr<true>(scans, frequencyCount);
			std::vector<Mask2DCPtr> masks;
			for(size_t p=0;p<reader->PolarizationCount();++p)
				masks.push_back(flaggedMask);
			reader->AddWriteTask(masks, i->antenna1, i->antenna2, i->band, i->sequenceId);
		}
		reader->PerformFlagWriteRequests();
	}
}
