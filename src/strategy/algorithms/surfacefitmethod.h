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

#ifndef IMAGE_H
#define IMAGE_H

#include "../../msio/image2d.h"
#include "../../msio/timefrequencydata.h"

#include "../../util/ffttools.h"

class SurfaceFitMethod {
	public:
		virtual void Initialize(const TimeFrequencyData &input) = 0;
		virtual unsigned TaskCount() = 0;
		virtual void PerformFit(unsigned taskNumber) = 0;
		virtual ~SurfaceFitMethod() { }
		virtual TimeFrequencyData Background() = 0;
		virtual enum TimeFrequencyData::PhaseRepresentation PhaseRepresentation() const = 0;
};

#endif
