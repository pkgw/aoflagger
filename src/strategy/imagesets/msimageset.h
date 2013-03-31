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
#ifndef MSIMAGESET_H
#define MSIMAGESET_H

#include <set>
#include <string>
#include <stdexcept>

#include "../../msio/antennainfo.h"
#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"
#include "../../msio/baselinereader.h"
#include "../../msio/measurementset.h"

#include "imageset.h"

#include "../../util/aologger.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/

namespace rfiStrategy {

	class MSImageSetIndex : public ImageSetIndex {
		public:
			friend class MSImageSet;
			
			MSImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _sequenceIndex(0), _isValid(true) { }
			
			virtual void Previous();
			virtual void Next();
			virtual void LargeStepPrevious();
			virtual void LargeStepNext();
			virtual std::string Description() const;
			virtual bool IsValid() const { return _isValid; }
			virtual MSImageSetIndex *Copy() const
			{
				MSImageSetIndex *index = new MSImageSetIndex(imageSet());
				index->_sequenceIndex = _sequenceIndex;
				index->_isValid = _isValid;
				return index;
			}
		private:
			size_t _sequenceIndex;
			bool _isValid;
	};
	
	class MSImageSet : public ImageSet {
		public:
			MSImageSet(const std::string &location, BaselineIOMode ioMode) :
				_msFile(location),
				_set(location),
				_reader(),
				_dataColumnName("DATA"), 
				_subtractModel(false),
				_readDipoleAutoPolarisations(true),
				_readDipoleCrossPolarisations(true),
				_readStokesI(false),
				_scanCountPartOverlap(100),
				_readFlags(true),
				_readUVW(false),
				_ioMode(ioMode)
			{
			}
			
			~MSImageSet()
			{
			}

			virtual MSImageSet *Copy()
			{
				MSImageSet *newSet = new MSImageSet(_set.Path(), _ioMode);
				newSet->_reader = _reader;
				newSet->_dataColumnName = _dataColumnName;
				newSet->_subtractModel = _subtractModel;
				newSet->_sequences = _sequences;
				newSet->_bandCount = _bandCount;
				newSet->_fieldCount = _fieldCount;
				newSet->_readDipoleAutoPolarisations = _readDipoleAutoPolarisations;
				newSet->_readDipoleCrossPolarisations = _readDipoleCrossPolarisations;
				newSet->_readStokesI = _readStokesI;
				newSet->_scanCountPartOverlap = _scanCountPartOverlap;
				newSet->_readFlags = _readFlags;
				newSet->_readUVW = _readUVW;
				newSet->_ioMode = _ioMode;
				return newSet;
			}
	
			virtual std::string Name() { return _set.Path(); }
			virtual std::string File() { return _set.Path(); }
			//virtual TimeFrequencyData *LoadData(const ImageSetIndex &index);
			
			virtual void AddReadRequest(const ImageSetIndex &index);
			virtual void PerformReadRequests();
			virtual BaselineData *GetNextRequested();

			virtual void AddWriteFlagsTask(const ImageSetIndex &index, std::vector<Mask2DCPtr> &flags);
			virtual void PerformWriteFlagsTask();

			virtual void Initialize();
	
			size_t GetAntenna1(const ImageSetIndex &index) {
				return _sequences[static_cast<const MSImageSetIndex&>(index)._sequenceIndex].antenna1;
			}
			size_t GetAntenna2(const ImageSetIndex &index) {
				return _sequences[static_cast<const MSImageSetIndex&>(index)._sequenceIndex].antenna2;
			}
			size_t GetBand(const ImageSetIndex &index) {
				return _sequences[static_cast<const MSImageSetIndex&>(index)._sequenceIndex].spw;
			}
			size_t GetField(const ImageSetIndex &index) {
				return _sequences[static_cast<const MSImageSetIndex&>(index)._sequenceIndex].fieldId;
			}
			size_t GetSequenceId(const ImageSetIndex &index) {
				return _sequences[static_cast<const MSImageSetIndex&>(index)._sequenceIndex].sequenceId;
			}
	
			virtual ImageSetIndex *StartIndex() { return new MSImageSetIndex(*this); }

			MSImageSetIndex *Index(size_t a1, size_t a2, size_t b, size_t s)
			{
				MSImageSetIndex *index = new MSImageSetIndex(*this);
				index->_sequenceIndex = FindBaselineIndex(a1, a2, b, s);
				return index;
			}
			
