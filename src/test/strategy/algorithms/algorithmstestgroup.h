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
#ifndef AOFLAGGER_ALGORITHMSTESTGROUP_H
#define AOFLAGGER_ALGORITHMSTESTGROUP_H

#include "../../testingtools/testgroup.h"

#include "convolutionstest.h"
#include "dilationtest.h"
#include "eigenvaluetest.h"
#include "highpassfiltertest.h"
#include "noisestatisticstest.h"
#include "noisestatisticscollectortest.h"
#include "siroperatortest.h"
#include "statisticalflaggertest.h"
#include "sumthresholdtest.h"
#include "thresholdtoolstest.h"

class AlgorithmsTestGroup : public TestGroup {
	public:
		AlgorithmsTestGroup() : TestGroup("Algorithms") { }
		
		virtual void Initialize()
		{
			Add(new ConvolutionsTest());
			Add(new DilationTest());
			Add(new EigenvalueTest());
			Add(new HighPassFilterTest());
			Add(new NoiseStatisticsTest());
			Add(new NoiseStatisticsCollectorTest());
			Add(new SIROperatorTest());
			Add(new StatisticalFlaggerTest());
			Add(new SumThresholdTest());
			Add(new ThresholdToolsTest());
		}
};

#endif
