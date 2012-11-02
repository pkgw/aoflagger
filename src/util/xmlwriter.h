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
#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <fstream>
#include <string>
#include <stack>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class XmlWriter {
	public:
		XmlWriter() : _file(0), _stream(0)
		{
		}

		~XmlWriter()
		{
			Close();
		}

		void Close()
		{
			if(_file != 0)
			{
				while(_openedElements.size() != 0)
					Close();
				delete _file;
				_file = 0;
			}
		}

		void StartDocument(const std::string &filename)
		{
			Close();
			_file = new std::ofstream(filename.c_str());
			_stream = _file;
			startDocument();
		}

		void StartDocument(std::ostream &stream)
		{
			Close();
			_stream = &stream;
			startDocument();
		}

		void Comment(const char *comment)
		{
			startNewTag();

			(*_stream) << "<!-- " << comment << "-->";
		}

		void Start(const char *element)
		{
			startNewTag();

			(*_stream) << '<' << element;
			_inTag = true;
			_tagHasData = false;
			_openedElements.push(element);
		}

		void End()
		{
			if(_openedElements.size() == 0)
				throw std::runtime_error("End() called without open element");
			if(_inTag)
				(*_stream) << " />";
			else {
				(*_stream) << '\n';
				indent(_openedElements.size()-1);
				(*_stream) << "</" << _openedElements.top() << '>';
			}
			_openedElements.pop();
			_inTag = false;
		}

		void Attribute(const char *attributeName, const char *value)
		{ 
			if(!_inTag)
				throw std::runtime_error("Attribute() called incorrectly");
			(*_stream) << ' ' << attributeName << "=\"" << value << "\"";
		}

		template<typename ValueType>
		void Attribute(const char *attributeName, ValueType value)
		{ 
			std::stringstream s;
			s << value;
			Attribute(attributeName, s.str().c_str());
		}

		void Write(const char *element, const char *value)
		{
			startNewTag();

			(*_stream) << '<' << element << '>' << value << "</" << element << '>';
		}

		template<typename ValueType>
		void Write(const char *element, ValueType value)
		{
			std::stringstream s;
			s << value;
			Write(element, s.str().c_str());
		}

	private:
		void startDocument()
		{
			(*_stream) << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
			_inTag = false;
		}
		void closeTag()
		{
			if(_inTag)
			{
				(*_stream) << '>';
				_inTag = false;
			}
		}
		void startNewTag()
		{
			closeTag();
			(*_stream) << '\n';
			indent(_openedElements.size());
		}

		void indent(unsigned depth)
		{
			for(unsigned i=0;i<depth;++i)
				(*_stream) << ' ' << ' ';
		}

		std::stack<std::string> _openedElements;

		std::ofstream *_file;
		std::ostream *_stream;
		bool _inTag;
		bool _tagHasData;
};

#endif
