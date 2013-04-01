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
#ifndef FOREACHMSACTION_H
#define FOREACHMSACTION_H

#include "../control/actionblock.h"

#include "../../msio/types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
namespace rfiStrategy {

	class ForEachMSAction  : public ActionBlock {
		public:
			ForEachMSAction() : _readUVW(false), _dataColumnName("DATA"), _subtractModel(false), _skipIfAlreadyProcessed(false), _loadOptimizedStrategy(false), _baselineIOMode(AutoReadMode),
			_threadCount(0)
			{
			}
			~ForEachMSAction()
			{
			}
			virtual std::string Description()
			{
				return "For each measurement set";
			}
			virtual void Initialize();
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress);
			virtual ActionType Type() const { return ForEachMSActionType; }
			virtual unsigned int Weight() const { return ActionBlock::Weight() * _filenames.size(); }
			void AddDirectory(const std::string &name);
			void writeHistory(const std::string &filename);

			std::vector<std::string> &Filenames() { return _filenames; }
			const std::vector<std::string> &Filenames() const { return _filenames; }

			BaselineIOMode IOMode() const { return _baselineIOMode; }
			void SetIOMode(BaselineIOMode ioMode) { _baselineIOMode = ioMode; }

			bool ReadUVW() const { return _readUVW; }
			void SetReadUVW(bool readUVW) { _readUVW = readUVW; }

			const std::string &DataColumnName() const { return _dataColumnName; }
			void SetDataColumnName(const std::string &name) { _dataColumnName = name; }

			bool SubtractModel() const { return _subtractModel; }
			void SetSubtractModel(bool subtractModel) { _subtractModel = subtractModel; }

			std::string CommandLineForHistory() const { return _commandLineForHistory; }
			void SetCommandLineForHistory(const std::string cmd) { _commandLineForHistory = cmd; }
			
			bool SkipIfAlreadyProcessed() const { return _skipIfAlreadyProcessed; }
			void SetSkipIfAlreadyProcessed(bool value) { _skipIfAlreadyProcessed = value; }
			
			bool LoadOptimizedStrategy() const { return _loadOptimizedStrategy; }
			void SetLoadOptimizedStrategy(bool value) { _loadOptimizedStrategy = value; }
			
			size_t LoadStrategyThreadCount() const { return _threadCount; }
			void SetLoadStrategyThreadCount(size_t threadCount) { _threadCount = threadCount; }
		private:
			std::vector<std::string> _filenames;
			bool _readUVW;
			std::string _dataColumnName;
			bool _subtractModel;
			std::string _commandLineForHistory;
			bool _skipIfAlreadyProcessed;
			bool _loadOptimizedStrategy;
			BaselineIOMode _baselineIOMode;
			size_t _threadCount;
	};

}

#endif
