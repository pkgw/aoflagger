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
#include "foreachbaselineaction.h"

#include "../../msio/antennainfo.h"

#include "../../util/aologger.h"
#include "../../util/stopwatch.h"

#include <iostream>
#include <sstream>

#include <boost/thread.hpp>

#include "../imagesets/fitsimageset.h"
#include "../imagesets/imageset.h"
#include "../imagesets/msimageset.h"

namespace rfiStrategy {
	
	void ForEachBaselineAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
	{
		if(!artifacts.HasImageSet())
		{
			progress.OnStartTask(*this, 0, 1, "For each baseline (no image set)");
			progress.OnEndTask(*this);
			AOLogger::Warn <<
				"I executed a ForEachBaselineAction without an active imageset: something is\n"
				"likely wrong. Check your strategy and the input files.\n";
		} else if(_selection == Current)
		{
			ActionBlock::Perform(artifacts, progress);
		} else
		{
			ImageSet *imageSet = artifacts.ImageSet();
			MSImageSet *msImageSet = dynamic_cast<MSImageSet*>(imageSet);
			if(msImageSet != 0)
			{
				// Check memory usage
				ImageSetIndex *tempIndex = msImageSet->StartIndex();
				size_t timeStepCount = msImageSet->ObservationTimesVector(*tempIndex).size();
				delete tempIndex;
				size_t channelCount = msImageSet->GetBandInfo(0).channels.size();
				size_t estMemorySizePerThread = 8/*bp complex*/ * 4 /*polarizations*/ * timeStepCount * channelCount * 3 /* approx copies of the data that will be made in memory*/;
				AOLogger::Debug << "Estimate of memory each thread will use: " << estMemorySizePerThread/(1024*1024) << " MB.\n";
				size_t compThreadCount = _threadCount;
				if(compThreadCount > 0) --compThreadCount;
				if(estMemorySizePerThread * compThreadCount > 12ul*1024ul*1024ul*1024ul)
				{
					size_t maxThreads = (12ul * 1024ul * 1024ul * 1024ul) / estMemorySizePerThread;
					if(maxThreads < 1) maxThreads = 1;
					AOLogger::Warn <<
						"WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING!\n"
						"This measurement set is TOO LARGE to be processed with " << _threadCount << " threads!\n" <<
						_threadCount << " threads would require " << ((estMemorySizePerThread*compThreadCount)/(1024*1024)) << " MB of memory approximately.\n"
						"Number of threads that will actually be used: " << maxThreads << "\n"
						"This might hurt performance a lot!\n\n";
					_threadCount = maxThreads;
				}
			}
			if(!_antennaeToSkip.empty())
			{
				AOLogger::Debug << "The following antenna's will be skipped: ";
				for(std::set<size_t>::const_iterator i=_antennaeToSkip.begin();i!=_antennaeToSkip.end(); ++i)
					AOLogger::Debug << (*i) << ' ';
				AOLogger::Debug <<'\n';
			}
			if(!_antennaeToInclude.empty())
			{
				AOLogger::Debug << "Only the following antenna's will be included: ";
				for(std::set<size_t>::const_iterator i=_antennaeToInclude.begin();i!=_antennaeToInclude.end(); ++i)
					AOLogger::Debug << (*i) << ' ';
				AOLogger::Debug <<'\n';
			}

			if(artifacts.MetaData() != 0)
			{
				_hasInitAntennae = true;
				if(artifacts.MetaData()->HasAntenna1())
					_initAntenna1 = artifacts.MetaData()->Antenna1();
				else
					_hasInitAntennae = false;
				if(artifacts.MetaData()->HasAntenna2())
					_initAntenna2 = artifacts.MetaData()->Antenna2();
				else
					_hasInitAntennae = false;
			}
			_artifacts = &artifacts;
			
			_initPartIndex = 0;
			_finishedBaselines = false;
			_baselineCount = 0;
			_baselineProgress = 0;
			_nextIndex = 0;
			
			// Count the baselines that are to be processed
			ImageSetIndex *iteratorIndex = imageSet->StartIndex();
			while(iteratorIndex->IsValid())
			{
				if(IsBaselineSelected(*iteratorIndex))
					++_baselineCount;
				iteratorIndex->Next();
			}
			delete iteratorIndex;
			AOLogger::Debug << "Will process " << _baselineCount << " baselines.\n";
			
			// Initialize thread data and threads
			_loopIndex = imageSet->StartIndex();
			_progressTaskNo = new int[_threadCount];
			_progressTaskCount = new int[_threadCount];
			progress.OnStartTask(*this, 0, 1, "Initializing");

			boost::thread_group threadGroup;
			ReaderFunction reader(*this);
			threadGroup.create_thread(reader);
			
			size_t mathThreads = mathThreadCount();
			for(unsigned i=0;i<mathThreads;++i)
			{
				PerformFunction function(*this, progress, i);
				threadGroup.create_thread(function);
			}
			
			threadGroup.join_all();
			progress.OnEndTask(*this);

			if(_resultSet != 0)
			{
				artifacts = *_resultSet;
				delete _resultSet;
			}

			delete[] _progressTaskCount;
			delete[] _progressTaskNo;

			delete _loopIndex;

			if(_exceptionOccured)
				throw std::runtime_error("An exception occured in one of the sub tasks of the (multi-threaded) \"For-each baseline\"-action: the RFI strategy will not continue.");
		}
	}

