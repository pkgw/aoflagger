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

#ifndef COLLECTNOISESTATISTICSACTION_H
#define COLLECTNOISESTATISTICSACTION_H

#include <string>

#include <boost/thread/mutex.hpp>

#include "../control/artifactset.h"

#include "../../util/progresslistener.h"

#include "../algorithms/noisestatisticscollector.h"

namespace rfiStrategy {

	class CollectNoiseStatisticsAction : public Action
	{
		public:
			CollectNoiseStatisticsAction() : _filename("noise-statistics")
			{
			}
			
			virtual ~CollectNoiseStatisticsAction()
			{
				_statistics.SaveTF(_filename + "-tf.txt");
				_statistics.SaveTA(_filename + "-ta.txt");
			}

			virtual std::string Description()
			{
				return "Collect noise statistics";
			}
			
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				if(data.PolarisationCount() != 1)
					throw std::runtime_error("The noise collector action needs a single polarization");
				boost::mutex::scoped_lock lock(_mutex);
				if(data.PhaseRepresentation() == TimeFrequencyData::ComplexRepresentation)
					_statistics.Add(data.GetRealPart(), data.GetImaginaryPart(), artifacts.OriginalData().GetSingleMask(), artifacts.MetaData());
				else
					_statistics.Add(data.GetSingleImage(), data.GetSingleImage(), artifacts.OriginalData().GetSingleMask(), artifacts.MetaData());
			}
			virtual ActionType Type() const { return CollectNoiseStatisticsActionType; }
			
			void SetFilename(const std::string &filename) { _filename = filename; }
			const std::string &Filename() const { return _filename; }
			
			unsigned ChannelDistance() const { return _statistics.ChannelDistance(); }
			void SetChannelDistance(unsigned channelDistance)
			{
				_statistics.SetChannelDistance(channelDistance);
			}
			
			unsigned TileWidth() const { return _statistics.TileWidth(); }
			void SetTileWidth(unsigned tileWidth)
			{
				_statistics.SetTileWidth(tileWidth);
			}

			unsigned TileHeight() const { return _statistics.TileHeight(); }
			void SetTileHeight(unsigned tileHeight)
			{
				_statistics.SetTileHeight(tileHeight);
			}
		private:
			boost::mutex _mutex;
			NoiseStatisticsCollector _statistics;
			std::string _filename;
	};
}

#endif // COLLECTNOISESTATISTICSACTION_H
