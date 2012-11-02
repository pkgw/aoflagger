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
#ifndef INTEGERDOMAIN_H
#define INTEGERDOMAIN_H

#include <vector>
#include <string>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class IntegerDomain {
	public:
		IntegerDomain(const IntegerDomain &source);
		IntegerDomain(int singleValue);
		IntegerDomain(const int *values, unsigned count);
		IntegerDomain(int first, unsigned count);
		IntegerDomain(int first, unsigned step, unsigned count);
		explicit IntegerDomain(const std::string &str);
		~IntegerDomain() { }
		unsigned ValueCount() const throw() { return _values.size(); }
		int GetValue(unsigned index) const throw() { return _values[index]; }
		void Join(const IntegerDomain &other) throw();
		bool IsIn(int number) const throw() {
			for(std::vector<int>::const_iterator i=_values.begin();	i!=_values.end();++i) {
				if(*i == number) return true;
			}
			return false;
		}
		IntegerDomain Split(unsigned partCount, unsigned partIndex) const;
		unsigned Index(int number) const throw() {
			for(unsigned i=0;	i!=_values.size();++i) {
				if(_values[i] == number) return i;
			}
			return -1;
		}
	private:
		std::vector<int> _values;
};

#endif