	bool ForEachBaselineAction::IsBaselineSelected(ImageSetIndex &index)
	{
		ImageSet *imageSet = _artifacts->ImageSet();
		MSImageSet *msImageSet = dynamic_cast<MSImageSet*>(imageSet);
		size_t a1id, a2id;
		if(msImageSet != 0)
		{
			a1id = msImageSet->GetAntenna1(index);
			a2id = msImageSet->GetAntenna2(index);
		} else {
			a1id = 0;
			a2id = 0;
		}
		if(_antennaeToSkip.count(a1id) != 0 || _antennaeToSkip.count(a2id) != 0)
			return false;
		if(!_antennaeToInclude.empty() && (_antennaeToInclude.count(a1id) == 0 && _antennaeToInclude.count(a2id) == 0))
			return false;
		
		// For SDFits files, we want to select everything -- it's confusing
		// if the default option "only flag cross correlations" would also
		// hold for sdfits files.
		if(dynamic_cast<FitsImageSet*>(imageSet)!=0)
			return true;

		switch(_selection)
		{
			case All:
				return true;
			case CrossCorrelations:
			{
				return a1id != a2id;
			}
			case AutoCorrelations:
				return a1id == a2id;
			case EqualToCurrent: {
				if(!_hasInitAntennae)
					throw BadUsageException("For each baseline over 'EqualToCurrent' with no current baseline");
				throw BadUsageException("Not implemented");
			}
			case AutoCorrelationsOfCurrentAntennae:
				if(!_hasInitAntennae)
					throw BadUsageException("For each baseline over 'AutoCorrelationsOfCurrentAntennae' with no current baseline");
				return a1id == a2id && (_initAntenna1.id == a1id || _initAntenna2.id == a1id);
			default:
				return false;
		}
	}

	class ImageSetIndex *ForEachBaselineAction::GetNextIndex()
	{
		boost::mutex::scoped_lock lock(_mutex);
		while(_loopIndex->IsValid())
		{
			if(IsBaselineSelected(*_loopIndex))
			{
				ImageSetIndex *newIndex = _loopIndex->Copy();
				_loopIndex->Next();

				return newIndex;
			}
			_loopIndex->Next();
		}
		return 0;
	}

	void ForEachBaselineAction::SetExceptionOccured()
	{
		boost::mutex::scoped_lock lock(_mutex);
		_exceptionOccured = true;
	}
	
	void ForEachBaselineAction::SetFinishedBaselines()
	{
		boost::mutex::scoped_lock lock(_mutex);
		_finishedBaselines = true;
	}
	
	void ForEachBaselineAction::PerformFunction::operator()()
	{
		boost::mutex::scoped_lock ioLock(_action._artifacts->IOMutex());
		ImageSet *privateImageSet = _action._artifacts->ImageSet()->Copy();
		ioLock.unlock();

		try {
			boost::mutex::scoped_lock lock(_action._mutex);
			ArtifactSet newArtifacts(*_action._artifacts);
			lock.unlock();
			
			BaselineData *baseline = _action.GetNextBaseline();
			
			while(baseline != 0) {
				baseline->Index().Reattach(*privateImageSet);
				
				std::ostringstream progressStr;
				if(_action._hasInitAntennae)
					progressStr << "Processing baseline " << baseline->MetaData()->Antenna1().name << " x " << baseline->MetaData()->Antenna2().name;
				else
					progressStr << "Processing next baseline";
				_action.SetProgress(_progress, _action.BaselineProgress(), _action._baselineCount, progressStr.str(), _threadIndex);
	
				newArtifacts.SetOriginalData(baseline->Data());
				newArtifacts.SetContaminatedData(baseline->Data());
				TimeFrequencyData *zero = new TimeFrequencyData(baseline->Data());
				zero->SetImagesToZero();
				newArtifacts.SetRevisedData(*zero);
				delete zero;
				newArtifacts.SetImageSetIndex(&baseline->Index());
				newArtifacts.SetMetaData(baseline->MetaData());

				_action.ActionBlock::Perform(newArtifacts, *this);
				delete baseline;
	
				baseline = _action.GetNextBaseline();
				_action.IncBaselineProgress();
			}
	
			if(_threadIndex == 0)
				_action._resultSet = new ArtifactSet(newArtifacts);

		} catch(std::exception &e)
		{
			_progress.OnException(_action, e);
			_action.SetExceptionOccured();
		}

		delete privateImageSet;
	}

