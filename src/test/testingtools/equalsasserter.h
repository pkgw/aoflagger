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
#ifndef AOFLAGGER_EQUALS_ASSERTER_H
#define AOFLAGGER_EQUALS_ASSERTER_H

#include <string>
#include <stdexcept>
#include <sstream>
#include <typeinfo>
#include <cmath>

class EqualsAsserter {
	public:
		EqualsAsserter()
		{
		}
		
		~EqualsAsserter()
		{
		}
		
		template<typename T>
		void AssertEquals(bool, T) const
		{
			std::stringstream s;
			s <<
				"AssertEquals(): Trying to compare a boolean with some other type is not allowed for safety "
				"(first parameter to AssertEquals was a bool, second was " << typeid(T).name() << ")";
			throw std::runtime_error(s.str());
		}
		
		template<typename T>
		void AssertEquals(T, bool) const
		{
			std::stringstream s;
			s <<
				"AssertEquals(): Trying to compare a boolean with some other type is not allowed for safety "
				"(second parameter to AssertEquals was a bool, first was " << typeid(T).name() << ")";
			throw std::runtime_error(s.str());
		}
		
		template<typename T>
		void AssertEquals(bool, T, const std::string &description) const
		{
			std::stringstream s;
			s <<
				"AssertEquals() on test \"" << description << "\": "
				"Trying to compare a boolean with some other type is not allowed for safety "
				"(first parameter to AssertEquals was a bool, second was " << typeid(T).name() << ")";
			throw std::runtime_error(s.str());
		}
		
		template<typename T>
		void AssertEquals(T, bool, const std::string &description) const
		{
			std::stringstream s;
			s <<
				"AssertEquals() on test \"" << description << "\": "
				"Trying to compare a boolean with some other type is not allowed for safety "
				"(second parameter to AssertEquals was a bool, first was " << typeid(T).name() << ")";
		}
		
