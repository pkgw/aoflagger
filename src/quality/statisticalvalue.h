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
#ifndef MSIO_STATISTICAL_VALUE_H
#define MSIO_STATISTICAL_VALUE_H

#include <complex>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/

class StatisticalValue {
	public:
		StatisticalValue(unsigned _polarizationCount) :
			_polarizationCount(_polarizationCount),
			_values(new std::complex<float>[_polarizationCount])
		{
		}
		
		StatisticalValue(const StatisticalValue &source) :
			_polarizationCount(source._polarizationCount),
			_values(new std::complex<float>[source._polarizationCount])
		{
			_kindIndex = source._kindIndex;
			for(unsigned i=0;i<_polarizationCount;++i)
				_values[i] = source._values[i];
		}
		
		~StatisticalValue()
		{
			delete[] _values;
		}
		
		StatisticalValue &operator=(const StatisticalValue &source)
		{
			if(_polarizationCount != source._polarizationCount)
			{
				_polarizationCount = source._polarizationCount;
				delete[] _values;
				_values = new std::complex<float>[_polarizationCount];
			}
			_kindIndex = source._kindIndex;
			for(unsigned i=0;i<_polarizationCount;++i)
				_values[i] = source._values[i];
			return *this;
		}
		
		unsigned PolarizationCount() const { return _polarizationCount; }
		
		unsigned KindIndex() const { return _kindIndex; }
		void SetKindIndex(unsigned kindIndex) { _kindIndex = kindIndex; }
		
		std::complex<float> Value(unsigned polarizationIndex) const { return _values[polarizationIndex]; }
		void SetValue(unsigned polarizationIndex, std::complex<float> newValue)
		{
			_values[polarizationIndex] = newValue;
		}
	
	private:
		unsigned _polarizationCount;
		unsigned _kindIndex;
		std::complex<float> *_values;
};

#endif
