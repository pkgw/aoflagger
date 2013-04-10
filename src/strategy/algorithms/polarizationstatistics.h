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
#ifndef POLARIZATIONSTATISTICS_H
#define POLARIZATIONSTATISTICS_H

#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>

#include "../../msio/mask2d.h"
#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

#include "../../util/aologger.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class PolarizationStatistics {
	public:
		PolarizationStatistics() { }
		~PolarizationStatistics() { }

		void Add(class TimeFrequencyData &data)
		{
			unsigned polarizationCount = data.PolarisationCount();
			if(_flaggedCounts.size() == 0)
			{
				_polarizationType = data.Polarisation();
				for(unsigned i=0;i<polarizationCount;++i)
				{
					_flaggedCounts.push_back(0);
					_totalCounts.push_back(0);
					TimeFrequencyData *polData = data.CreateTFDataFromPolarisationIndex(i);
					_names.push_back(TimeFrequencyData::GetPolarisationName(polData->Polarisation()));
					delete polData;
				}
			} else if(_polarizationType != data.Polarisation())
			{
				throw std::runtime_error("Adding differently polarized data to statistics");
			}
			for(unsigned i=0;i<polarizationCount;++i)
			{
				TimeFrequencyData *polData = data.CreateTFDataFromPolarisationIndex(i);
				Mask2DCPtr mask = polData->GetSingleMask();
				delete polData;
				_flaggedCounts[i] += mask->GetCount<true>();
				_totalCounts[i] += mask->Width() * mask->Height();
			}
		}
		bool HasData() { return !_flaggedCounts.empty(); }
		void Report()
		{
			if(HasData())
			{
				AOLogger::Info
					<< "Polarization statistics: ";
				for(unsigned i=0;i<_flaggedCounts.size();++i)
				{
					numl_t percentage = (numl_t) _flaggedCounts[i] * 100.0 / (numl_t) _totalCounts[i];
					if(i!=0)
						AOLogger::Info << ", ";
					AOLogger::Info
						<< _names[i] << ": " << formatPercentage(percentage) << '%';
				}
				AOLogger::Info << '\n';
			} else {
				AOLogger::Info
					<< "No polarization statistics were collected.\n";
			}
		}
	private:
		std::string formatPercentage(numl_t percentage)
		{
			std::ostringstream s;
			if(percentage >= 1.0)
				s << round(percentage*10.0)/10.0;
			else if(percentage >= 0.1)
				s << round(percentage*100.0)/100.0;
			else
				s << round(percentage*1000.0)/1000.0;
			return s.str();
		}

		std::vector<long unsigned> _flaggedCounts, _totalCounts;
		std::vector<std::string> _names;
		enum PolarisationType _polarizationType;
};

#endif
