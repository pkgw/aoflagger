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

#ifndef PLOTACTION_H
#define PLOTACTION_H 

#include "action.h"

namespace rfiStrategy {

	class PlotAction : public Action
	{
		public:
			enum PlotKind {
				AntennaFlagCountPlot, FrequencyFlagCountPlot, FrequencyPowerPlot, TimeFlagCountPlot,
				BaselineSpectrumPlot, PolarizationStatisticsPlot, BaselineRMSPlot, IterationsPlot
			};

			PlotAction() : _plotKind(FrequencyPowerPlot), _logYAxis(false) { }
			virtual ~PlotAction() { }
			virtual std::string Description()
			{
				switch(_plotKind)
				{
					case AntennaFlagCountPlot:
					return "Plot antenna flag counts";
					case FrequencyFlagCountPlot:
					return "Plot frequency flag counts";
					case FrequencyPowerPlot:
					return "Plot frequency power";
					case TimeFlagCountPlot:
					return "Plot time flag counts";
					case BaselineSpectrumPlot:
					return "Plot spectrum per baseline";
					case PolarizationStatisticsPlot:
					return "Plot polarization flag counts";
					case BaselineRMSPlot:
					return "Plot baseline RMS";
					case IterationsPlot:
					return "Plot iteration convergence";
					default:
					return "Unknown plot action";
				}
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);
			virtual ActionType Type() const { return PlotActionType; }

			enum PlotKind PlotKind() const { return _plotKind; }
			void SetPlotKind(enum PlotKind plotKind) { _plotKind = plotKind; }

			bool LogarithmicYAxis() const { return _logYAxis; }
			void SetLogarithmicYAxis(bool logYAxis) { _logYAxis = logYAxis; }
		private:
			enum PlotKind _plotKind;
			bool _logYAxis;
			boost::mutex _plotMutex;

			void plotAntennaFlagCounts(class ArtifactSet &artifacts);
			void plotFrequencyFlagCounts(class ArtifactSet &artifacts);
			void plotFrequencyPower(class ArtifactSet &artifacts);
			void plotTimeFlagCounts(class ArtifactSet &artifacts);
			void plotSpectrumPerBaseline(class ArtifactSet &artifacts);
			void plotPolarizationFlagCounts(class ArtifactSet &artifacts);
			void plotBaselineRMS(class ArtifactSet &artifacts);
			void plotIterations(class ArtifactSet &artifacts);
	};

}

#endif // PLOTACTION_H

