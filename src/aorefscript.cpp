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

#include <iostream>

#include "ref/concatenatescript.h"
#include "ref/copyallscript.h"
#include "ref/flagallscript.h"
#include "ref/listnodes.h"
#include "ref/listsets.h"
#include "ref/refmovescript.h"

using namespace std;

int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		cerr << "Syntax: " << argv[0] << " <scripttype> <reffile> <local-destination>\n"
			"script types: concatenate, copyall, flagall, refmove, listnodes, listsets\n";
		return -1;
	} else {
		string type(argv[1]);
		if(type == "concatenate")
			AOTools::ConcatenateScript::Make(cout, argv[2], argv[3]);
		else if(type =="copyall")
			AOTools::CopyAllScript::Make(cout, argv[2], argv[3]);
		else if(type =="flagall")
			AOTools::FlagAllScript::Make(cout, argv[2], argv[3]);
		else if(type =="refmove")
			AOTools::RefMoveScript::Make(cout, argv[2], argv[3]);
		else if(type =="listnodes")
			AOTools::ListNodes::Make(cout, argv[2]);
		else if(type =="listsets")
			AOTools::ListSets::Make(cout, argv[2]);
		else
			cerr << "Incorrect script type given." << "\n";
		return 0;
	}
}
