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
#ifndef INDIRECTBASELINEREADER_H
#define INDIRECTBASELINEREADER_H

#include <map>
#include <vector>
#include <stdexcept>

#include "baselinereader.h"
#include "directbaselinereader.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class IndirectBaselineReader : public BaselineReader {
	public:
		explicit IndirectBaselineReader(const std::string &msFile);
		~IndirectBaselineReader();

		virtual void PerformReadRequests();
		virtual void PerformFlagWriteRequests();
		virtual void PerformDataWriteTask(std::vector<Image2DCPtr> _realImages, std::vector<Image2DCPtr> _imaginaryImages, int antenna1, int antenna2, int spectralWindow);
		
		void ShowStatistics();
		virtual size_t GetMinRecommendedBufferSize(size_t /*threadCount*/) { return 1; }
		virtual size_t GetMaxRecommendedBufferSize(size_t /*threadCount*/) { return 2; }
		void SetReadUVW(bool readUVW) { _readUVW = readUVW; }
	private:
		void initializeReorderedMS();
		void reorderMS();
		void updateOriginalMSData();
		void updateOriginalMSFlags();
		void performFlagWriteTask(std::vector<Mask2DCPtr> flags, int antenna1, int antenna2);
		
		template<bool UpdateData, bool UpdateFlags>
		void updateOriginalMS();
		
		void removeTemporaryFiles();
		std::string DataFilename(int antenna1, int antenna2) const
		{
			std::stringstream dataFilename;
			dataFilename << "data-" << antenna1 << "x" << antenna2 << ".tmp";
			return dataFilename.str();
		}
		std::string FlagFilename(int antenna1, int antenna2) const
		{
			std::stringstream flagFilename;
			flagFilename << "flag-" << antenna1 << "x" << antenna2 << ".tmp";
			return flagFilename.str();
		}

		DirectBaselineReader _directReader;
		bool _msIsReordered;
		bool _removeReorderedFiles;
		bool _reorderedDataFilesHaveChanged;
		bool _reorderedFlagFilesHaveChanged;
		size_t _maxMemoryUse;
		bool _readUVW;
};

#endif // INDIRECTBASELINEREADER_H
