/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#ifndef AO_REFFILEENTRY_H
#define AO_REFFILEENTRY_H

#include <string>
#include <sstream>
#include <cstdlib>

#include "reffileexception.h"

namespace AOTools
{
	class RefFileEntry
	{
		public:
			friend class RefFile;
	
			RefFileEntry() : _size(0)
			{
			}

			RefFileEntry(const RefFileEntry &source) :
				_path(source._path),
				_frequency(source._frequency),
				_size(source._size),
				_node(source._node)
			{
			}
			
			void operator=(const RefFileEntry &source)
			{
				_path = source._path;
				_frequency = source._frequency;
				_size = source._size;
				_node = source._node;
			}

			const std::string &Path() const { return _path; }
			const std::string &Frequency() const { return _frequency; }
			unsigned Size() const { return _size; }
			const std::string &Node() const { return _node; }

			void SetPath(const std::string &path) { _path = path; }
		private:
			std::string _path;
			std::string _frequency;
			unsigned _size;
			std::string _node;

			bool read(std::istream &stream)
			{
				std::string line;
				do {
					if(!stream.good()) return false;
					std::getline(stream, line);
					if(stream.fail()) return false;
					if(stream.bad()) throw RefFileException("Error in IO");
				} while(ignoreLine(line));

				assignFromString(line);
				return true;
			}

			void write(std::ostream &stream) const
			{
				stream << _path << ' ' << _frequency << ' ' << _size << ' ' << _node << "\n";
			}

			void assignFromString(const std::string &line)
			{
				std::string::const_iterator i=line.begin();
				if(!getNextToken(_path, i, line.end()))
					throw RefFileException("Expecting a path");
				if(!getNextToken(_frequency, i, line.end()))
					throw RefFileException("Expecting frequency description");
				std::string sizeString;
				if(!getNextToken(sizeString, i, line.end()))
					throw RefFileException("Expecting a size");
				_size = atoi(sizeString.c_str());
				if(!getNextToken(_node, i, line.end()))
					throw RefFileException("Expecting a node");
			}

			static bool ignoreLine(const std::string &line)
			{
				for(std::string::const_iterator i=line.begin();i!=line.end();++i)
				{
					if(*i == '#') return true;
					if(!ignorable(*i)) return false;
				}
				return true;
			}

			static bool ignorable(std::string::value_type ch)
			{
				return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
			}

			static bool getNextToken(std::string &dest, std::string::const_iterator &ptr, const std::string::const_iterator end)
			{
				std::ostringstream token;
				while(ptr != end && ignorable(*ptr))
					++ptr;
				if(ptr == end) return false;
				while(ptr != end && !ignorable(*ptr))
				{
					token << *ptr;
					++ptr;
				}
				dest = token.str();
				return dest.size() != 0;
			}
	};
}

#endif //AO_REFFILEENTRY_H
