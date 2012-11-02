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
#ifndef MEDIANWINDOW_H
#define MEDIANWINDOW_H

#include <set>

#include "../../msio/samplerow.h"

template<typename NumType>
class MedianWindow
{
	public:
		void Add(NumType newSample)
		{
			_set.insert(newSample);
		}
		void Remove(NumType sample)
		{
			_set.erase(_set.find(sample));
		}
		NumType Median() const
		{
			if(_set.size() == 0)
				return std::numeric_limits<NumType>::quiet_NaN();
			if(_set.size() % 2 == 0)
			{
				unsigned m = _set.size() / 2 - 1;
				typename std::multiset<NumType>::const_iterator i = _set.begin();
				for(unsigned j=0;j<m;++j)
					++i;
				NumType lMid = *i;
				++i;
				NumType rMid = *i;
				return (lMid + rMid) / 2.0;
			} else {
				unsigned m = _set.size() / 2;
				typename std::multiset<NumType>::const_iterator i = _set.begin();
				for(unsigned j=0;j<m;++j)
					++i;
				return *i;
			}
		}
		static void SubtractMedian(SampleRowPtr sampleRow, unsigned windowSize)
		{
			if(windowSize > sampleRow->Size()*2)
				windowSize = sampleRow->Size()*2;
			SampleRowCPtr copy = SampleRow::CreateCopy(sampleRow);
			MedianWindow<num_t> window;
			unsigned rightPtr, leftPtr = 0;
			for(rightPtr=0;rightPtr<windowSize/2;++rightPtr)
			{
				if(!copy->ValueIsMissing(rightPtr))
					window.Add(copy->Value(rightPtr));
			}
			for(unsigned i=0;i<sampleRow->Size();++i)
			{
				if(rightPtr < sampleRow->Size())
				{
					if(!copy->ValueIsMissing(rightPtr))
						window.Add(copy->Value(rightPtr));
					++rightPtr;
				}
				if(rightPtr >= windowSize)
				{
					if(!copy->ValueIsMissing(leftPtr))
						window.Remove(copy->Value(leftPtr));
					++leftPtr;
				}
				sampleRow->SetValue(i, copy->Value(i) - window.Median());
			}
		}
	private:
		std::multiset<NumType> _set;
};

#endif
