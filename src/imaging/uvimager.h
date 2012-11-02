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
#ifndef UVIMAGER_H
#define UVIMAGER_H

#include "../msio/timefrequencymetadata.h"
#include "../msio/measurementset.h"
#include "../msio/date.h"

#include "../msio/timefrequencydata.h"

struct SingleFrequencySingleBaselineData {
	casa::Complex data;
	bool flag;
	bool available;
	double time;
	unsigned field;
};

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class UVImager {
	public:
		enum ImageKind { Homogeneous, Flagging };
		UVImager(unsigned long xRes, unsigned long yRes, ImageKind imageKind=Homogeneous);
		~UVImager();
		void Image(class MeasurementSet &measurementSet, unsigned band);
		void Image(class MeasurementSet &measurementSet, unsigned band, const class IntegerDomain &frequencies);
		void Image(const class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, unsigned frequencyIndex);
		void Image(const class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
		{
			for(unsigned y=0;y<data.ImageHeight();++y)
				Image(data, metaData, y);
		}
		void Image(const class TimeFrequencyData &data, class SpatialMatrixMetaData *metaData);
		void InverseImage(class MeasurementSet &prototype, unsigned band, const class Image2D &uvReal, const class Image2D &uvImaginary, unsigned antenna1, unsigned antenna2);
		const class Image2D &WeightImage() const { return *_uvWeights; }
		const class Image2D &RealUVImage() const { return *_uvReal; }
		const class Image2D &ImaginaryUVImage() const { return *_uvImaginary; }
		void SetInvertFlagging(bool newValue) { _invertFlagging = newValue; }
		void SetDirectFT(bool directFT) { _directFT = directFT; }

		/**
		 * This function calculates the uv position, but it's not optimized for speed, so it's not to be used in an imager.
		 * @param [out] u the u position (in the uv-plane domain)
		 * @param [out] v the v position (in the uv-plane domain)
		 * @param [in] timeIndex the time index to calculate the u,v position for
		 * @param [in] frequencyIndex the frequency index to calculate the u,v position for
		 * @param [in] metaData information about the baseline
		 */
		static void GetUVPosition(num_t &u, num_t &v, size_t timeIndex, size_t frequencyIndex, TimeFrequencyMetaDataCPtr metaData);

		static num_t GetFringeStopFrequency(size_t time, const Baseline &baseline, num_t delayDirectionRA, num_t delayDirectionDec, num_t frequency, TimeFrequencyMetaDataCPtr metaData);
		//static double GetFringeCount(long double timeStart, long double timeEnd, const Baseline &baseline, long double delayDirectionRA, long double delayDirectionDec, long double frequency);
		static num_t GetFringeCount(size_t timeIndexStart, size_t timeIndexEnd, unsigned channelIndex, TimeFrequencyMetaDataCPtr metaData);
		
		static numl_t GetWPosition(numl_t delayDirectionDec, numl_t delayDirectionRA, numl_t frequency, numl_t earthLattitudeAngle, numl_t dx, numl_t dy)
		{
			numl_t wavelength = 299792458.0L / frequency;
			numl_t raSinEnd = sinn(-delayDirectionRA - earthLattitudeAngle);
			numl_t raCosEnd = cosn(-delayDirectionRA - earthLattitudeAngle);
			numl_t decCos = cosn(delayDirectionDec);
			// term "+ dz * decCos" is eliminated because of subtraction
			num_t wPosition =
				(dx*raCosEnd - dy*raSinEnd) * (-decCos) / wavelength;
			return wPosition;
		}
		
		static numl_t TimeToEarthLattitude(unsigned x, TimeFrequencyMetaDataCPtr metaData)
		{
			return TimeToEarthLattitude(metaData->ObservationTimes()[x]);
		}
		
		static numl_t TimeToEarthLattitude(double time)
		{
			return time*M_PInl/(12.0*60.0*60.0);
		}
		
		void Empty();
		void PerformFFT();
		bool HasUV() const { return _uvReal != 0; }
		bool HasFFT() const { return _uvFTReal != 0; }
		const class Image2D &FTReal() const { return *_uvFTReal; }
		const class Image2D &FTImaginary() const { return *_uvFTImaginary; }
		class Image2D &FTReal() { return *_uvFTReal; }
		class Image2D &FTImaginary() { return *_uvFTImaginary; }
		void SetUVScaling(num_t newScale)
		{
			_uvScaling = newScale;
		}
		num_t UVScaling() const {
			return _uvScaling;
		}
		void ApplyWeightsToUV();
		void SetUVValue(num_t u, num_t v, num_t r, num_t i, num_t weight);

		template<typename T>
		static T FrequencyToWavelength(const T frequency)
		{
			return SpeedOfLight() / frequency; 
		}
		static long double SpeedOfLight()
		{
			return 299792458.0L;
		}
		numl_t ImageDistanceToDecRaDistance(numl_t imageDistance) const
		{
			return imageDistance * _uvScaling;
		}
		static numl_t AverageUVDistance(TimeFrequencyMetaDataCPtr metaData, const double frequencyHz)
		{
			const std::vector<UVW> &uvw = metaData->UVW();
			numl_t avgDist = 0.0;
			for(std::vector<UVW>::const_iterator i=uvw.begin();i!=uvw.end();++i)
			{
				numl_t dist = i->u*i->u + i->v*i->v;
				avgDist += sqrtnl(dist);
			}
			return avgDist * frequencyHz / (SpeedOfLight() * (numl_t) uvw.size());
		}
		static numl_t UVTrackLength(TimeFrequencyMetaDataCPtr metaData, const double frequencyHz)
		{
			const std::vector<UVW> &uvw = metaData->UVW();
			numl_t length = 0.0;
			std::vector<UVW>::const_iterator i=uvw.begin();
			if(i == uvw.end()) return 0.0;
			while((i+1)!=uvw.end())
			{
				std::vector<UVW>::const_iterator n=i;
				++n;
				const numl_t
					du = n->u - i->u,
					dv = n->v - i->v;
				length += sqrtnl(du*du + dv*dv);
				i=n;
			}
			return length * frequencyHz / SpeedOfLight();
		}
		numl_t ImageDistanceToFringeSpeedInSamples(numl_t imageDistance, double frequencyHz, TimeFrequencyMetaDataCPtr metaData) const
		{
			return ImageDistanceToDecRaDistance(imageDistance) * AverageUVDistance(metaData, frequencyHz) / (0.5 * (numl_t) metaData->UVW().size());
		}
	private:
		void Clear();
		struct AntennaCache {
			num_t wavelength;
			num_t dx, dy, dz;
		};
		void Image(const class IntegerDomain &frequencies);
		void Image(const IntegerDomain &frequencies, const IntegerDomain &antenna1Domain, const IntegerDomain &antenna2Domain);
		void Image(unsigned frequencyIndex, class AntennaInfo &antenna1, class AntennaInfo &antenna2, SingleFrequencySingleBaselineData *data);

		// This is the fast variant.
		void GetUVPosition(num_t &u, num_t &v, const SingleFrequencySingleBaselineData &data, const AntennaCache &cache);
		void SetUVFTValue(num_t u, num_t v, num_t r, num_t i, num_t weight);


		unsigned long _xRes, _yRes;
		unsigned long _xResFT, _yResFT;
		num_t _uvScaling;
		class Image2D *_uvReal, *_uvImaginary, *_uvWeights;
		class Image2D *_uvFTReal, *_uvFTImaginary;
		class Image2D *_timeFreq;
		MeasurementSet *_measurementSet;
		unsigned _antennaCount, _fieldCount;
		AntennaInfo *_antennas;
		BandInfo _band;
		FieldInfo *_fields;
		size_t _scanCount;
		ImageKind _imageKind;
		bool _invertFlagging, _directFT;
		bool _ignoreBoundWarnings;
};

#endif
