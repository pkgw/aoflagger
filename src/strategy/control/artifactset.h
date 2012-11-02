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

#ifndef RFI_RFISTRATEGY_H
#define RFI_RFISTRATEGY_H 

#include <vector>

#include "../../msio/types.h"
#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

#include "../../types.h"

#include "../algorithms/types.h"

#include "../control/types.h"

class UVImager;

namespace rfiStrategy {
	class	ArtifactSet
	{
		public:
			ArtifactSet(boost::mutex *ioMutex) : _metaData(), _sensitivity(1.0L), _projectedDirectionRad(0.0L), _imageSet(0),
			_imageSetIndex(0), _imager(0),
			_ioMutex(ioMutex),
			_antennaFlagCountPlot(0), _frequencyFlagCountPlot(0),
			_frequencyPowerPlot(0), _timeFlagCountPlot(0), _iterationsPlot(0),
			_polarizationStatistics(0), _baselineSelectionInfo(0), _observatorium(0),
			_model(0),
			_horizontalProfile(), _verticalProfile()
			{
			}

			ArtifactSet(const ArtifactSet &source)
				: _originalData(source._originalData), _contaminatedData(source._contaminatedData),
				_revisedData(source._revisedData), _metaData(source._metaData), _sensitivity(source._sensitivity), _projectedDirectionRad(source._projectedDirectionRad),
				_imageSet(source._imageSet), _imageSetIndex(source._imageSetIndex),
				_imager(source._imager), _ioMutex(source._ioMutex),
				_antennaFlagCountPlot(source._antennaFlagCountPlot), _frequencyFlagCountPlot(source._frequencyFlagCountPlot),
				_frequencyPowerPlot(source._frequencyPowerPlot),
				_timeFlagCountPlot(source._timeFlagCountPlot),
				_iterationsPlot(source._iterationsPlot),
				_polarizationStatistics(source._polarizationStatistics),
				_baselineSelectionInfo(source._baselineSelectionInfo),
				_observatorium(source._observatorium),
				_model(source._model),
				_horizontalProfile(source._horizontalProfile),
				_verticalProfile(source._verticalProfile)
			{
			}

			~ArtifactSet()
			{
			}

			ArtifactSet &operator=(const ArtifactSet &source)
			{
				_originalData = source._originalData;
				_contaminatedData = source._contaminatedData;
				_revisedData = source._revisedData;
				_metaData = source._metaData;
				_sensitivity = source._sensitivity;
				_projectedDirectionRad = source._projectedDirectionRad;
				_imageSet = source._imageSet;
				_imageSetIndex = source._imageSetIndex;
				_imager = source._imager;
				_ioMutex = source._ioMutex;
				_antennaFlagCountPlot = source._antennaFlagCountPlot;
				_frequencyFlagCountPlot = source._frequencyFlagCountPlot;
				_frequencyPowerPlot = source._frequencyPowerPlot;
				_timeFlagCountPlot = source._timeFlagCountPlot;
				_iterationsPlot = source._iterationsPlot;
				_polarizationStatistics = source._polarizationStatistics;
				_baselineSelectionInfo = source._baselineSelectionInfo;
				_observatorium = source._observatorium;
				_model = source._model;
				_horizontalProfile = source._horizontalProfile;
				_verticalProfile = source._verticalProfile;
				return *this;
			}

			void SetOriginalData(const TimeFrequencyData &data)
			{
				_originalData = data;
			}

			void SetRevisedData(const TimeFrequencyData &data)
			{
				_revisedData = data;
			}

			void SetContaminatedData(const TimeFrequencyData &data)
			{
				_contaminatedData = data;
			}

			void SetSensitivity(numl_t sensitivity)
			{
				_sensitivity = sensitivity;
			}
			numl_t Sensitivity() const { return _sensitivity; }

			const TimeFrequencyData &OriginalData() const { return _originalData; }
			TimeFrequencyData &OriginalData() { return _originalData; }

			const TimeFrequencyData &RevisedData() const { return _revisedData; }
			TimeFrequencyData &RevisedData() { return _revisedData; }

			const TimeFrequencyData &ContaminatedData() const { return _contaminatedData; }
			TimeFrequencyData &ContaminatedData() { return _contaminatedData; }

			class ImageSet *ImageSet() const { return _imageSet; }
			void SetImageSet(class ImageSet *imageSet) { _imageSet = imageSet; }
			void SetNoImageSet() { _imageSet = 0; _imageSetIndex = 0; }
			
			class ImageSetIndex *ImageSetIndex() const { return _imageSetIndex; }
			void SetImageSetIndex(class ImageSetIndex *imageSetIndex) { _imageSetIndex = imageSetIndex; }

