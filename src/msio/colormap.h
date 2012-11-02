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
/** @file
 * This is the header file for the ColorMap base class and several specific color map classes.
 * @author André Offringa <offringa@gmail.com>
 */
#ifndef COLORMAP_H
#define COLORMAP_H

#include <math.h>
#include <string>

/**
 * The ColorMap class turns a value between -1 and 1 into a gradient color scale.
 */
class ColorMap {
	public:
		/**
		 * Constructor.
		 */
		ColorMap();
		/**
		 * Destructor.
		 */
		virtual ~ColorMap();
		/**
		 * Maps a double value to the red component of the color map.
		 * @param value Value to be mapped  (-1 to 1).
		 * @return The red color value (0 - 255).
		 */
		virtual unsigned char ValueToColorR(long double value) const = 0;
		/**
		 * Maps a double value to the green component of the color map.
		 * @param value Value to be mapped (-1 to 1).
		 * @return The green color value (0 - 255).
		 */
		virtual unsigned char ValueToColorG(long double value) const = 0;
		/**
		 * Maps a double value to the blue component of the color map.
		 * @param value Value to be mapped (-1 to 1).
		 * @return The blue color value (0 - 255).
		 */
		virtual unsigned char ValueToColorB(long double value) const = 0;
		/**
		 * Maps a double value to the alfa (transparency) component of the color map.
		 * @param value Value to be mapped (-1 to 1).
		 * @return The alfa (transparency) color value (0 - 255). 255=fully opaque, 0=fully transparent.
		 */
		virtual unsigned char ValueToColorA(long double value) const = 0;
		/**
		 * Create a color map given its name. The human readable list of names can be retrieved with GetColorMapsString().
		 * @param typeStr name of the color map type.
		 * @return The new create color map. The caller is responsible for @c delete -ing the color map after usage.
		 * @see GetColorMapsString().
		 */
		static ColorMap *CreateColorMap(const std::string &typeStr) throw();
		/**
		 * Returns a string containing a description of the color map names. These names can be used to
		 * create the color map with CreateColorMap().
		 * @return 
		 * @see CreateColorMap().
		 */
		static const std::string &GetColorMapsString() throw();
	private:
		static const std::string _colorMapsString;
};

/**
 * ColorMap that turns a value into a gray scale value. High values represent whiter colors.
 */