			const std::string &DataColumnName() const { return _dataColumnName; }
			void SetDataColumnName(const std::string &name) {
				if(_reader != 0)
					throw std::runtime_error("Trying to set data column after creating the reader!");
				_dataColumnName = name;
			}

			bool SubtractModel() const { return _subtractModel; }
			void SetSubtractModel(bool subtractModel) {
				if(_reader != 0)
					throw std::runtime_error("Trying to set model subtraction after creating the reader!");
				_subtractModel = subtractModel;
			}

			void SetReadAllPolarisations() throw()
			{
				if(_reader != 0)
					throw std::runtime_error("Trying to set polarization to read after creating the reader!");
				_readDipoleAutoPolarisations = true;
				_readDipoleCrossPolarisations = true;
				_readStokesI = false;
			}
			void SetReadDipoleAutoPolarisations() throw()
			{
				if(_reader != 0)
					throw std::runtime_error("Trying to set polarization to read after creating the reader!");
				_readDipoleAutoPolarisations = true;
				_readDipoleCrossPolarisations = false;
				_readStokesI = false;
			}
			void SetReadStokesI() throw()
			{
				if(_reader != 0)
					throw std::runtime_error("Trying to set polarization to read after creating the reader!");
				_readStokesI = true;
				_readDipoleAutoPolarisations = false;
				_readDipoleCrossPolarisations = false;
			}

			size_t AntennaCount() { return _set.AntennaCount(); }
			class ::AntennaInfo GetAntennaInfo(unsigned antennaIndex) { return _set.GetAntennaInfo(antennaIndex); }
			class ::BandInfo GetBandInfo(unsigned bandIndex)
			{
				return _set.GetBandInfo(bandIndex);
			}
			class ::FieldInfo GetFieldInfo(unsigned fieldIndex)
			{
				return _set.GetFieldInfo(fieldIndex);
			}
			std::vector<double> ObservationTimesVector(const ImageSetIndex &index);
			size_t BandCount() const { return _bandCount; }
			size_t FieldCount() const { return _fieldCount; }
			size_t SequenceCount() const { return _sequencesPerBaselineCount; }
			virtual void WriteFlags(const ImageSetIndex &index, TimeFrequencyData &data);
			void SetReadFlags(bool readFlags) { _readFlags = readFlags; }
			BaselineReaderPtr Reader() { return _reader; }
			virtual void PerformWriteDataTask(const ImageSetIndex &index, std::vector<Image2DCPtr> realImages, std::vector<Image2DCPtr> imaginaryImages)
			{
				const MSImageSetIndex &msIndex = static_cast<const MSImageSetIndex&>(index);
				_reader->PerformDataWriteTask(realImages, imaginaryImages, GetAntenna1(msIndex), GetAntenna2(msIndex), GetBand(msIndex), GetSequenceId(msIndex));
			}
			void SetReadUVW(bool readUVW)
			{
				_readUVW = readUVW;
			}
		private:
			friend class MSImageSetIndex;
			MSImageSet(const std::string &location, BaselineReaderPtr reader) :
				_msFile(location), _set(location), _reader(reader),
				_dataColumnName("DATA"), _subtractModel(false),
				_readDipoleAutoPolarisations(true),
				_readDipoleCrossPolarisations(true),
				_readStokesI(false),
				_scanCountPartOverlap(100),
				_readFlags(true),
				_readUVW(false),
				_ioMode(AutoReadMode)
			{ }
			size_t StartIndex(const MSImageSetIndex &index);
			size_t EndIndex(const MSImageSetIndex &index);
			size_t LeftBorder(const MSImageSetIndex &index);
			size_t RightBorder(const MSImageSetIndex &index);
			void initReader();
			size_t FindBaselineIndex(size_t antenna1, size_t antenna2, size_t band, size_t sequenceId);
			TimeFrequencyMetaDataCPtr createMetaData(const ImageSetIndex &index, std::vector<UVW> &uvw);

			const std::string _msFile;
			MeasurementSet _set;
			BaselineReaderPtr _reader;
			std::string _dataColumnName;
			bool _subtractModel;
			bool _readDipoleAutoPolarisations, _readDipoleCrossPolarisations, _readStokesI;
			std::vector<MeasurementSet::Sequence> _sequences;
			size_t _bandCount, _fieldCount, _sequencesPerBaselineCount;
			size_t _scanCountPartOverlap;
			bool _readFlags, _readUVW;
			BaselineIOMode _ioMode;
			std::vector<BaselineData> _baselineData;
	};

}
	
#endif
