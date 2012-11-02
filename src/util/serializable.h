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
#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include <iostream>
#include <complex>

#include <stdint.h>

class Serializable
{
	public:
		virtual ~Serializable()
		{
		}
		virtual void Serialize(std::ostream &stream) const = 0;
		virtual void Unserialize(std::istream &stream) = 0;
		
		template<typename T>
		static void SerializeToUInt64(std::ostream &stream, T value)
		{
			uint64_t val64t = value;
			stream.write(reinterpret_cast<char *>(&val64t), sizeof(val64t));
		}
		
		template<typename T>
		static void SerializeToUInt32(std::ostream &stream, T value)
		{
			uint32_t val32t = value;
			stream.write(reinterpret_cast<char *>(&val32t), sizeof(val32t));
		}
		
		static void SerializeToFloat(std::ostream &stream, float value)
		{
			stream.write(reinterpret_cast<char *>(&value), sizeof(value));
		}
		
		static void SerializeToDouble(std::ostream &stream, double value)
		{
			stream.write(reinterpret_cast<char *>(&value), sizeof(value));
		}
		
		static void SerializeToLDouble(std::ostream &stream, long double value)
		{
			stream.write(reinterpret_cast<char *>(&value), sizeof(value));
		}
		
		static void SerializeToLDoubleC(std::ostream &stream, std::complex<long double> value)
		{
			stream.write(reinterpret_cast<char *>(&value), sizeof(value));
		}
		
		static void SerializeToString(std::ostream &stream, const std::string &str)
		{
			SerializeToUInt64(stream, str.size());
			stream.write(str.c_str(), str.size());
		}
		
		static uint64_t UnserializeUInt64(std::istream &stream)
		{
			return Unserialize<uint64_t>(stream);
		}
		
		static uint32_t UnserializeUInt32(std::istream &stream)
		{
			return Unserialize<uint32_t>(stream);
		}
		
		static double UnserializeFloat(std::istream &stream)
		{
			return Unserialize<float>(stream);
		}

		static double UnserializeDouble(std::istream &stream)
		{
			return Unserialize<double>(stream);
		}

		static long double UnserializeLDouble(std::istream &stream)
		{
			return Unserialize<long double>(stream);
		}
		
		static std::complex<long double> UnserializeLDoubleC(std::istream &stream)
		{
			return Unserialize<std::complex<long double> >(stream);
		}
		
		static void UnserializeString(std::istream &stream, std::string &destStr)
		{
			size_t size = UnserializeUInt64(stream);
			char *str = new char[size];
			stream.read(str, size);
			destStr = std::string(str, size);
			delete[] str;
		}
	private:
		template<typename T>
		static T Unserialize(std::istream &stream)
		{
			T val;
			stream.read(reinterpret_cast<char *>(&val), sizeof(val));
			return val;
		}
};

#endif
