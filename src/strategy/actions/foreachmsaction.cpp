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
#include "foreachmsaction.h"

#include <boost/filesystem.hpp>

#include "../../msio/measurementset.h"

#include "strategyaction.h"

#include "../control/artifactset.h"
#include "../control/defaultstrategy.h"

#include "../imagesets/imageset.h"
#include "../imagesets/msimageset.h"

#include "../../util/aologger.h"
#include "../../util/progresslistener.h"

namespace rfiStrategy {

void ForEachMSAction::Initialize()
{
}
	
void ForEachMSAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
{
	unsigned taskIndex = 0;
	
	FinishAll();

	for(std::vector<std::string>::const_iterator i=_filenames.begin();i!=_filenames.end();++i)
	{
		std::string filename = *i;
		
		progress.OnStartTask(*this, taskIndex, _filenames.size(), std::string("Processing measurement set ") + filename);
		
		bool skip = false;
		if(_skipIfAlreadyProcessed)
		{
			MeasurementSet set(filename);
			if(set.HasRFIConsoleHistory())
			{
				skip = true;
				AOLogger::Info << "Skipping " << filename << ",\n"
					"because the set contains AOFlagger history and -skip-flagged was given.\n";
			}
		}
		
		if(!skip)
		{
			ImageSet *imageSet = ImageSet::Create(filename, _baselineIOMode, _readUVW);
			bool isMS = dynamic_cast<MSImageSet*>(imageSet) != 0;
			if(isMS)
			{ 
				MSImageSet *msImageSet = static_cast<MSImageSet*>(imageSet);
				msImageSet->SetDataColumnName(_dataColumnName);
				msImageSet->SetSubtractModel(_subtractModel);
			}
			imageSet->Initialize();
			
			if(_loadOptimizedStrategy)
			{
				rfiStrategy::DefaultStrategy::TelescopeId telescopeId;
				unsigned flags;
				double frequency, timeResolution, frequencyResolution;
				rfiStrategy::DefaultStrategy::DetermineSettings(*imageSet, telescopeId, flags, frequency, timeResolution, frequencyResolution);
				RemoveAll();
				rfiStrategy::DefaultStrategy::LoadFullStrategy(
					*this,
					telescopeId,
					flags,
					frequency,
					timeResolution,
					frequencyResolution
				);
				
				if(_threadCount != 0)
					rfiStrategy::Strategy::SetThreadCount(*this, _threadCount);
			}
			
			ImageSetIndex *index = imageSet->StartIndex();
			artifacts.SetImageSet(imageSet);
			artifacts.SetImageSetIndex(index);

			InitializeAll();
			
			ActionBlock::Perform(artifacts, progress);
			
			FinishAll();

			artifacts.SetNoImageSet();
			delete index;
			delete imageSet;

			if(isMS)
				writeHistory(*i);
		}
	
		progress.OnEndTask(*this);

		
		++taskIndex;
	}

	InitializeAll();
}

void ForEachMSAction::AddDirectory(const std::string &name)
{
  // get all files ending in .MS
  boost::filesystem::path dir_path(name);
  boost::filesystem::directory_iterator end_it;

  for(boost::filesystem::directory_iterator it(dir_path); it != end_it; ++it) {
    if( is_directory(it->status()) && extension(it->path()) == ".MS" ) {
      _filenames.push_back( it->path().string() );
    }
  }
}

void ForEachMSAction::writeHistory(const std::string &filename)
{
	if(GetChildCount() != 0)
	{
		MeasurementSet ms(filename);
		const Strategy *strategy = 0;
		if(GetChildCount() == 1 && dynamic_cast<const Strategy*>(&GetChild(0)) != 0)
		{
			strategy = static_cast<const Strategy*>(&GetChild(0));
		} else {
			const ActionContainer *root = GetRoot();
			if(dynamic_cast<const Strategy*>(root) != 0)
				strategy = static_cast<const Strategy*>(root);
		}
		AOLogger::Debug << "Adding strategy to history table of MS...\n";
		if(strategy != 0) {
			try {
				ms.AddAOFlaggerHistory(*strategy, _commandLineForHistory);
			} catch(std::exception &e)
			{
				AOLogger::Warn << "Failed to write history to MS: " << e.what() << '\n';
			}
		}
		else
			AOLogger::Error << "Could not find root strategy to write to Measurement Set history table!\n";
	}
}

}
