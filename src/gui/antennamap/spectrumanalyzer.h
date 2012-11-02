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
#ifndef ANTENNAMAP_SPECTRUMANALYZER_H
#define ANTENNAMAP_SPECTRUMANALYZER_H

#include <gtkmm/drawingarea.h>

#include <cmath>

#include "spectrumanalyzerchannel.h"

namespace antennaMap
{
	
/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class SpectrumAnalyzer {
	public:
		SpectrumAnalyzer(unsigned channelCount)
		{
			for(unsigned i=0;i<channelCount;++i)
				_channels.push_back(new SpectrumAnalyzerChannel());
		}
		
		~SpectrumAnalyzer()
		{
			for(std::vector<SpectrumAnalyzerChannel*>::iterator i=_channels.begin();i!=_channels.end();++i)
				delete *i;
		}
		
		void SetValue(unsigned index, double value, double timeInFrameSeconds)
		{
			_channels[index]->SetValue(value, timeInFrameSeconds);
		}
		
		void Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX, double offsetY, double width, double height, double timeInFrameSeconds)
		{
			double analyzerWidth = width * 0.9;
			double channelMargin = analyzerWidth / (_channels.size()) * 0.2;
			
			double avgValue = 0.0;
			for(unsigned i=0;i<_channels.size();++i)
			{
				const double
					channelStart = offsetX + analyzerWidth * i / _channels.size(),
					channelWidth = analyzerWidth / _channels.size();
				_channels[i]->Draw(cairo, channelStart + channelMargin, offsetY, channelWidth - 2.0 * channelMargin,  height, timeInFrameSeconds);
				avgValue += _channels[i]->Value();
			}
			
			_mainChannel.SetValue(avgValue / _channels.size(), timeInFrameSeconds);
			_mainChannel.Draw(cairo, offsetX + analyzerWidth + channelMargin, offsetY, width - 2.0 * channelMargin, height, timeInFrameSeconds);
		}
		
	private:
		std::vector<SpectrumAnalyzerChannel*> _channels;
		SpectrumAnalyzerChannel _mainChannel;
};

}

#endif
