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
#include "statwriter.h"

#include <math.h>
#include <fstream>
#include <sstream>

StatWriter::StatWriter(const std::string &baseName) : _baseFilename(baseName), _activeColumn(0), _activeRow(0), _width(0), _height(0), _writeCount(0), _lazyWriteWait(8), _newMethodsStart(0), _lastActionWasNewMethod(true)
{
}


StatWriter::~StatWriter()
{
	Save();

	for(std::vector<std::vector<long double> *>::iterator i=_table.begin();i!=_table.end();++i)
	{
		delete *i;
	}	
}

void StatWriter::AddColumn()
{
	for(std::vector<std::vector<long double> *>::iterator i=_table.begin();i!=_table.end();++i)
	{
		(*i)->push_back(0.0);
	}
	_width++;
}

void StatWriter::AddRow()
{
	std::vector<long double> *newRow = new std::vector<long double>();
	for(unsigned i=0;i<_width;++i)
		newRow->push_back(0.0);
	_table.push_back(newRow);
	_height++;
}

void StatWriter::SetValue(long double value)
{
	MakeAccessable(_activeColumn, _activeRow);
	(*_table[_activeRow])[_activeColumn] = value;
	_writeCount++;
	if(_writeCount >= _lazyWriteWait)
		Save();
	_lastActionWasNewMethod = false;
}

long double StatWriter::Format(long double value, unsigned significantDigits)
{
	unsigned long base = 1;
	while(significantDigits > 0) {
		base *= 10;
		significantDigits--;
	}
	return roundl(value * base) / base;
}

void StatWriter::Save()
{
	std::stringstream filename;
	filename << _baseFilename << ".txt";
	std::ofstream file(filename.str().c_str());

	if(_methods.size() > 0) {
		file << "#" << _methods[0];
		for(std::vector<std::string>::const_iterator i=_methods.begin()+1;i!=_methods.end();++i) {
			file << "\t" << *i; 
		}
		file << "\n";
	}

	for(std::vector<std::vector<long double> *>::const_iterator j=_table.begin();j!=_table.end();++j)
	{
		if((*j)->size()>0) {
			file << (**j)[0];
			for(std::vector<long double>::const_iterator i=(*j)->begin()+1;i!=(*j)->end();++i)
				file << '\t' << (*i);
		}
		file << "\n";
	}
	file.close();
	_writeCount = 0;
}
