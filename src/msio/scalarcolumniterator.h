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
#ifndef SCALARCOLUMNITERATOR_H
#define SCALARCOLUMNITERATOR_H

#include <ms/MeasurementSets/MSColumns.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
template<typename T>
class ScalarColumnIterator {
	public:
		ScalarColumnIterator(class casa::ROScalarColumn<T> &column, unsigned row) :
			_column(&column), _row(row)
		{
		}
		ScalarColumnIterator(const ScalarColumnIterator<T> &source) :
			_column(source._column), _row(source._row)
		{ }
		~ScalarColumnIterator() { }
		ScalarColumnIterator &operator=(const ScalarColumnIterator<T> &source)
		{
			_column = source._column;
			_row = source._row;
			return *this;
		}
		ScalarColumnIterator<T> &operator++() {
			_row++;
			return *this;
		}
		T *operator->() const {
			return (*_column)(_row);
		}
		T operator*() const {
			return (*_column)(_row);
		}
		bool operator!=(const ScalarColumnIterator<T> &other) const {
			return _row!=other._row;
		}
		bool operator==(const ScalarColumnIterator<T> &other) const {
			return _row==other._row;
		}
		static ScalarColumnIterator First(casa::ScalarColumn<T> &column)
		{
			return ScalarColumnIterator<T>(column, 0);
		}
		static ScalarColumnIterator First(casa::ROScalarColumn<T> &column)
		{
			return ScalarColumnIterator<T>(column, 0);
		}
	private:
		casa::ROScalarColumn<T> *_column;
		unsigned _row;
};

#endif
