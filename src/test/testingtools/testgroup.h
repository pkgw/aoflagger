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
#ifndef AOFLAGGER_TESTGROUP_H
#define AOFLAGGER_TESTGROUP_H

#include <stdexcept>

#include "testitem.h"
#include "unittest.h"

class TestGroup : public TestItem {
	public:
		TestGroup(const std::string &name) : _name(name), _successes(0), _failures(0)
		{
		}
		
		virtual ~TestGroup()
		{
			for(std::vector<TestItem *>::iterator i=_tests.begin();i!=_tests.end();++i)
			{
				delete *i;
			}
		}
		
		virtual void Initialize() = 0;
		
		void Add(UnitTest *test)
		{
			_tests.push_back(test);
		}
		
		void Add(TestGroup *group)
		{
			_tests.push_back(group);
		}
		
		void Run()
		{
			std::cout << "\n=== Group " << Name() << " ===\n\n";
			Initialize();
			for(std::vector<TestItem *>::iterator i=_tests.begin();i!=_tests.end();++i)
			{
				TestItem *item = *i;
				
				TestGroup *group = dynamic_cast<TestGroup*>(item);
				UnitTest *unitTest = dynamic_cast<UnitTest*>(item);
				if(group != 0)
				{
					std::cout << "=== Group " << group->Name() << " ===\n";
					group->Run();
				} else if(unitTest != 0)
				{
					std::cout << "Unit test '" << unitTest->Name() << "':\n";
					unitTest->Run();
					_successes += unitTest->Successes();
					_failures += unitTest->Failures();
				} else
				{
					throw std::runtime_error("Invalid item in test group");
				}
				std::cout << '\n';
			}
		}
		
		const std::string &Name() const
		{
			return _name;
		}
		
		unsigned Successes() const { return _successes; }
		unsigned Failures() const { return _failures; }
	private:
		std::vector<TestItem *> _tests;
		std::string _name;
		unsigned _successes, _failures;
};

#endif
