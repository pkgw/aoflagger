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
#ifndef STATWRITER_H
#define STATWRITER_H

#include <vector>
#include <string>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class StatWriter {
	private:
		StatWriter(const std::string &baseName);
		~StatWriter();

		void SetValue(long double value);
		void SetValue(long double value, unsigned significantDigits)
		{
			SetValue(Format(value, significantDigits));
		}

		void NewMethod(const std::string &name) {
			if(!_lastActionWasNewMethod) {
				_newMethodsStart = _methods.size();
				_activeColumn = _methods.size();
				_activeRow = 0;
				_lastActionWasNewMethod = true;
			}
			_methods.push_back(name);
			AddColumn();
		}

		void NextMethod() throw() {
			_activeColumn++;
			_lastActionWasNewMethod = false;
			if(_activeColumn >= _methods.size())
			{
				_activeColumn = _newMethodsStart;
				_activeRow++;
			}
		}
		void NextMeasurement() throw() { _activeRow++; _activeRow=_newMethodsStart; }
		void Save();
		void SetValueInc(long double value) { SetValue(value); NextMethod(); }
		void SetValueInc(long double value, unsigned significantDigits) { SetValue(value, significantDigits); NextMethod(); }
	private:
		void MakeAccessable(unsigned /*column*/, unsigned row)
		{
			//while(column >= _width)
			//	AddColumn();
			while(row >= _height)
				AddRow();
		}
		void AddColumn();
		void AddRow();
		long double Format(long double value, unsigned significantDigits);

		const std::string _baseFilename;
		std::vector<std::string> _methods;
		std::vector<std::vector<long double> *> _table;
		unsigned _activeColumn;
		unsigned _activeRow;
		unsigned _width, _height;
		unsigned _writeCount;
		unsigned _lazyWriteWait;
		unsigned _newMethodsStart;
		bool _lastActionWasNewMethod;
};

#endif
