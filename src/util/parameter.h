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

#ifndef PARAMETER_H
#define PARAMETER_H

#include <stdexcept>

template<typename T>
class Parameter {
	public:
		Parameter() : _isSet(false), _value() { }
		Parameter(const T val) : _isSet(true), _value(val) { }
		Parameter(const Parameter<T> &source)
			: _isSet(source._isSet), _value(source._value) { }

		Parameter &operator=(const Parameter<T> &source)
		{
			_isSet = source._isSet;
			_value = source._value;
		}

		Parameter &operator=(T val)
		{
			_isSet = true;
			_value = val;
			return *this;
		}
		bool IsSet() const { return _isSet; }

		operator T() const
		{
			return Value();
		}

		T Value() const
		{
			if(_isSet)
				return _value;
			else
				throw std::runtime_error("Trying to access unset parameter");
		}

		T Value(T defaultValue) const
		{
			if(_isSet)
				return _value;
			else
				return defaultValue;
		}
	private:
		bool _isSet;
		T _value;
};

#endif //PARAMETER_H
