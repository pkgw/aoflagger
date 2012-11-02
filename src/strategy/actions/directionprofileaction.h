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
#ifndef DIRECTIONPROFILEACTION_H
#define DIRECTIONPROFILEACTION_H

#include <cmath>

#include "../../msio/timefrequencydata.h"

#include "action.h"

#include "../algorithms/vertevd.h"

#include "../control/artifactset.h"

namespace rfiStrategy {
	
	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class DirectionProfileAction : public Action {
		public:
			enum Axis { HorizontalAxis, VerticalAxis };
			enum ProfileAction { Store, Apply, Unapply };
			
			DirectionProfileAction() : _axis(HorizontalAxis), _profileAction(Store)
			{
			}
			~DirectionProfileAction()
			{
			}
			virtual std::string Description()
			{
				std::string s;
				if(_axis == VerticalAxis)
					s = "vertically";
				else
					s = "horizontally";
				switch(_profileAction)
				{
					default:
					case Store: return std::string("Store profile ") + s;
					case Apply: return std::string("Apply profile ") + s;
					case Unapply: return std::string("Unapply profile ") + s;
				}
			}

			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData data(artifacts.ContaminatedData());
				if(data.PolarisationCount()!=1)
				{
					throw std::runtime_error("Profile changing requires one polarization");
				}
				switch(_profileAction)
				{
					default:
					case Store:
						if(_axis == HorizontalAxis)
							storeHorizontalProfile(artifacts);
						else
							storeVerticalProfile(artifacts);
						break;
					case Apply:
						if(_axis == HorizontalAxis)
							applyHorizontalProfile(artifacts, false);
						else
							applyVerticalProfile(artifacts, false);
						break;
					case Unapply:
						if(_axis == HorizontalAxis)
							applyHorizontalProfile(artifacts, true);
						else
							applyVerticalProfile(artifacts, true);
						break;
				}
			}

			virtual ActionType Type() const { return DirectionProfileActionType; }
			
			enum Axis Axis() const { return _axis; }
			void SetAxis(enum Axis axis) { _axis = axis; }
			
			enum ProfileAction ProfileAction() const { return _profileAction; }
			void SetProfileAction(enum ProfileAction profileAction) { _profileAction = profileAction; }
		private:
			enum Axis _axis;
			enum ProfileAction _profileAction;
			
			void storeHorizontalProfile(ArtifactSet &artifacts)
			{
				const TimeFrequencyData &data = artifacts.ContaminatedData();
				std::vector<num_t> &profile = artifacts.HorizontalProfile();
				profile.clear();
			
				Image2DCPtr input = data.GetSingleImage();
				for(unsigned x=0;x<input->Width();++x)
				{
					num_t timeAvg = 0.0;
					for(unsigned y=0;y<input->Height();++y)
					{
						if(std::isfinite(input->Value(x, y)))
							timeAvg += input->Value(x, y);
					}
					timeAvg /= (num_t) input->Height();
					profile.push_back(timeAvg);
				}
			}
			
			void storeVerticalProfile(ArtifactSet &artifacts)
			{
				const TimeFrequencyData &data = artifacts.ContaminatedData();
				std::vector<num_t> &profile = artifacts.VerticalProfile();
				profile.clear();
				
				Image2DCPtr input = data.GetSingleImage();
				for(unsigned y=0;y<input->Height();++y)
				{
					num_t timeAvg = 0.0;
					for(unsigned x=0;x<input->Width();++x)
					{
						if(std::isfinite(input->Value(x, y)))
							timeAvg += input->Value(x, y);
					}
					timeAvg /= (num_t) input->Width();
					profile.push_back(timeAvg);
				}
			}
			
			void applyHorizontalProfile(ArtifactSet &artifacts, bool inverse)
			{
				TimeFrequencyData &data = artifacts.ContaminatedData();
				const std::vector<num_t> &profile = artifacts.HorizontalProfile();
				if(profile.size() != data.ImageWidth())
					throw std::runtime_error("Can not apply horizontal profile: profile not stored or stored for different image size");
				for(unsigned i=0;i<data.ImageCount();++i)
				{
					Image2DCPtr input = data.GetImage(i);
					Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
					for(unsigned x=0;x<input->Width();++x)
					{
						for(unsigned y=0;y<input->Height();++y)
						{
							if(inverse)
							{
								if(profile[x] != 0.0)
									output->SetValue(x, y, input->Value(x, y) / profile[x]);
								else
									output->SetValue(x, y, 0.0);
							} else {
									output->SetValue(x, y, input->Value(x, y) * profile[x]);
							}
						}
					}
					data.SetImage(i, output);
				}
			}
			
			void applyVerticalProfile(ArtifactSet &artifacts, bool inverse)
			{
				TimeFrequencyData &data = artifacts.ContaminatedData();
				const std::vector<num_t> &profile = artifacts.VerticalProfile();
				if(profile.size() != data.ImageHeight())
					throw std::runtime_error("Can not apply horizontal profile: profile not stored or stored for different image size");
				for(unsigned i=0;i<data.ImageCount();++i)
				{
					Image2DCPtr input = data.GetImage(i);
					Image2DPtr output = Image2D::CreateUnsetImagePtr(input->Width(), input->Height());
					for(unsigned y=0;y<input->Height();++y)
					{
						for(unsigned x=0;x<input->Width();++x)
						{
							if(inverse)
							{
								if(profile[y] != 0.0)
									output->SetValue(x, y, input->Value(x, y) / profile[y]);
								else
									output->SetValue(x, y, 0.0);
							} else {
									output->SetValue(x, y, input->Value(x, y) * profile[y]);
							}
						}
					}
					data.SetImage(i, output);
				}
			}
	};

}
	
#endif // DIRECTIONPROFILEACTION_H
