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

#include "../../util/progresslistener.h"

#include "statisticalflagaction.h"

#include "../algorithms/statisticalflagger.h"
#include "../algorithms/siroperator.h"

#include "../control/artifactset.h"

namespace rfiStrategy {

	void StatisticalFlagAction::Perform(ArtifactSet &artifacts, class ProgressListener &)
	{
		TimeFrequencyData &data = artifacts.ContaminatedData();
		if(data.MaskCount() > 1)
			throw std::runtime_error("Error: the statistical flag action (dilation operation) can only be applied on data with a single mask. Therefore, it should be placed 'under' a Set all polarization equal-operation, or inside a For each polarization action.");
			
		Mask2DPtr mask = Mask2D::CreateCopy(data.GetSingleMask());
		
		StatisticalFlagger::DilateFlags(mask, _enlargeTimeSize, _enlargeFrequencySize);
		//StatisticalFlagger::LineRemover(mask, (size_t) (_maxContaminatedTimesRatio * (double) mask->Width()), (size_t) (_maxContaminatedFrequenciesRatio * (double) mask->Height()));
		SIROperator::OperateHorizontally(mask, _minimumGoodTimeRatio);
		SIROperator::OperateVertically(mask, _minimumGoodFrequencyRatio);
		data.SetGlobalMask(mask);
		//artifacts.SetRevisedData(data);
	}

} // namespace rfiStrategy
