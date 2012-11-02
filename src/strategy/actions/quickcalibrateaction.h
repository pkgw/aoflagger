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

#ifndef RFIQUICKCALIBRATEACTION_H
#define RFIQUICKCALIBRATEACTION_H

#include "../algorithms/thresholdtools.h"

#include "../control/actioncontainer.h"

#include "../../util/progresslistener.h"

namespace rfiStrategy {

	class QuickCalibrateAction : public Action
	{
		public:
			QuickCalibrateAction() { }

			virtual std::string Description()
			{
				return "Quickly calibrate";
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &)
			{
				Image2DCPtr image = artifacts.ContaminatedData().GetSingleImage();
				Mask2DCPtr mask = artifacts.ContaminatedData().GetSingleMask();
				num_t mean, stddev;
				ThresholdTools::WinsorizedMeanAndStdDev(image, mask, mean, stddev);
				for(size_t i=0;i!=artifacts.ContaminatedData().ImageCount();++i)
				{
					Image2DCPtr image = artifacts.ContaminatedData().GetImage(i);
					Image2DPtr normalized = Image2D::CreateCopy(image);
					normalized->MultiplyValues(1.0/mean);
					artifacts.ContaminatedData().SetImage(i, normalized);
				}
			}
			virtual ActionType Type() const { return QuickCalibrateActionType; }
		private:
	};
}

#endif // RFIQUICKCALIBRATEACTION_H
