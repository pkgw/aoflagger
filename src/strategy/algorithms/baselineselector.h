/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#ifndef BASELINE_SELECTOR_H
#define BASELINE_SELECTOR_H

#include <string>
#include <set>
#include <vector>

#include <boost/thread/mutex.hpp>

#include "../../msio/timefrequencymetadata.h"
#include "../../msio/mask2d.h"

class DefaultStatistics;

namespace rfiStrategy {

	class BaselineSelector
	{
		public:
			struct SingleBaselineInfo
			{
				SingleBaselineInfo() : marked(false) { }

				SingleBaselineInfo(const SingleBaselineInfo &source) :
					antenna1(source.antenna1),
					antenna2(source.antenna2),
					antenna1Name(source.antenna1Name),
					antenna2Name(source.antenna2Name),
					band(source.band),
					length(source.length),
					rfiCount(source.rfiCount),
					totalCount(source.totalCount),
					marked(source.marked)
				{
				}
				void operator=(const SingleBaselineInfo &source)
				{
					antenna1 = source.antenna1;
					antenna2 = source.antenna2;
					antenna1Name = source.antenna1Name;
					antenna2Name = source.antenna2Name;
					band = source.band;
					length = source.length;
					rfiCount = source.rfiCount;
					totalCount = source.totalCount;
					marked = source.marked;
				}
				bool operator<(const SingleBaselineInfo &rhs) const
				{
					return length < rhs.length;
				}
					
				int antenna1, antenna2;
				std::string antenna1Name, antenna2Name;
				int band;
				double length;
				unsigned long rfiCount, totalCount;
				bool marked;
			};
			
			BaselineSelector() :
			_threshold(8.0), _absThreshold(0.4), _smoothingSigma(0.6),
			_makePlot(false), _useLog(true)
			{
			}

			typedef std::vector<SingleBaselineInfo> BaselineVector;
			void Search(std::vector<BaselineSelector::SingleBaselineInfo> &markedBaselines);
			void ImplyStations(const std::vector<BaselineSelector::SingleBaselineInfo> &markedBaselines, double maxRatio, std::set<unsigned> &badStations) const;
			void Add(Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData);
			void Add(class DefaultStatistics &baselineStat, class AntennaInfo &antenna1, class AntennaInfo &antenna2);
			
			boost::mutex &Mutex() { return _mutex; }
			
			double Threshold() const { return _threshold; }
			double AbsThreshold() const { return _absThreshold; }
			
			void SetThreshold(double threshold) { _threshold = threshold; }
			void SetAbsThreshold(double absThreshold) { _absThreshold = absThreshold; }
			void SetSmoothingSigma(double smoothingSigma) { _smoothingSigma = smoothingSigma; }
			void SetUseLog(bool useLog) { _useLog = useLog; }
			
			size_t BaselineCount() const { return _baselines.size(); }
		private:
			boost::mutex _mutex;
			BaselineVector _baselines;
			double _threshold, _absThreshold;
			double _smoothingSigma;
			bool _makePlot;
			bool _useLog;
			
			double smoothedValue(double length) const;
			double smoothedValue(const BaselineSelector::SingleBaselineInfo &baseline) const
			{
				return smoothedValue(baseline.length);
			}

	};

} // end of namespace

#endif