			class UVImager *Imager() const { return _imager; }
			void SetImager(class UVImager *imager) { _imager = imager; }
			
			bool HasImageSet() const { return _imageSet != 0; }
			bool HasImageSetIndex() const { return _imageSetIndex != 0; }
			bool HasImager() const { return _imager != 0; }

			bool HasMetaData() const { return _metaData != 0; }
			TimeFrequencyMetaDataCPtr MetaData()
			{
				return _metaData;
			}
			void SetMetaData(TimeFrequencyMetaDataCPtr metaData)
			{
				_metaData = metaData;
			}

			boost::mutex &IOMutex()
			{
				return *_ioMutex;
			}

			class AntennaFlagCountPlot *AntennaFlagCountPlot()
			{
				return _antennaFlagCountPlot;
			}
			void SetAntennaFlagCountPlot(class AntennaFlagCountPlot *plot)
			{
				_antennaFlagCountPlot = plot;
			}
			class FrequencyFlagCountPlot *FrequencyFlagCountPlot()
			{
				return _frequencyFlagCountPlot;
			}
			void SetFrequencyFlagCountPlot(class FrequencyFlagCountPlot *plot)
			{
				_frequencyFlagCountPlot = plot;
			}
			class FrequencyPowerPlot *FrequencyPowerPlot()
			{
				return _frequencyPowerPlot;
			}
			void SetFrequencyPowerPlot(class FrequencyPowerPlot *plot)
			{
				_frequencyPowerPlot = plot;
			}
			class TimeFlagCountPlot *TimeFlagCountPlot()
			{
				return _timeFlagCountPlot;
			}
			void SetTimeFlagCountPlot(class TimeFlagCountPlot *plot)
			{
				_timeFlagCountPlot = plot;
			}
			class PolarizationStatistics *PolarizationStatistics()
			{
				return _polarizationStatistics;
			}
			void SetPolarizationStatistics(class PolarizationStatistics *statistics)
			{
				_polarizationStatistics = statistics;
			}
			class BaselineSelector *BaselineSelectionInfo()
			{
				return _baselineSelectionInfo;
			}
			void SetIterationsPlot(class IterationsPlot *iterationsPlot)
			{
				_iterationsPlot = iterationsPlot;
			}
			class IterationsPlot *IterationsPlot()
			{
				return _iterationsPlot;
			}
			void SetBaselineSelectionInfo(class BaselineSelector *baselineSelectionInfo)
			{
				_baselineSelectionInfo = baselineSelectionInfo;
			}
			void SetObservatorium(class Observatorium *observatorium)
			{
				_observatorium = observatorium;
			}
			class Observatorium *Observatorium() const
			{
				return _observatorium;
			}
			void SetModel(class Model *model)
			{
				_model = model;
			}
			class Model *Model() const
			{
				return _model;
			}
			void SetProjectedDirectionRad(numl_t projectedDirectionRad)
			{
				_projectedDirectionRad = projectedDirectionRad;
			}
			numl_t ProjectedDirectionRad() const
			{
				return _projectedDirectionRad;
			}
			const std::vector<num_t> &HorizontalProfile() const { return _horizontalProfile; }
			std::vector<num_t> &HorizontalProfile() { return _horizontalProfile; }
			
			const std::vector<num_t> &VerticalProfile() const { return _verticalProfile; }
			std::vector<num_t> &VerticalProfile() { return _verticalProfile; }
		private:
			TimeFrequencyData _originalData;
			TimeFrequencyData _contaminatedData;
			TimeFrequencyData _revisedData;
			TimeFrequencyMetaDataCPtr _metaData;
			numl_t _sensitivity;
			numl_t _projectedDirectionRad;

			class ImageSet *_imageSet;
			class ImageSetIndex *_imageSetIndex;
			class UVImager *_imager;

			boost::mutex *_ioMutex;
			class AntennaFlagCountPlot *_antennaFlagCountPlot;
			class FrequencyFlagCountPlot *_frequencyFlagCountPlot;
			class FrequencyPowerPlot *_frequencyPowerPlot;
			class TimeFlagCountPlot *_timeFlagCountPlot;
			class IterationsPlot *_iterationsPlot;
			
			class PolarizationStatistics *_polarizationStatistics;
			class BaselineSelector *_baselineSelectionInfo;
			class Observatorium *_observatorium;
			class Model *_model;
			std::vector<num_t> _horizontalProfile, _verticalProfile;
	};
}

#endif //RFI_RFISTRATEGY_H
