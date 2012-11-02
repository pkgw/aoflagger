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
#ifndef MEMORY_BASELINE_READER_H
#define MEMORY_BASELINE_READER_H

#include <map>
#include <vector>
#include <stdexcept>

#include "antennainfo.h"
#include "baselinereader.h"
#include "image2d.h"
#include "mask2d.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class MemoryBaselineReader : public BaselineReader {
	public:
		explicit MemoryBaselineReader(const std::string &msFile)
			: BaselineReader(msFile), _isRead(false), _areFlagsChanged(false)
		{
		}
		
		~MemoryBaselineReader()
		{
			if(_areFlagsChanged) writeFlags();
		}

		virtual void PerformReadRequests();
		
		virtual void PerformFlagWriteRequests();
		
		virtual void PerformDataWriteTask(std::vector<Image2DCPtr> /*_realImages*/, std::vector<Image2DCPtr> /*_imaginaryImages*/, int /*antenna1*/, int /*antenna2*/, int /*spectralWindow*/)
		{
			throw std::runtime_error("The full mem reader can not write data back to file: use the indirect reader");
		}
		
		static bool IsEnoughMemoryAvailable(const std::string &msFile);
		
		virtual size_t GetMinRecommendedBufferSize(size_t /*threadCount*/) { return 1; }
		virtual size_t GetMaxRecommendedBufferSize(size_t /*threadCount*/) { return 2; }
	private:
		void readSet();
		void writeFlags();
		
		bool _isRead, _areFlagsChanged;
		
		class BaselineID
		{
		public:
			unsigned antenna1, antenna2, spw;
			
			bool operator<(const BaselineID &other) const
			{
				if(antenna1<other.antenna1) return true;
				else if(antenna1==other.antenna1)
				{
					if(antenna2<other.antenna2) return true;
					else if(antenna2==other.antenna2) return spw < other.spw;
				}
				return false;
			}
		};
		
		std::map<BaselineID, BaselineReader::Result> _baselines;
};

#endif // DIRECTBASELINEREADER_H
