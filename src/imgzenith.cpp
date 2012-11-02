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

#include <tables/Tables/Table.h>

#include <measures/Measures/UVWMachine.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>

#include <ms/MeasurementSets/MeasurementSet.h>

#include "msio/antennainfo.h"
#include "msio/measurementset.h"

#include "imaging/zenithimager.h"
#include "msio/pngfile.h"

const casa::Unit radUnit("rad");
const casa::Unit dayUnit("d");
const casa::Unit degUnit("deg");

void repoint(casa::MDirection &phaseDirection, casa::MPosition &position, const casa::MEpoch &obstime, double &u, double &v, double &w, double &phaseRotation, casa::MPosition &a1, casa::MPosition &a2, bool verbose)
{
	//MPosition location(MVPosition(Quantity(1, "km"), Quantity(150, "deg"), Quantity(20, "deg")), MPosition::WGS84);
	
	//casa::MEpoch obstime(casa::Quantity(t, dayUnit), casa::MEpoch::UTC);
	//if(verbose)
	//	std::cout << "Time=" << obstime.getValue() << '\n';
	casa::MeasFrame timeAndLocation(obstime, position);
	
	casa::MDirection::Ref refApparent(casa::MDirection::AZEL, timeAndLocation);
	
	// Calculate zenith
	casa::MDirection outDirection(
		casa::Quantity(0.0, radUnit),       // Az
		casa::Quantity(0.5*M_PI, radUnit),  // El
	refApparent);
	casa::MDirection j2000Direction =
		casa::MDirection::Convert(outDirection, casa::MDirection::J2000)();
	if(verbose)
		std::cout << "Zenith=" << j2000Direction.getAngle().getValue(degUnit) << '\n';
	
	// Construct a CASA UVW converter
	casa::UVWMachine uvwConverter(j2000Direction, phaseDirection);
	casa::Vector<double> uvwVector(3);
	uvwVector[0] = u;
	uvwVector[1] = v;
	uvwVector[2] = w;
	//std::cout << "In: " << u << ',' << v << ',' << w << '\n';
	phaseRotation = uvwConverter.getPhase(uvwVector);
	// std::cout << "Phase shift: " << phaseRotation << '\n';
	//uvwConverter.convertUVW(uvwVector); // getPhase already does that!
	u = uvwVector[0];
	v = uvwVector[1];
	w = uvwVector[2];
	//std::cout << "Out: " << u << ',' << v << ',' << w << '\n';
	//std::cout << "Phase centre: " << uvwConverter.phaseCenter().getValue() << '\n';
}


void repoint(casa::MDirection &phaseDirection, casa::MDirection &newDirection, double &u, double &v, double &w, double &phaseRotation)
{
	// Construct a CASA UVW converter
	casa::UVWMachine uvwConverter(newDirection, phaseDirection);
	casa::Vector<double> uvwVector(3);
	uvwVector[0] = u;
	uvwVector[1] = v;
	uvwVector[2] = w;
	phaseRotation = uvwConverter.getPhase(uvwVector);
	u = uvwVector[0];
	v = uvwVector[1];
	w = uvwVector[2];
}


