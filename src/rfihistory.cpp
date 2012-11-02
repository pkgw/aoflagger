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

#include <iostream>

#include <libgen.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSTable.h>
#include <ms/MeasurementSets/MSColumns.h>

#include "msio/date.h"

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " <ms1>\n"
		"This program will list the RFI actions that have been performed on the measurement \n"
		"set, by reading the HISTORY table in the set.\n";
	}
	else
	{
		std::ostream &stream(std::cout);

		casa::MeasurementSet ms(argv[1]);
		casa::Table histtab(ms.history());
		casa::ROScalarColumn<double>       time        (histtab, "TIME");
		casa::ROScalarColumn<casa::String> application (histtab, "APPLICATION");
		casa::ROArrayColumn<casa::String>  cli         (histtab, "CLI_COMMAND");
		casa::ROArrayColumn<casa::String>  parms       (histtab, "APP_PARAMS");
		for(unsigned i=0;i<histtab.nrow();++i)
		{
			if(application(i) == "AOFlagger")
			{
				stream << "====================\n"
					"Command: " << cli(i)[0] << "\n"
					"Date: " << Date::AipsMJDToDateString(time(i)) << "\n"
					"Time: " << Date::AipsMJDToTimeString(time(i)) << "\n"
					"Strategy: \n     ----------     \n";
				const casa::Vector<casa::String> appParamsVec = parms(i);
				for(casa::Vector<casa::String>::const_iterator j=appParamsVec.begin();j!=appParamsVec.end();++j)
				{
					stream << *j << '\n';
				}
				stream << "     ----------     \n";
			}
		}
	}
}
