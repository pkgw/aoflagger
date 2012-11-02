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
#include "timeselectionaction.h"

#include <map>

#include "../../msio/timefrequencydata.h"
#include "../../msio/image2d.h"
#include "../../msio/samplerow.h"

#include "../algorithms/medianwindow.h"
#include "../algorithms/rfistatistics.h"

#include "../control/artifactset.h"

namespace rfiStrategy {

/**
 * Automatic selection selects all timesteps which RMS is higher than some value relative to the stddev of
 * all timesteps.
 */
void TimeSelectionAction::AutomaticSelection(ArtifactSet &artifacts)
{
	Image2DCPtr image = artifacts.ContaminatedData().GetSingleImage();
	SampleRowPtr timesteps = SampleRow::CreateEmpty(image->Width());
	Mask2DPtr mask = Mask2D::CreateCopy(artifacts.ContaminatedData().GetSingleMask());
	for(size_t x=0;x<image->Width();++x)
	{
		SampleRowPtr row = SampleRow::CreateFromColumnWithMissings(image, mask, x);
		timesteps->SetValue(x, row->RMSWithMissings());
	}
	bool change;
	MedianWindow<num_t>::SubtractMedian(timesteps, 512);
	do {
		num_t median = 0.0;
		num_t stddev = timesteps->StdDevWithMissings(0.0);
		change = false;
		for(size_t x=0;x<timesteps->Size();++x)
		{
			if(!timesteps->ValueIsMissing(x) && (timesteps->Value(x) - median > stddev * _threshold || median - timesteps->Value(x) > stddev * _threshold))
			{
				mask->SetAllVertically<true>(x);
				timesteps->SetValueMissing(x);
				change = true;
			}
		}
	} while(change);
	artifacts.ContaminatedData().SetGlobalMask(mask);
}

} // end of namespace