		void AssertEquals(const std::string &actual, const std::string &expected) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(std::string("'") + actual + "'", std::string("'") + expected + "'", typeid(const std::string &));
			}
		}
		
		void AssertEquals(const std::string &actual, const char *expected) const
		{ AssertEquals(actual, std::string(expected)); }
		
		void AssertEquals(const char *actual, const std::string &expected) const
		{ AssertEquals(std::string(actual), expected); }
		
		void AssertEquals(const std::string &actual, const std::string &expected, const std::string &description) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(std::string("'") + actual + "'", std::string("'") + expected + "'", typeid(const std::string &), description);
			}
		}
		
		void AssertEquals(const std::string &actual, const char *expected, const std::string &description) const
		{
			AssertEquals(actual, std::string(expected), description);
		}
		
		void AssertEquals(const char *actual, const std::string &expected, const std::string &description) const
		{
			AssertEquals(std::string(actual), expected, description);
		}
		
		template<typename T>
		void AssertEquals(T actual, T expected) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(actual, expected, typeid(T));
			}
		}
		
		template<typename T>
		void AssertEquals(T actual, T expected, const std::string &description) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(actual, expected, typeid(T), description);
			}
		}
		
		void AssertEquals(float actual, double expected) const
		{ AssertEquals<float>((float) actual, (float) expected); }
		
		void AssertEquals(double actual, float expected) const
		{ AssertEquals<float>((float) actual, (float) expected); }
		
		void AssertEquals(float actual, long double expected) const
		{ AssertEquals<float>((float) actual, (float) expected); }
		
		void AssertEquals(long double actual, float expected) const
		{ AssertEquals<float>((float) actual, (float) expected); }
		
		void AssertEquals(double actual, long double expected) const
		{ AssertEquals<double>((double) actual, (double) expected); }
		
		void AssertEquals(long double actual, double expected) const
		{ AssertEquals<double>((double) actual, (double) expected); }
		
		void AssertEquals(float actual, double expected, const std::string&description) const
		{ AssertEquals<float>((float) actual, (float) expected, description); }
		
		void AssertEquals(double actual, float expected, const std::string &description) const
		{ AssertEquals<float>((float) actual, (float) expected, description); }
		
		void AssertEquals(float actual, long double expected, const std::string &description) const
		{ AssertEquals<float>((float) actual, (float) expected, description); }
		
		void AssertEquals(long double actual, float expected, const std::string &description) const
		{ AssertEquals<float>((float) actual, (float) expected, description); }
		
		void AssertEquals(double actual, long double expected, const std::string &description) const
		{ AssertEquals<double>((double) actual, (double) expected, description); }
		
		void AssertEquals(long double actual, double expected, const std::string &description) const
		{ AssertEquals<double>((double) actual, (double) expected, description); }
		
		void AssertAlmostEqual(float actual, float expected) const
		{
			assertAlmostEqual(actual, expected, 24, "float");
		}
		
		void AssertAlmostEqual(double actual, double expected) const
		{
			assertAlmostEqual<double>(actual, expected, 40, "double");
		}

		void AssertAlmostEqual(long double actual, long double expected) const
		{
			assertAlmostEqual<long double>(actual, expected, 40, "long double");
		}
		
		void AssertAlmostEqual(float actual, float expected, const std::string &description) const
		{
			assertAlmostEqual<float>(actual, expected, 12, "float", description);
		}
		
		void AssertAlmostEqual(double actual, double expected, const std::string &description) const
		{
			assertAlmostEqual<double>(actual, expected, 24, "double", description);
		}

		void AssertAlmostEqual(long double actual, long double expected, const std::string &description) const
		{
			assertAlmostEqual<long double>(actual, expected, 32, "long double", description);
		}
		
		void AssertAlmostEqual(float actual, double expected) const
		{ AssertAlmostEqual((float) actual, (float) expected); }

		void AssertAlmostEqual(float actual, long double expected) const
		{ AssertAlmostEqual((float) actual, (float) expected); }
		
		void AssertAlmostEqual(double actual, float expected) const
		{ AssertAlmostEqual((float) actual, (float) expected); }

		void AssertAlmostEqual(long double actual, float expected) const
		{ AssertAlmostEqual((float) actual, (float) expected); }
		
		void AssertAlmostEqual(double actual, long double expected) const
		{ AssertAlmostEqual((double) actual, (double) expected); }
		
		void AssertAlmostEqual(long double actual, double expected) const
		{ AssertAlmostEqual((double) actual, (double) expected); }
		
		void AssertAlmostEqual(float actual, double expected, const std::string &description) const
		{ AssertAlmostEqual((float) actual, (float) expected, description); }

		void AssertAlmostEqual(float actual, long double expected, const std::string &description) const
		{ AssertAlmostEqual((float) actual, (float) expected, description); }
		
		void AssertAlmostEqual(double actual, float expected, const std::string &description) const
		{ AssertAlmostEqual((float) actual, (float) expected, description); }

		void AssertAlmostEqual(long double actual, float expected, const std::string &description) const
		{ AssertAlmostEqual((float) actual, (float) expected, description); }
		
		void AssertAlmostEqual(double actual, long double expected, const std::string &description) const
		{ AssertAlmostEqual((double) actual, (double) expected, description); }
		
		void AssertAlmostEqual(long double actual, double expected, const std::string &description) const
		{ AssertAlmostEqual((double) actual, (double) expected, description); }
		
		void AssertTrue(bool actual) const
		{ assertEqualsBool(actual, true); }

		void AssertTrue(bool actual, const std::string &description) const
		{ assertEqualsBool(actual, true, description); }

		void AssertFalse(bool actual) const
		{ assertEqualsBool(actual, false); }

		void AssertFalse(bool actual, const std::string &description) const
		{ assertEqualsBool(actual, false, description); }

		static bool IsAlmostEqual(float first, float second)
		{
			return isAlmostEqual<float>(first, second, 12);
		}
	private:
		void assertEqualsBool(bool actual, bool expected) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(boolToString(actual), boolToString(expected), typeid(bool));
			}
		}
		
		void assertEqualsBool(bool actual, bool expected, const std::string &description) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(boolToString(actual), boolToString(expected), typeid(bool), description);
			}
		}
		
		template <typename T>
		void throwComparisonError(T actual, T expected, const std::type_info &type) const
		{
			std::stringstream s;
			s << "AssertEquals failed: " << actual << " == " << expected << " was false" << "\n("
			<< expected << " was expected, " << actual << " was the actual value, type = '" << type.name() << "')";
			throw std::runtime_error(s.str());
		}
		
		template <typename T>
		void throwComparisonError(T actual, T expected, const std::type_info &type, const std::string &description) const
		{
			std::stringstream s;
			s << "AssertEquals failed on test '" << description << "': " << actual << " == " << expected << " was false" << "\n("
			<< expected << " was expected, " << actual << " was the actual value, type = '" << type.name() << "')";
			throw std::runtime_error(s.str());
		}
		
		template <typename T>
		void assertAlmostEqual(T actual, T expected, unsigned precision, const char *typeStr) const
		{
			if(!std::isfinite(actual) || !std::isfinite(expected))
			{
				std::stringstream s;
				s << "AssertAlmostEqual was called with nonfinite value: " << actual << " == " << expected << "\n("
				<< expected << " was expected, " << actual << " was the actual value, type = " << typeStr << ")";
				throw std::runtime_error(s.str());
			}
			T maxArgument = std::max(std::max(std::abs(actual), std::abs(expected)), (T) 1.0);
			T maxDistance = maxArgument/(T) powl((T) 2.0, precision);
			T distance = std::abs(actual - expected);
			if(distance > maxDistance)
			{
				std::stringstream s;
				s << "AssertAlmostEqual failed: |" << actual << " - " << expected << "| = " << std::abs(actual - expected)
				<< " > " << maxDistance << "\n("
				<< expected << " was expected, " << actual << " was the actual value, type = " << typeStr << ")";
				throw std::runtime_error(s.str());
			}
		}
		
		template <typename T>
		void assertAlmostEqual(T actual, T expected, unsigned precision, const char *typeStr, const std::string &description) const
		{
			if(!std::isfinite(actual) || !std::isfinite(expected))
			{
				std::stringstream s;
				s << "AssertAlmostEqual in test '" << description << "' was called with nonfinite value: " << actual << " == " << expected << "\n("
				<< expected << " was expected, " << actual << " was the actual value, type = " << typeStr << ")";
				throw std::runtime_error(s.str());
			}
		T maxArgument = std::max(std::max(std::abs(actual), std::abs(expected)), (T) 1.0);
			T maxDistance = maxArgument/(T) powl((T) 2.0, precision);
			T distance = std::abs(actual - expected);
			if(distance > maxDistance)
			{
				std::stringstream s;
				s << "AssertAlmostEqual failed on test '" << description << "': |" << actual << " - " << expected << "| = " << std::abs(actual - expected)
				<< " > " << maxDistance << "\n("
				<< expected << " was expected, " << actual << " was the actual value, type = " << typeStr << ")";
				throw std::runtime_error(s.str());
			}
		}
		
		template<typename T>
		static bool isAlmostEqual(T first, T second, unsigned precision)
		{
			T maxArgument = std::max(std::max(std::abs(first), std::abs(second)), (T) 1.0);
			T maxDistance = maxArgument/(T) powl((T) 2.0, precision);
			T distance = std::abs(first - second);
			return distance <= maxDistance;
		}
		
		const char *boolToString(bool value) const
		{
			if(value)
				return "true";
			else
				return "false";
		}
};

#endif
