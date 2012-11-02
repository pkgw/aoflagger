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
#ifndef COMBINE_FLAG_RESULTS_H
#define COMBINE_FLAG_RESULTS_H

#include "../../util/progresslistener.h"

#include "../control/artifactset.h"
#include "../control/actioncontainer.h"

namespace rfiStrategy {

	class CombineFlagResults : public ActionContainer
	{
			public:
				virtual std::string Description()
				{
					return "Combine flag results";
				}
				virtual ActionType Type() const { return CombineFlagResultsType; }
				virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
				{
					if(GetChildCount() == 1)
					{
						GetFirstChild().Perform(artifacts, listener);
					} else {
						TimeFrequencyData originalFlags = artifacts.ContaminatedData();
						TimeFrequencyData joinedFlags = artifacts.ContaminatedData();
						size_t nr = 0;
						for(const_iterator i=begin();i!=end();++i)
						{
							artifacts.SetContaminatedData(originalFlags);
							Action *action = *i;
							listener.OnStartTask(*this, nr, GetChildCount(), action->Description());
							action->Perform(artifacts, listener);
							listener.OnEndTask(*this);
							++nr;
	
							joinedFlags.JoinMask(artifacts.ContaminatedData());
						}
						artifacts.SetContaminatedData(joinedFlags);
					}
				}
	};

} // namespace

#endif //COMBINE_FLAG_RESULTS_H
