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

#ifndef SVDACTION_H
#define SVDACTION_H 

#include "action.h"

namespace rfiStrategy {

	class SVDAction : public Action
	{
		public:
			SVDAction() : _singularValueCount(1) { }
			virtual ~SVDAction() { }
			virtual std::string Description()
			{
				return "Singular value decomposition";
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);
			virtual ActionType Type() const { return SVDActionType; }

			size_t SingularValueCount() const throw() { return _singularValueCount; }
			void SetSingularValueCount(size_t svCount) throw() { _singularValueCount = svCount; }
		private:
			size_t _singularValueCount;
	};

}

#endif // SVDACTION_H
