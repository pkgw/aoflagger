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
#include "frequencyselectionaction.h"

#include "../../msio/image2d.h"
#include "../../msio/samplerow.h"

#include "../control/artifactset.h"

namespace rfiStrategy {
	
void FrequencySelectionAction::Perform(ArtifactSet &artifacts, class ProgressListener &)
{
	Image2DCPtr image = artifacts.ContaminatedData().GetSingleImage();
	SampleRowPtr channels = SampleRow::CreateEmpty(image->Height());
	Mask2DPtr mask = Mask2D::CreateCopy(artifacts.ContaminatedData().GetSingleMask());
	for(size_t y=0;y<image->Height();++y)
	{
		SampleRowPtr row = SampleRow::CreateFromRowWithMissings(image, mask, y);
		channels->SetValue(y, row->RMSWithMissings());
	}
	bool change;
	do {
		num_t median = channels->MedianWithMissings();
		num_t stddev = channels->StdDevWithMissings(median);
		change = false;
		double effectiveThreshold = _threshold * stddev * artifacts.Sensitivity();
		for(size_t y=0;y<channels->Size();++y)
		{
			if(!channels->ValueIsMissing(y) && (channels->Value(y) - median > effectiveThreshold || (_clipDown && median - channels->Value(y) > effectiveThreshold)))
			{
				mask->SetAllHorizontally<true>(y);
				channels->SetValueMissing(y);
				change = true;
			}
		}
	} while(change);
	artifacts.ContaminatedData().SetGlobalMask(mask);
}

} // namespace
