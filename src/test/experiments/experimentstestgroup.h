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
#ifndef AOFLAGGER_EXPERIMENTSTESTGROUP_H
#define AOFLAGGER_EXPERIMENTSTESTGROUP_H

#include "../testingtools/testgroup.h"

#include "defaultstrategyspeedtest.h"
//#include "filterresultstest.h"
#include "highpassfilterexperiment.h"
//#include "scaleinvariantdilationexperiment.h"
//#include "rankoperatorrocexperiment.h"

class ExperimentsTestGroup : public TestGroup {
	public:
		ExperimentsTestGroup() : TestGroup("Experiments") { }
		
		virtual void Initialize()
		{
			Add(new HighPassFilterExperiment());
			//Add(new RankOperatorROCExperiment());
			Add(new DefaultStrategySpeedTest());
			//Add(new FilterResultsTest());
			//Add(new ScaleInvariantDilationExperiment());
		}
};

#endif