class MonochromeMap : public ColorMap {
	public:
		MonochromeMap() { }
		~MonochromeMap() { }
		unsigned char ValueToColorR(long double value) const { return (unsigned char) (value*127.5+127.5); }
		unsigned char ValueToColorG(long double value) const { return (unsigned char) (value*127.5+127.5); }
		unsigned char ValueToColorB(long double value) const { return (unsigned char) (value*127.5+127.5); }
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that turns a value into a gray scale value. High values represent whiter colors.
 */
class InvertedMap : public ColorMap {
	public:
		InvertedMap() { }
		~InvertedMap() { }
		unsigned char ValueToColorR(long double value) const { return (unsigned char) (127.5-value*127.5); }
		unsigned char ValueToColorG(long double value) const { return (unsigned char) (127.5-value*127.5); }
		unsigned char ValueToColorB(long double value) const { return (unsigned char) (127.5-value*127.5); }
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that turns negative values into blue and positive values into red. Zero is represented with black.
 */
class RedBlueMap : public ColorMap {
	public:
		RedBlueMap() { }
		~RedBlueMap() { }
		unsigned char ValueToColorR(long double value) const {
			if(value>0.0)
				return (unsigned char) (value*255.0);
			else
				return 0;
		}
		unsigned char ValueToColorG(long double) const { return 0; }
		unsigned char ValueToColorB(long double value) const {
			if(value<0.0)
				return (unsigned char) (value*-255.0);
			else
				return 0;
		}
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that turns negative values into red and positive values into black. Zero is represented with white.
 */
class BlackRedMap : public ColorMap {
	public:
		BlackRedMap() { }
		~BlackRedMap() { }
		unsigned char ValueToColorR(long double value) const {
			if(value>0.0)
				return (unsigned char) (255-value*255.0);
			else
				return 255;
		}
		unsigned char ValueToColorG(long double value) const
		{
			if(value>0.0)
				return (unsigned char) (255-value*255.0);
			else
				return 255+value*255.0;
		}
		unsigned char ValueToColorB(long double value) const {
			if(value>0.0)
				return (unsigned char) (255-value*255.0);
			else
				return 255+value*255.0;
		}
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that turns negative values into blue and positive values into red. Zero is represented with white.
 */
class RedWhiteBlueMap : public ColorMap {
	public:
		RedWhiteBlueMap() { }
		~RedWhiteBlueMap() { }
		unsigned char ValueToColorR(long double value) const {
			if(value<0.0)
				return (unsigned char) (255.0 + (value*255.0));
			else
				return 255;
		}
		unsigned char ValueToColorG(long double value) const {
			if(value<0.0)
				return (unsigned char) (255.0 + (value*255.0));
			else
				return (unsigned char) (255.0 - (value*255.0));
		}
		unsigned char ValueToColorB(long double value) const {
			if(value>0.0)
				return (unsigned char) (255.0 - (value*255.0));
			else
				return 255;
		}
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that passes several colors.
 */
class ColdHotMap : public ColorMap {
	public:
		ColdHotMap() { }
		~ColdHotMap() { }
		unsigned char ValueToColorR(long double value) const {
			if(value >= 0.5)
				return (unsigned char) ((1.5 - value) * 255.0);
			else if(value < -0.5)
				return 0;
			else if(value < 0.0)
				return (unsigned char) (((value + 0.5)*2.0) * 255.0);
			else
				return 255;
		}
		unsigned char ValueToColorG(long double value) const {
			if(value < -0.5)
				return (unsigned char) (((value + 1.0)*2.0) * 255.0);
			else if(value < 0.0)
				return 255;
			else if(value < 0.5)
				return (unsigned char) (((0.5 - value)*2.0)*255.0);
			else
				return 0;
		}
		unsigned char ValueToColorB(long double value) const {
			if(value < -0.5)
				return (unsigned char) ((value + 1.5) * 255.0);
			else if(value < 0.0)
				return (unsigned char) (((-value)*2.0)*255.0);
			else
				return 0;
		}
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that uses another color map and increases its contrast.
 */
class ContrastMap : public ColorMap {
	private:
		const ColorMap *_map;
	public:
		ContrastMap(const std::string type) :
			_map(CreateColorMap(type)) { }
		~ContrastMap() { delete _map; }
		unsigned char ValueToColorR(long double value) const { return _map->ValueToColorR(value>=0.0 ? sqrt(value) : -sqrt(-value)); }
		unsigned char ValueToColorG(long double value) const { return _map->ValueToColorG(value>=0.0 ? sqrt(value) : -sqrt(-value)); }
		unsigned char ValueToColorB(long double value) const { return _map->ValueToColorB(value>=0.0 ? sqrt(value) : -sqrt(-value)); }
		unsigned char ValueToColorA(long double value) const { return _map->ValueToColorA(value>=0.0 ? sqrt(value) : -sqrt(-value)); }
};

/**
 * ColorMap that uses another color map and applies logarithmic scaling to it. Expects inputs
 * between 0 and 1, will output to map between -1 and 1.
 */
class PosLogMap : public ColorMap {
	private:
		const ColorMap &_map;
	public:
		PosLogMap(const ColorMap &map) :
			_map(map) { }
		~PosLogMap() { }
		unsigned char ValueToColorR(long double value) const { return value >= 0.0  ? _map.ValueToColorR((log10(value*0.9999+0.0001)+2.0)/2.0) : _map.ValueToColorR(-1.0); }
		unsigned char ValueToColorG(long double value) const { return value >= 0.0  ? _map.ValueToColorG((log10(value*0.9999+0.0001)+2.0)/2.0) : _map.ValueToColorG(-1.0);  }
		unsigned char ValueToColorB(long double value) const { return value >= 0.0  ? _map.ValueToColorB((log10(value*0.9999+0.0001)+2.0)/2.0) : _map.ValueToColorB(-1.0);  }
		unsigned char ValueToColorA(long double value) const { return value >= 0.0  ? _map.ValueToColorA((log10(value*0.9999+0.0001)+2.0)/2.0) : _map.ValueToColorA(-1.0);  }
};

class PosMonochromeLogMap : public PosLogMap {
	private:
		const MonochromeMap _monochromeMap;
	public:
		PosMonochromeLogMap() : PosLogMap(_monochromeMap) { }
};

/**
 * ColorMap that uses another color map and applies logarithmic scaling to it. Expects inputs
 * between -1 and 1, will output to map between -1 and 1.
 */
class FullLogMap : public ColorMap {
	private:
		const ColorMap &_map;
	public:
		FullLogMap(const ColorMap &map) :
			_map(map) { }
		~FullLogMap() { }
		unsigned char ValueToColorR(long double value) const { return _map.ValueToColorR((log10(value*0.49995+0.50005)+2.0)/2.0); }
		unsigned char ValueToColorG(long double value) const { return _map.ValueToColorG((log10(value*0.49995+0.50005)+2.0)/2.0);  }
		unsigned char ValueToColorB(long double value) const { return _map.ValueToColorB((log10(value*0.49995+0.50005)+2.0)/2.0);  }
		unsigned char ValueToColorA(long double value) const { return _map.ValueToColorA((log10(value*0.49995+0.50005)+2.0)/2.0);  }
};

/**
 * ColorMap that turns negative values into blue and positive values into red. Zero is represented with yellow.
 */
class RedYellowBlueMap : public ColorMap {
	public:
		RedYellowBlueMap() { }
		~RedYellowBlueMap() { }
		unsigned char ValueToColorR(long double value) const {
			if(value >= 1.0/3.0)
				return 255;
			else if(value >= -1.0/3.0)
				return (unsigned char) (value*(255.0 * 3.0/2.0)) + 128;
			else
				return 0;
		}
		unsigned char ValueToColorG(long double value) const {
			if(value >= 1.0/3.0)
				return 255 - (unsigned char) ((value-1.0/3.0)*(255.0*3.0/2.0));
			else if(value >= 0.0)
				return 255;
			else if(value >= -1.0/3.0)
				return (unsigned char) ((value+1.0/3.0)*(255.0*6.0/2.0));
			else 
				return 0;
		}
		unsigned char ValueToColorB(long double value) const {
			if(value >= 1.0/3.0)
				return 0;
			else if(value >= -1.0/3.0)
				return 255 - (unsigned char) ((value+1.0/3.0)*(255.0*3.0/2.0));
			else
				return (unsigned char) ((value+1.0)*(255.0*3.0/2.0));
		}
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that turns negative values into blue and positive values into red. Zero is represented with black.
 */
class RedYellowBlackBlueMap : public ColorMap {
	public:
		RedYellowBlackBlueMap() { }
		~RedYellowBlackBlueMap() { }
		unsigned char ValueToColorR(long double value) const {
			if(value < 0.0)
				return 0;
			else if(value < 0.5)
				return (unsigned char) (value*2.0*255.0);
			else
				return 255;
		}
		unsigned char ValueToColorG(long double value) const {
			if(value < -0.5)
				return (unsigned char) ((value+1.0)*255.0*2.0);
			else if(value >= 0.5)
				return (unsigned char) ((-value+1.0)*255.0*2.0);
			else if(value < 0.0)
				return (unsigned char) (-value*2.0*255.0);
			else
				return (unsigned char) (value*2.0*255.0);
		}
		unsigned char ValueToColorB(long double value) const {
			if(value >= 0.0)
				return 0;
			else if(value >= -0.5)
				return (unsigned char) (-value*2.0*255.0);
			else
				return 255;
		}
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that turns all negative values into black and positive values into a gray scale.
 */
class PositiveMap : public ColorMap {
	public:
		PositiveMap() { }
		~PositiveMap() { }
		unsigned char ValueToColorR(long double value) const { return (unsigned char) (value>0.0 ? value*255.0 : 0); }
		unsigned char ValueToColorG(long double value) const { return (unsigned char) (value>0.0 ? value*255.0 : 0); }
		unsigned char ValueToColorB(long double value) const { return (unsigned char) (value>0.0 ? value*255.0 : 0); }
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that is the negation of PositiveMap.
 */
class InvPositiveMap : public ColorMap {
	public:
		InvPositiveMap() { }
		~InvPositiveMap() { }
		unsigned char ValueToColorR(long double value) const { return (unsigned char) (value>0.0 ? 255.0-value*255.0 : 255.0); }
		unsigned char ValueToColorG(long double value) const { return (unsigned char) (value>0.0 ? 255.0-value*255.0 : 255.0); }
		unsigned char ValueToColorB(long double value) const { return (unsigned char) (value>0.0 ? 255.0-value*255.0 : 255.0); }
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that is the logarithmic negation of PositiveMap.
 */
class LogInvPositiveMap : public ColorMap {
	public:
		LogInvPositiveMap() { }
		~LogInvPositiveMap() { }
		unsigned char ValueToColorR(long double value) const { return (unsigned char) (value>0.0 ? -log10(value*0.9+0.1)*255.0 : 255.0); }
		unsigned char ValueToColorG(long double value) const { return (unsigned char) (value>0.0 ? -log10(value*0.9+0.1)*255.0 : 255.0); }
		unsigned char ValueToColorB(long double value) const { return (unsigned char) (value>0.0 ? -log10(value*0.9+0.1)*255.0 : 255.0); }
		unsigned char ValueToColorA(long double) const { return 255; }
};

/**
 * ColorMap that turns different integer values into different colours.
 */
class IntMap {
	public:
		static unsigned char R(int value) {
			switch (value%16)
			{
				case 0:
				case 3:
				case 5:
				case 6:
					return 255;
				case 8:
				case 11:
				case 13:
				case 14:
					return 127;
				default:
					return 0;	
			}
		}
		static unsigned char G(int value) {
			switch (value%16)
			{
				case 1:
				case 3:
				case 4:
				case 6:
					return 255;
				case 9:
				case 11:
				case 12:
				case 14:
					return 127;
				default:
					return 0;	
			}
		}
		static unsigned char B(int value) {
			switch (value%16)
			{
				case 2:
				case 4:
				case 5:
				case 6:
					return 255;
				case 10:
				case 12:
				case 13:
				case 14:
					return 127;
				default:
					return 0;	
			}
		}
		static unsigned char A(int) { return 255; }
};

#endif
