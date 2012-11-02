/***************************************************************************
 *   Copyright (C) 2007 by Andre Offringa   *
 *   offringa@gmail.com   *
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
#include "colormap.h"

ColorMap::ColorMap()
{
}

ColorMap::~ColorMap()
{
}

ColorMap *ColorMap::CreateColorMap(const std::string &type) throw()
{
	if(type == "coldhot")
		return new ColdHotMap();
	else if(type == "redblue")
		return new RedBlueMap();
	else if(type == "redwhiteblue")
		return new RedWhiteBlueMap();
	else if(type=="redyellowblue")
		return new RedYellowBlueMap();
	else if(type == "positive")
		return new PositiveMap();
	else if(type == "invpositive")
		return new InvPositiveMap();
	else if(type == "contrast")
		return new ContrastMap("monochrome");
	else if(type == "redbluecontrast")
		return new ContrastMap("redblue");
	else if(type == "redyellowbluecontrast")
		return new ContrastMap("redyellowblue");
	else if(type == "coldhotcontrast")
		return new ContrastMap("coldhot");
	else if(type == "positivecontrast")
		return new ContrastMap("positive");
	else if(type == "invpositivecontrast")
		return new ContrastMap("invpositive");
	else
		return new MonochromeMap();
}

const std::string ColorMap::_colorMapsString("monochrome, coldhot, redblue, redyellowblue, contrast, coldhotcontrast, redbluecontrast, redyellowbluecontrast, positive, invpositive, positivecontrast, invpositivecontrast");

const std::string &ColorMap::GetColorMapsString() throw()
{
	return _colorMapsString;
}