int main(int argc, char *argv[])
{
	std::string filename(argv[1]);
	size_t integrationSteps, resolution, startTimeIndex;
	bool useFlags;
	if(argc >= 3)
		integrationSteps = atoi(argv[2]);
	else
		integrationSteps = 60;
	if(argc >= 4)
		resolution = atoi(argv[3]);
	else
		resolution = 1024;
	if(argc >= 5)
		startTimeIndex = atoi(argv[4]);
	else
		startTimeIndex = 0;
	if(argc >= 6)
		useFlags = atoi(argv[4])==1;
	else
		useFlags = true;
	
	std::cout << "Opening " << filename << "...\n";
	
	const unsigned polarizationCount = MeasurementSet::GetPolarizationCount(filename);
	const BandInfo band = MeasurementSet::GetBandInfo(filename, 0);
	
	const unsigned antennaIndex = 0;
	
	casa::MeasurementSet table(argv[1]);
	casa::MEpoch::ROScalarColumn timeColumn(table, "TIME");
	casa::ROArrayColumn<double> uvwColumn(table, "UVW");
	casa::ROScalarColumn<int> ant1Column(table, "ANTENNA1");
	casa::ROScalarColumn<int> ant2Column(table, "ANTENNA2");
	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	
	casa::Table antennaTable(table.antenna());
	casa::MPosition::ROScalarColumn antPositionColumn(antennaTable, "POSITION");
	casa::ROScalarColumn<casa::String> antNameColumn(antennaTable, "NAME");
	casa::MPosition antennaPositions[antennaTable.nrow()];
	for(unsigned i = 0;i<antennaTable.nrow();++i)
	{
		antennaPositions[i] = antPositionColumn(i);
	}
	casa::MPosition position = antennaPositions[0];
	std::cout << "Frame of reference of antennae: " << position.getRefString() << '\n';
	std::cout << "Imaging zenith of antenna " << antNameColumn(antennaIndex)
		<< ", pos=" << position.getValue() << '\n';
		
	casa::Table fieldTable(table.field());
	casa::MDirection::ROArrayColumn phaseDirColumn(fieldTable, "PHASE_DIR");
	casa::MDirection phaseDirection = *phaseDirColumn(0).begin();
	std::cout << "Phase direction: " << phaseDirection.getAngle().getValue(degUnit) << '\n';

	std::complex<float> *samples[polarizationCount];
	bool *isRFI[polarizationCount];
	for(unsigned p = 0; p < polarizationCount; ++p)
	{
		isRFI[p] = new bool[band.channels.size()];
		samples[p] = new std::complex<float>[band.channels.size()];
	}

	unsigned row = 0;
	ZenithImager imager;
	imager.Initialize(resolution);
	size_t timeStep = 0;
	bool directionIsSet = false;
	
	casa::MEpoch curT = timeColumn(0);
	while(row<table.nrow() && timeStep < startTimeIndex)
	{
		if(timeColumn(row).getValue() != curT.getValue())
		{
			++timeStep;
			curT = timeColumn(row);
		}
		++row;
	}
	
	while(row<table.nrow())
	{
		const casa::MEpoch t = timeColumn(row);
		casa::MDirection j2000Direction;
		
		if(!directionIsSet)
		{
			// Calculate zenith for this time range
			casa::MeasFrame timeAndLocation(t, position);
			casa::MDirection::Ref refApparent(casa::MDirection::AZEL, timeAndLocation);
			casa::MDirection outDirection(
				casa::Quantity(0.0, radUnit),       // Az
				casa::Quantity(0.5*M_PI, radUnit),  // El
			refApparent);
			j2000Direction =
				casa::MDirection::Convert(outDirection, casa::MDirection::J2000)();
			std::cout << "Zenith=" << j2000Direction.getAngle().getValue(degUnit) << '\n';
			directionIsSet = true;
		}
		
		do
		{
			int a1 = ant1Column(row), a2 = ant2Column(row);
			if(a1 != a2)
			{
				casa::Array<double> uvw = uvwColumn(row);
				casa::Array<double>::const_iterator uvw_i = uvw.begin();
				double u = *uvw_i; ++uvw_i;
				double v = *uvw_i; ++uvw_i;
				double w = *uvw_i;
				double phaseRotation;
				repoint(phaseDirection, j2000Direction, u, v, w, phaseRotation);
				
				const casa::Array<casa::Complex> dataArray = dataColumn(row);
				const casa::Array<bool> flagArray = flagColumn(row);
				
				casa::Array<casa::Complex>::const_iterator dataIter = dataArray.begin();
				casa::Array<bool>::const_iterator flagIter = flagArray.begin();
				
				for(unsigned channel = 0; channel<band.channels.size(); ++channel)
				{
					for(unsigned p = 0; p < polarizationCount; ++p)
					{
						samples[p][channel] = *dataIter;
						if(useFlags)
							isRFI[p][channel] = *flagIter;
						else
							isRFI[p][channel] = false;
						
						++dataIter;
						++flagIter;
					}
				}
				imager.Add(band, samples[0], isRFI[0], u, v, w, phaseRotation);
			}
			++row;
		} while(row<table.nrow() && timeColumn(row).getValue() == t.getValue());
		
		timeStep++;
		if(timeStep % integrationSteps == 0)
		{
			Image2DPtr real, imaginary;
			imager.FourierTransform(real, imaginary);
			
			std::stringstream s;
			if(timeStep < 10000) s << '0';
			if(timeStep < 1000) s << '0';
			if(timeStep < 100) s << '0';
			if(timeStep < 10) s << '0';
			s << timeStep << ".png";
			BlackRedMap map;
			std::cout << "Saving " << s.str() << "... " << std::flush;
			PngFile::Save(*real, std::string("zen/") + s.str(), map);
			PngFile::Save(*imager.UVReal(), std::string("zen-uv/") + s.str(), map);
			imager.Clear();
			std::cout << "Done.\n";
			directionIsSet = false;
		}
	}
}
