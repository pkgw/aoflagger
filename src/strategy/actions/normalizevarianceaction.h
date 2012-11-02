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
#ifndef RFISTRATEGYNORMALIZEVARIANCEACTION_H
#define RFISTRATEGYNORMALIZEVARIANCEACTION_H

#include <map>

#include <boost/thread/mutex.hpp>

#include "action.h"

#include "../control/artifactset.h"

namespace rfiStrategy {

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class NormalizeVarianceAction : public Action {
		public:
			NormalizeVarianceAction() :
				_medianFilterSizeInS(60.0*15.0), _isInitialized(false)
			{
			}
			virtual ~NormalizeVarianceAction();
			virtual std::string Description()
			{
				return "Normalize variance over time";
			}
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress);

			virtual void Initialize()
			{
				clean();
			}
			virtual void Finish()
			{
				clean();
			}
			virtual ActionType Type() const { return NormalizeVarianceActionType; }

			double MedianFilterSizeInS() const { return _medianFilterSizeInS; }
			void SetMedianFilterSizeInS(double filterSize) { _medianFilterSizeInS = filterSize; }
		private:
			double _medianFilterSizeInS;
			boost::mutex _mutex;
			bool _isInitialized;
			std::map<double, double> _stddevs;
			
			void initializeStdDevs(ArtifactSet &artifacts);
			void clean();
			void correctData(std::vector<Image2DPtr> &data, size_t timeStep, double stddev);
			void correctDataUpTo(std::vector<Image2DPtr> &data, size_t &dataTimeIndex, double time, const std::vector<double> &observationTimes, double stddev);
	};
}

#endif
