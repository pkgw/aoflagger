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
#ifndef TIMEFREQUENCYIMAGER_H
#define TIMEFREQUENCYIMAGER_H

#include <map>

#include "image2d.h"
#include "mask2d.h"
#include "measurementset.h"
#include "antennainfo.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class TimeFrequencyImager {
	public:
		explicit TimeFrequencyImager(MeasurementSet &measurementSet);
		~TimeFrequencyImager();
		void Image(size_t antenna1Select, size_t antenna2Select, size_t spectralWindowSelect);
		void Image(size_t antenna1Select, size_t antenna2Select, size_t spectralWindowSelect, size_t startIndex, size_t endIndex);

		Image2DCPtr RealXX() const { return _realXX; }
		Image2DCPtr ImaginaryXX() const { return _imaginaryXX; }
		Image2DCPtr RealXY() const { return _realXY; }
		Image2DCPtr ImaginaryXY() const { return _imaginaryXY; }
		Image2DCPtr RealYX() const { return _realYX; }
		Image2DCPtr ImaginaryYX() const { return _imaginaryYX; }
		Image2DCPtr RealYY() const { return _realYY; }
		Image2DCPtr ImaginaryYY() const { return _imaginaryYY; }

		Mask2DCPtr FlagXX() const { return _flagXX; }
		Mask2DCPtr FlagXY() const { return _flagXY; }
		Mask2DCPtr FlagYX() const { return _flagYX; }
		Mask2DCPtr FlagYY() const { return _flagYY; }
		Mask2DCPtr FlagStokesI() const { return _flagCombined; }

		bool HasData() const { return _realXX != 0; }
		bool HasFlags() const { return _flagXX != 0; }
		void SetReadFlags(bool readFlags) { _readFlags = readFlags; }
		void SetReadData(bool readData) { _readData = readData; }
		void SetReadStokesI(bool readStokesI) { _readStokesI = readStokesI; }
		void SetReadIndividualPolarisations(bool readPolarisations) throw()
		{
			_readXX = readPolarisations;
			_readXY = readPolarisations;
			_readYX = readPolarisations;
			_readYY = readPolarisations;
		}
		void SetReadXX(bool readXX) { _readXX = readXX; }
		void SetReadXY(bool readXY) { _readXY = readXY; }
		void SetReadYX(bool readYX) { _readYX = readYX; }
		void SetReadYY(bool readYY) { _readYY = readYY; }

		void WriteNewFlags(Mask2DCPtr newXX, Mask2DCPtr newXY, Mask2DCPtr newYX, Mask2DCPtr newYY);
		void WriteNewFlags(Mask2DCPtr newXX, Mask2DCPtr newXY, Mask2DCPtr newYX, Mask2DCPtr newYY, int antenna1, int antenna2, int spectralWindow);
		void WriteNewFlagsPart(Mask2DCPtr newXX, Mask2DCPtr newXY, Mask2DCPtr newYX, Mask2DCPtr newYY, int antenna1, int antenna2, int spectralWindow, size_t timeOffset, size_t timeEnd, size_t leftBorder=0, size_t rightBorder=0);
		void SetDataKind(enum DataKind kind) { _dataKind = kind; }

		const class BandInfo &BandInfo() const { return _bandInfo; }

		size_t Antenna1() const { return _antenna1Select; }
		size_t Antenna2() const { return _antenna2Select; }
		size_t SpectralWindow() const { return _spectralWindowSelect; }

		class TimeFrequencyData GetData() const;
		static void PartInfo(const std::string &msFile, size_t maxTimeScans, size_t &timeScanCount, size_t &partCount);
		const std::vector<class UVW> &UVW() const { return _uvw; }
		void ClearImages();
	private:
		void initializePolarizations();
		void checkPolarizations();
		void image(size_t antenna1Select, size_t antenna2Select, size_t spectralWindowSelect, size_t startIndex, size_t endIndex);

		static void setObservationTimes(MeasurementSet &set, std::map<double,size_t> &observationTimes);
		void readUVWData();

		casa::ROArrayColumn<casa::Complex> *CreateDataColumn(DataKind kind, class casa::Table &table);
		void ReadTimeData(size_t xOffset, int frequencyCount, const casa::Array<casa::Complex> data, const casa::Array<casa::Complex> *model);
		void ReadTimeFlags(size_t xOffset, int frequencyCount, const casa::Array<bool> flag);
		void ReadWeights(size_t xOffset, int frequencyCount, const casa::Array<float> weight);

		Image2DPtr _realXX, _imaginaryXX;
		Image2DPtr _realXY, _imaginaryXY;
		Image2DPtr _realYX, _imaginaryYX;
		Image2DPtr _realYY, _imaginaryYY;

		Image2DPtr _realStokesI, _imaginaryStokesI;

		Mask2DPtr _flagXX;
		Mask2DPtr _flagXY;
		Mask2DPtr _flagYX;
		Mask2DPtr _flagYY;
		Mask2DPtr _flagCombined;

		bool _readData, _readFlags;

		bool _readXX, _readXY, _readYX, _readYY, _readStokesI, _readStokesIDirectly;
		int _xxIndex, _xyIndex, _yxIndex, _yyIndex, _stokesIIndex;

		class MeasurementSet *_measurementSet;
		size_t _antenna1Select;
		size_t _antenna2Select;
		size_t _spectralWindowSelect;
		enum DataKind _dataKind;
		class BandInfo _bandInfo;
		std::map<double,size_t> _observationTimes;
		std::vector<class UVW> _uvw;
		casa::Table *_sortedTable;
};

#endif
