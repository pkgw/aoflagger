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
#include "fringetestcreater.h"

#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

#include "../../imaging/uvimager.h"

void FringeTestCreater::AddStaticFringe(class TimeFrequencyData &ftData, TimeFrequencyMetaDataCPtr metaData, long double strength)
{
	Image2DCPtr
		real = ftData.GetRealPart(),
		imaginary = ftData.GetImaginaryPart();
	Image2DPtr
		newReal = Image2D::CreateUnsetImagePtr(real->Width(), real->Height()),
		newImaginary = Image2D::CreateUnsetImagePtr(real->Width(), real->Height());
	
	for(size_t channelIndex = 0; channelIndex < ftData.ImageHeight() ; ++channelIndex)
	{
		for(size_t t = 0; t < ftData.ImageWidth() ; ++t)
		{
			num_t fringeRotation =
				2.0 * M_PIn * UVImager::GetFringeCount(0, t, channelIndex, metaData);
			
			newReal->SetValue(t, channelIndex, strength * cosn(fringeRotation) + real->Value(t, channelIndex) );
			newImaginary->SetValue(t, channelIndex, strength * sinn(fringeRotation) + imaginary->Value(t, channelIndex) );
		}
	}
	
	ftData.Set(ftData.Polarisation(), newReal, newImaginary);
}
