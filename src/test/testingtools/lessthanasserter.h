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
#ifndef AOFLAGGER_LESS_THAN_ASSERTER_H
#define AOFLAGGER_LESS_THAN_ASSERTER_H

#include <stdexcept>
#include <sstream>
#include <string>
#include <typeinfo>

class LessThanAsserter {
	public:
		LessThanAsserter()
		{
		}
		
		~LessThanAsserter()
		{
		}
		
		template<typename T, typename S>
		void AssertLessThan(T actual, S limit) const
		{
			if(!(actual < limit))
			{
				throwLessThanComparisonError(actual, limit);
			}
		}

		template<typename T, typename S>
		void AssertLessThan(T actual, S limit, const std::string &description) const
		{
			if(!(actual < limit))
			{
				throwLessThanComparisonError(actual, limit, description);
			}
		}

		template<typename T, typename S>
		void AssertGreaterThan(T actual, S limit) const
		{
			if(!(actual > limit))
			{
				throwGreaterThanComparisonError(actual, limit);
			}
		}

		template<typename T, typename S>
		void AssertGreaterThan(T actual, S limit, const std::string &description) const
		{
			if(!(actual > limit))
			{
				throwGreaterThanComparisonError(actual, limit, description);
			}
		}

		template<typename T, typename S>
		void AssertNotEqual(T first, S second) const
		{
			if(!(first != second))
			{
				throwNotEqualsError(first, second);
			}
		}

		template<typename T, typename S>
		void AssertNotEqual(T first, S second, const std::string &description) const
		{
			if(!(first != second))
			{
				throwNotEqualsError(first, second, description);
			}
		}
	private:
		template <typename T, typename S>
		void throwLessThanComparisonError(T actual, S limit) const
		{
			std::stringstream s;
			s << "AssertLessThan failed: " << actual << " < " << limit << " was false" << "\n("
			<< limit << " was the limit, " << actual << " was the actual value, ";
			addTypes(s, typeid(actual), typeid(limit));
			throw std::runtime_error(s.str());
		}

		template <typename T, typename S>
		void throwLessThanComparisonError(T actual, S limit, const std::string &description) const
		{
			std::stringstream s;
			s << "AssertLessThan failed on test '" << description
			<< "': " << actual << " < " << limit << " was false" << "\n("
			<< limit << " was the limit, " << actual << " was the actual value, ";
			addTypes(s, typeid(actual), typeid(limit));
			throw std::runtime_error(s.str());
		}

		template <typename T, typename S>
		void throwGreaterThanComparisonError(T actual, S limit) const
		{
			std::stringstream s;
			s << "AssertGreaterThan failed: " << actual << " > " << limit << " was false" << "\n("
			<< limit << " was the limit, " << actual << " was the actual value, ";
			addTypes(s, typeid(actual), typeid(limit));
			throw std::runtime_error(s.str());
		}

		template <typename T, typename S>
		void throwGreaterThanComparisonError(T actual, S limit, const std::string &description) const
		{
			std::stringstream s;
			s << "AssertGreaterThan failed on test '" << description
			<< "': " << actual << " > " << limit << " was false" << "\n("
			<< limit << " was the limit, " << actual << " was the actual value, ";
			addTypes(s, typeid(actual), typeid(limit));
			throw std::runtime_error(s.str());
		}
		
		template <typename T, typename S>
		void throwNotEqualsError(T first, S second) const
		{
			std::stringstream s;
			s << "AssertNotEqual failed: " << first << " != " << second << " was false" << "\n(";
			addTypes(s, typeid(first), typeid(second));
			throw std::runtime_error(s.str());
		}

		template <typename T, typename S>
		void throwNotEqualsError(T first, S second, const std::string &description) const
		{
			std::stringstream s;
			s << "AssertNotEqual failed on test '" << description
			<< "': " << first << " != " << second << " was false" << "\n(";
			addTypes(s, typeid(first), typeid(second));
			throw std::runtime_error(s.str());
		}

		void addTypes(std::stringstream &s, const std::type_info &leftHandType, const std::type_info &rightHandType) const
		{
			if(leftHandType == rightHandType)
			{
				s << "type = '" << leftHandType.name() << "')";
			} else {
				s << "compared different types: " << leftHandType.name() << ", " << rightHandType.name() << ')';
			}
		}
};

#endif
