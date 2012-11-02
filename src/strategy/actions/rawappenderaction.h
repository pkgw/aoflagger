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

#ifndef RAWAPPENDERACTION_H
#define RAWAPPENDERACTION_H 

#include "action.h"
#include "../control/artifactset.h"

#include "../../msio/rawreader.h"

#include <string>

namespace rfiStrategy {

	class RawAppenderAction : public Action
	{
		public:
			RawAppenderAction() : _filename("output.1ch"), _rawWriter(0), _firstWrite(true)
			{
			}
			virtual ~RawAppenderAction()
			{
				if(_rawWriter != 0)
					delete _rawWriter;
			}
			virtual std::string Description()
			{
				return "Append to raw";
			}
			void Initialize()
			{
				if(_rawWriter != 0)
					delete _rawWriter;
				AOLogger::Debug << "Opening raw writer for filename " << _filename << ".\n";
				_rawWriter = new RawReader(_filename);
				_firstWrite = true;
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener)
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				if(_firstWrite)
				{
					_rawWriter->SetChannelCount(image->Height());
					_rawWriter->StartWrite();
					_firstWrite = false;
				}
				float buffer[image->Height()];
				for(unsigned x=0;x<image->Width();++x)
				{
					for(unsigned y=0;y<image->Height();++y)
						buffer[y] = image->Value(x, y);
					_rawWriter->Write(buffer, 1);
				}
			}
			virtual ActionType Type() const { return RawAppenderActionType; }
		private:
			std::string _filename;
			RawReader *_rawWriter;
			bool _firstWrite;
	};

}

#endif // RAWAPPENDERACTION_H
