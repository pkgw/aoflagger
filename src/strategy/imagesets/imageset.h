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

#ifndef GUI_IMAGESET_H
#define GUI_IMAGESET_H

#include <string>
#include <cstring>
#include <vector>

#include "../../msio/types.h"
#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

namespace rfiStrategy {

	class ImageSet;
	
	class ImageSetIndex {
		public:
			ImageSetIndex(ImageSet &set) : _set(&set) { }
			virtual ~ImageSetIndex() { }
			virtual void Previous() = 0;
			virtual void Next() = 0;
			virtual void LargeStepPrevious() = 0;
			virtual void LargeStepNext() = 0;
			virtual std::string Description() const = 0;
			virtual bool IsValid() const = 0;
			virtual ImageSetIndex *Copy() const = 0;
			void Reattach(ImageSet &imageSet) { _set = &imageSet; }
		protected:
			ImageSet &imageSet() const { return *_set; }
		private:
			class ImageSet *_set;
	};
	
	class BaselineData {
		public:
			BaselineData(TimeFrequencyData data, TimeFrequencyMetaDataCPtr metaData, const ImageSetIndex &index)
			: _data(data), _metaData(metaData), _index(index.Copy())
			{
			}
			BaselineData(const ImageSetIndex &index)
			: _data(), _metaData(), _index(index.Copy())
			{
			}
			BaselineData(TimeFrequencyData data, TimeFrequencyMetaDataCPtr metaData)
			: _data(data), _metaData(metaData), _index(0)
			{
			}
			BaselineData(TimeFrequencyMetaDataCPtr metaData)
			: _data(), _metaData(metaData), _index(0)
			{
			}
			BaselineData()
			: _data(), _metaData(), _index(0)
			{
			}
			BaselineData(const BaselineData &source)
			: _data(source._data), _metaData(source._metaData), _index(0)
			{
				if(source._index != 0) _index = source._index->Copy();
			}
			~BaselineData()
			{
				if(_index != 0)
					delete _index;
			}
			void operator=(const BaselineData &source)
			{
				if(_index != 0)
					delete _index;
				_data = source._data;
				_metaData = source._metaData;
				_index = source._index->Copy();
			}
			const TimeFrequencyData &Data() const { return _data; }
			void SetData(const TimeFrequencyData &data) { _data = data; }
			
			TimeFrequencyMetaDataCPtr MetaData() const { return _metaData; }
			void SetMetaData(TimeFrequencyMetaDataCPtr metaData) { _metaData = metaData; }

			const ImageSetIndex &Index() const { return *_index; }
			ImageSetIndex &Index() { return *_index; }
			void SetIndex(const ImageSetIndex &newIndex)
			{
				if(_index != 0)
					delete _index;
				_index = newIndex.Copy();
			}
		
		private:
			TimeFrequencyData _data;
			TimeFrequencyMetaDataCPtr _metaData;
			ImageSetIndex *_index;
	};
	
	class ImageSet {
		public:
			virtual ~ImageSet() { };
			virtual ImageSet *Copy() = 0;

			virtual ImageSetIndex *StartIndex() = 0;
			
			/**
			 * Initialize is used to initialize the image set after it has been created and
			 * after all possible options have been set that might influence initialization
			 * (such as number of parts to read).
			 */
			virtual void Initialize() = 0;
			virtual std::string Name() = 0;
			virtual std::string File() = 0;
			virtual void WriteFlags(const ImageSetIndex &/*index*/, TimeFrequencyData &/*data*/)
			{
				throw std::runtime_error("Not implemented");
			}
			static class ImageSet *Create(const std::string &file, BaselineIOMode ioMode, bool readUVW=false);
			static bool IsFitsFile(const std::string &file);
			static bool IsRCPRawFile(const std::string &file);
			static bool IsTKPRawFile(const std::string &file);
			static bool IsRawDescFile(const std::string &file);
			static bool IsParmFile(const std::string &file);
			static bool IsTimeFrequencyStatFile(const std::string &file);
			static bool IsMSFile(const std::string &file);
			static bool IsNoiseStatFile(const std::string &file);
			static bool IsHarishFile(const std::string &file);

			virtual void AddReadRequest(const ImageSetIndex &index) = 0;
			virtual void PerformReadRequests() = 0;
			virtual BaselineData *GetNextRequested() = 0;
			
			void AddWriteFlagsTask(const ImageSetIndex &index, const TimeFrequencyData &data)
			{
				std::vector<Mask2DCPtr> flags;
				for(size_t i=0;i!=data.MaskCount();++i)
					flags.push_back(data.GetMask(i));
				AddWriteFlagsTask(index, flags);
			}
			virtual void AddWriteFlagsTask(const ImageSetIndex &/*index*/, std::vector<Mask2DCPtr> &/*flags*/)
			{
				throw std::runtime_error("Not implemented");
			}
			virtual void PerformWriteFlagsTask()
			{
				throw std::runtime_error("Not implemented");
			}

			void PerformWriteDataTask(const ImageSetIndex &index, const TimeFrequencyData &data)
			{
				std::vector<Image2DCPtr> realImages, imaginaryImages;
				for(size_t i=0;i!=data.PolarisationCount();++i)
				{
					TimeFrequencyData *polData = data.CreateTFDataFromPolarisationIndex(i);
					realImages.push_back(polData->GetRealPart());
					imaginaryImages.push_back(polData->GetImaginaryPart());
					delete polData;
				}
				PerformWriteDataTask(index, realImages, imaginaryImages);
			}
			virtual void PerformWriteDataTask(const ImageSetIndex &/*index*/, std::vector<Image2DCPtr> /*_realImages*/, std::vector<Image2DCPtr> /*_imaginaryImages*/)
			{
				throw std::runtime_error("Not implemented");
			}
	};

}

#endif
