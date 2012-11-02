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
#ifndef SVDMITIGATER_H
#define SVDMITIGATER_H

#include <iostream>

#include "../../msio/image2d.h"

#include "surfacefitmethod.h"

// Needs to be included LAST
#include "../../f2c.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class SVDMitigater : public SurfaceFitMethod {
	public:
		SVDMitigater();
		~SVDMitigater();

		virtual void Initialize(const TimeFrequencyData &data) {
			Clear();
			_data = data;
			_iteration = 0;
		}
		virtual unsigned TaskCount() { return 1; }
		virtual void PerformFit(unsigned)
		{
			_iteration++;
			RemoveSingularValues(_removeCount);
		}

		virtual void RemoveSingularValues(unsigned singularValueCount)
		{
			if(!IsDecomposed())
				Decompose();
			for(unsigned i=0;i<singularValueCount;++i)
				SetSingularValue(i, 0.0);
			Compose();
		}

		virtual TimeFrequencyData Background()
		{
			return *_background;
		}

		virtual enum TimeFrequencyData::PhaseRepresentation PhaseRepresentation() const
		{
			return TimeFrequencyData::ComplexRepresentation;
		}

		bool IsDecomposed() const throw() { return _singularValues != 0 ; }
		double SingularValue(unsigned index) const throw() { return _singularValues[index]; }
		void SetRemoveCount(unsigned removeCount) throw() { _removeCount = removeCount; }
		void SetVerbose(bool verbose) throw() { _verbose = verbose; }
		static void CreateSingularValueGraph(const TimeFrequencyData &data, class Plot2D &plot);
	private:
		void Clear();
		void Decompose();
		void Compose();
		void SetSingularValue(unsigned index, double newValue) throw() { _singularValues[index] = newValue; }

		TimeFrequencyData _data;
		TimeFrequencyData *_background;
		double *_singularValues;
		doublecomplex *_leftSingularVectors;
		doublecomplex *_rightSingularVectors;
		long int _m, _n;
		unsigned _iteration;
		unsigned _removeCount;
		bool _verbose;
};

#endif
