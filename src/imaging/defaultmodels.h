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
#ifndef DEFAULTMODELS_H
#define DEFAULTMODELS_H

#include "model.h"
#include "observatorium.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class DefaultModels {
	public:
		enum SetLocation { EmptySet, NCPSet, B1834Set };
		enum Distortion { NoDistortion, ConstantDistortion, VariableDistortion, FaintDistortion, MislocatedDistortion, OnAxisSource };

		static double DistortionRA()
		{
			return 4.940;
		}
		
		static double DistortionDec()
		{
			return 0.571;
		}
		
		static std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> LoadSet(enum SetLocation setLocation, enum Distortion distortion, double noiseSigma, size_t channelCount = 64, double bandwidth = 2500000.0*16.0, unsigned a1=0, unsigned a2=5)
		{
			double ra, dec, factor;
			getSetData(setLocation, ra, dec, factor);
			Model model;
			model.SetNoiseSigma(noiseSigma);
			if(setLocation != EmptySet)
				model.loadUrsaMajor(ra, dec, factor);
			switch(distortion)
			{
				case NoDistortion:
					break;
				case ConstantDistortion:
					model.loadUrsaMajorDistortingSource(ra, dec, factor, true);
					break;
				case VariableDistortion:
					model.loadUrsaMajorDistortingVariableSource(ra, dec, factor, false, false);
					break;
				case FaintDistortion:
					model.loadUrsaMajorDistortingVariableSource(ra, dec, factor, true, false);
					break;
				case MislocatedDistortion:
					model.loadUrsaMajorDistortingVariableSource(ra, dec, factor, false, true);
					break;
				case OnAxisSource:
					model.loadOnAxisSource(ra, dec, factor);
					break;
			}
			WSRTObservatorium wsrtObservatorium(channelCount, bandwidth);
			return model.SimulateObservation(wsrtObservatorium, dec, ra, a1, a2);
		}
	
	private:
		static void getSetData(enum SetLocation setLocation, double &ra, double &dec, double &factor)
		{
			if(setLocation == NCPSet)
			{
				dec = 0.5*M_PI + 0.12800;
				ra = -0.03000;
				factor = 1.0;
			} else {
				dec = 1.083;
				ra = 4.865;
				factor = 4.0;
			}
		}
	
};

#endif
