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
#ifndef RFI_RESAMPLING_ACTION
#define RFI_RESAMPLING_ACTION

#include "action.h"

#include "../control/actionblock.h"
#include "../control/artifactset.h"

namespace rfiStrategy {

	class ResamplingAction : public Action
	{
		public:
			enum Operation { Average, NearestNeighbour };
			
			ResamplingAction() : Action(), _operation(Average), _sizeX(5), _sizeY(5)
			{
			}
			virtual std::string Description()
			{
				switch(_operation)
				{
					case Average:
						return "Resample by averaging";
						break;
					case NearestNeighbour:
						return "Resample to nearest neighbour";
						break;
					default:
						return "?";
						break;
				}
			}
			virtual ActionType Type() const { return ResamplingActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData &contaminated = artifacts.ContaminatedData();
				
				for(unsigned i=0;i<contaminated.ImageCount();++i)
				{
					Image2DCPtr image = contaminated.GetImage(i);
					Image2DCPtr newImage;
					
					switch(_operation)
					{
						case Average:
							newImage = performAveraging(image);
							break;
						case NearestNeighbour:
							newImage = performNN(image);
							break;
						default:
							newImage = image;
							break;
					}
					contaminated.SetImage(i, newImage);
				}
			}
			
			enum Operation Operation() const { return _operation; }
			void SetOperation(enum Operation operation) { _operation = operation; }
			
			void SetSizeX(unsigned x) { _sizeX = x; }
			unsigned SizeX() const { return _sizeX; }
			
			void SetSizeY(unsigned y) { _sizeY = y; }
			unsigned SizeY() const { return _sizeY; }
	private:
		enum Operation _operation;
		size_t _sizeX, _sizeY;
		
		Image2DPtr performAveraging(Image2DCPtr input)
		{
			Image2DPtr output = Image2D::CreateZeroImagePtr(input->Width(), input->Height());
			const unsigned
				displaceX = _sizeX / 2,
				displaceY = _sizeY / 2;
			for(unsigned y=0;y<input->Height();++y)
			{
				unsigned destY = (y / _sizeY)  *_sizeY + displaceY;
				if(destY >= input->Height())
					destY = input->Height()-1;
				for(unsigned x=0;x<input->Width();++x)
				{
					unsigned destX = (x / _sizeX) * _sizeX + displaceX;
					if(destX >= input->Width())
						destX = input->Width()-1;
					output->SetValue(destX, destY, output->Value(destX, destY) + input->Value(x, y));
				}
			}
			return output;
		}
		
		Image2DPtr performNN(Image2DCPtr input)
		{
			Image2DPtr output = Image2D::CreateZeroImagePtr(input->Width(), input->Height());
			const unsigned
				displaceX = _sizeX / 2,
				displaceY = _sizeY / 2;
			for(unsigned y=displaceY;y<input->Height();y+=_sizeY)
			{
				for(unsigned x=displaceX;x<input->Width();x+=_sizeX)
				{
					output->SetValue(x, y, input->Value(x, y) * (_sizeX * _sizeY));
				}
			}
			return output;
		}
	};

} // namespace

#endif // RFI_TIME_CONVOLUTION_ACTION
