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
#ifndef HIGHPASSFILTERACTION_H
#define HIGHPASSFILTERACTION_H

#include "action.h"

#include "../control/artifactset.h"

namespace rfiStrategy {

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class HighPassFilterAction : public Action {
		public:
			enum Mode { StoreContaminated, StoreRevised };
			HighPassFilterAction() :
				_windowWidth(22),
				_windowHeight(45),
				_hKernelSigmaSq(7.5),
				_vKernelSigmaSq(15.0),
				_mode(StoreContaminated)
			{
			}
			virtual ~HighPassFilterAction()
			{
			}
			virtual std::string Description()
			{
				return "High-pass filter (Gaussian)";
			}
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress);

			virtual ActionType Type() const { return HighPassFilterActionType; }
			
			unsigned WindowWidth() const { return _windowWidth; }
			unsigned WindowHeight() const { return _windowHeight; }
			double HKernelSigmaSq() const { return _hKernelSigmaSq; }
			double VKernelSigmaSq() const { return _vKernelSigmaSq; }
			enum Mode Mode() const { return _mode; }
			
			void SetWindowWidth(unsigned width) { _windowWidth = width; }
			void SetWindowHeight(unsigned height) { _windowHeight = height; }
			void SetHKernelSigmaSq(double hSigmaSquared) { _hKernelSigmaSq = hSigmaSquared; }
			void SetVKernelSigmaSq(double vSigmaSquared) { _vKernelSigmaSq = vSigmaSquared; }
			void SetMode(enum Mode mode) { _mode = mode; }

		private:
			unsigned _windowWidth, _windowHeight;
			double _hKernelSigmaSq, _vKernelSigmaSq;
			enum Mode _mode;
	};

}

#endif
