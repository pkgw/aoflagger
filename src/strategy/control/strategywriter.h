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
#ifndef RFISTRATEGYWRITER_H
#define RFISTRATEGYWRITER_H

#include <set>
#include <sstream>
#include <string>
#include <stdexcept>

#include "../../util/xmlwriter.h"

#include "../actions/action.h"

namespace rfiStrategy {

	class StrategyWriterError : public std::runtime_error
	{
		public:
			StrategyWriterError(const std::string &arg) : std::runtime_error(arg) { }
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class StrategyWriter : private XmlWriter {
		public:
			StrategyWriter() : _writeDescriptions(false)
			{
			}
			~StrategyWriter()
			{
			}

			void WriteToFile(const class Strategy &strategy, const std::string &filename)
			{
				StartDocument(filename);
				write(strategy);
			}

			void WriteToStream(const class Strategy &strategy, std::ostream &stream)
			{
				StartDocument(stream);
				write(strategy);
			}

			void SetWriteComments(bool writeDescriptions)
			{
				_writeDescriptions = writeDescriptions;
			}
		private:
			void write(const class Strategy &strategy);

			void writeAction(const class Action &action);
			void writeContainerItems(const class ActionContainer &actionContainer);

			void writeAbsThresholdAction(const class AbsThresholdAction &action);
			void writeAdapter(const class Adapter &action);
			void writeAddStatisticsAction(const class AddStatisticsAction &action);
			void writeBaselineSelectionAction(const class BaselineSelectionAction &action);
			void writeChangeResolutionAction(const class ChangeResolutionAction &action);
			void writeCollectNoiseStatisticsAction(const class CollectNoiseStatisticsAction &action);
			void writeCombineFlagResults(const class CombineFlagResults &action);
			void writeCutAreaAction(const class CutAreaAction &action);
			void writeDirectionalCleanAction(const class DirectionalCleanAction &action);
			void writeDirectionProfileAction(const class DirectionProfileAction &action);
			void writeEigenValueVerticalAction(const class EigenValueVerticalAction &action);
			void writeForEachBaselineAction(const class ForEachBaselineAction &action);
			void writeForEachComplexComponentAction(const class ForEachComplexComponentAction &action);
			void writeForEachMSAction(const class ForEachMSAction &action);
			void writeForEachPolarisationBlock(const class ForEachPolarisationBlock &action);
			void writeFourierTransformAction(const class FourierTransformAction &action);
			void writeFrequencyConvolutionAction(const class FrequencyConvolutionAction &action);
			void writeFrequencySelectionAction(const class FrequencySelectionAction &action);
			void writeFringeStopAction(const class FringeStopAction &action);
			void writeHighPassFilterAction(const class HighPassFilterAction &action);
			void writeImagerAction(const class ImagerAction &action);
			void writeIterationBlock(const class IterationBlock &action);
			void writeNormalizeVarianceAction(const class NormalizeVarianceAction &action);
			void writePlotAction(const class PlotAction &action);
			void writeQuickCalibrateAction(const class QuickCalibrateAction &action);
			void writeRawAppenderAction(const class RawAppenderAction &action);
			void writeSetFlaggingAction(const class SetFlaggingAction &action);
			void writeSetImageAction(const class SetImageAction &action);
			void writeSlidingWindowFitAction(const class SlidingWindowFitAction &action);
			void writeStatisticalFlagAction(const class StatisticalFlagAction &action);
			void writeStrategy(const class Strategy &action);
			void writeSVDAction(const class SVDAction &action);
			void writeSumThresholdAction(const class SumThresholdAction &action);
			void writeTimeConvolutionAction(const class TimeConvolutionAction &action);
			void writeTimeSelectionAction(const class TimeSelectionAction &action);
			void writeUVProjectAction(const class UVProjectAction &action);
			void writeWriteDataAction(const class WriteDataAction &action);
			void writeWriteFlagsAction(const class WriteFlagsAction &action);

			std::string wrap(const std::string &input, size_t max) const;

			bool _writeDescriptions;
			std::set<enum ActionType> _describedActions;
	};

}

#endif