	void ForEachBaselineAction::PerformFunction::OnStartTask(const Action &/*action*/, size_t /*taskNo*/, size_t /*taskCount*/, const std::string &/*description*/, size_t /*weight*/)
	{
	}

	void ForEachBaselineAction::PerformFunction::OnEndTask(const Action &/*action*/)
	{
	}

	void ForEachBaselineAction::PerformFunction::OnProgress(const Action &/*action*/, size_t /*progres*/, size_t /*maxProgress*/)
	{
	}

	void ForEachBaselineAction::PerformFunction::OnException(const Action &action, std::exception &thrownException)
	{
		_progress.OnException(action, thrownException);
	}
	
	void ForEachBaselineAction::ReaderFunction::operator()()
	{
		Stopwatch watch(true);
		bool finished = false;
		size_t threadCount = _action.mathThreadCount();
		size_t minRecommendedBufferSize, maxRecommendedBufferSize;
		MSImageSet *msImageSet = dynamic_cast<MSImageSet*>(_action._artifacts->ImageSet());
		if(msImageSet != 0)
		{
			minRecommendedBufferSize = msImageSet->Reader()->GetMinRecommendedBufferSize(threadCount);
			maxRecommendedBufferSize = msImageSet->Reader()->GetMaxRecommendedBufferSize(threadCount) - _action.GetBaselinesInBufferCount();
		} else {
			minRecommendedBufferSize = 1;
			maxRecommendedBufferSize = 2;
		}
		
		do {
			watch.Pause();
			_action.WaitForBufferAvailable(minRecommendedBufferSize);
			
			size_t wantedCount = maxRecommendedBufferSize - _action.GetBaselinesInBufferCount();
			size_t requestedCount = 0;
			
			boost::mutex::scoped_lock lock(_action._artifacts->IOMutex());
			watch.Start();
			
			for(size_t i=0;i<wantedCount;++i)
			{
				ImageSetIndex *index = _action.GetNextIndex();
				if(index != 0)
				{
					_action._artifacts->ImageSet()->AddReadRequest(*index);
					++requestedCount;
					delete index;
				} else {
					finished = true;
					break;
				}
			}
			
			if(requestedCount > 0)
			{
				_action._artifacts->ImageSet()->PerformReadRequests();
				watch.Pause();
				
				for(size_t i=0;i<requestedCount;++i)
				{
					BaselineData *baseline = _action._artifacts->ImageSet()->GetNextRequested();
					
					boost::mutex::scoped_lock bufferLock(_action._mutex);
					_action._baselineBuffer.push(baseline);
					bufferLock.unlock();
				}
			}
			
			lock.unlock();
			
			_action._dataAvailable.notify_all();
			watch.Start();
		} while(!finished);
		_action.SetFinishedBaselines();
		_action._dataAvailable.notify_all();
		watch.Pause();
		AOLogger::Debug << "Time spent on reading: " << watch.ToString() << '\n';
	}

	void ForEachBaselineAction::SetProgress(ProgressListener &progress, int no, int count, std::string taskName, int threadId)
	{
	  boost::mutex::scoped_lock lock(_mutex);
		_progressTaskNo[threadId] = no;
		_progressTaskCount[threadId] = count;
		size_t totalCount = 0, totalNo = 0;
		for(size_t i=0;i<_threadCount;++i)
		{
			totalCount += _progressTaskCount[threadId];
			totalNo += _progressTaskNo[threadId];
		}
		progress.OnEndTask(*this);
		std::stringstream str;
		str << "T" << threadId << ": " << taskName;
		progress.OnStartTask(*this, totalNo, totalCount, str.str());
	}
}

