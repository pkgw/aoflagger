#ifndef SPATIALTIMEIMAGESET_H
#define SPATIALTIMEIMAGESET_H

#include <string>
#include <cstring>
#include <sstream>
#include <stack>
#include <stdexcept>

#include "imageset.h"

#include "../../msio/measurementset.h"
#include "../../msio/spatialtimeloader.h"

namespace rfiStrategy {

	class SpatialTimeImageSetIndex : public ImageSetIndex {
		public:
			friend class SpatialTimeImageSet;

			SpatialTimeImageSetIndex(ImageSet &set) : ImageSetIndex(set), _channelIndex(0), _isValid(true)
			{
			}
			inline virtual void Previous();
			inline virtual void Next();
			inline virtual void LargeStepPrevious();
			inline virtual void LargeStepNext();
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Channel " << _channelIndex;
				return s.str();
			}
			virtual bool IsValid() const
			{
				return _isValid;
			}
			virtual ImageSetIndex *Copy() const
			{
				SpatialTimeImageSetIndex *newIndex = new SpatialTimeImageSetIndex(imageSet());
				newIndex->_channelIndex = _channelIndex;
				newIndex->_isValid = _isValid;
				return newIndex;
			}
		private:
			inline class SpatialTimeImageSet &STMSSet() const;
			size_t _channelIndex;
			bool _isValid;
	};
	
	class SpatialTimeImageSet : public ImageSet {
		public:
			SpatialTimeImageSet(const std::string &location) : _set(location), _loader(_set)
			{
			}
			virtual ~SpatialTimeImageSet()
			{
			}
			virtual ImageSet *Copy()
			{
				return 0;
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new SpatialTimeImageSetIndex(*this);
			}
			virtual void Initialize()
			{
			}
			virtual std::string Name()
			{
				return "Spatial time matrix";
			}
			virtual std::string File()
			{
				return _set.Path(); 
			}
			virtual TimeFrequencyData *LoadData(const ImageSetIndex &index)
			{
				const SpatialTimeImageSetIndex &sIndex = static_cast<const SpatialTimeImageSetIndex&>(index);
				TimeFrequencyData *result = new TimeFrequencyData(_loader.Load(sIndex._channelIndex));
				return result;
			}
			virtual void LoadFlags(const ImageSetIndex &/*index*/, TimeFrequencyData &/*destination*/)
			{
			}
			virtual TimeFrequencyMetaDataCPtr LoadMetaData(const ImageSetIndex &/*index*/)
			{
				return TimeFrequencyMetaDataCPtr();
			}
			virtual void WriteFlags(const ImageSetIndex &/*index*/, TimeFrequencyData &/*data*/)
			{
			}
			virtual size_t GetPart(const ImageSetIndex &/*index*/)
			{
				return 0;
			}
			virtual size_t GetAntenna1(const ImageSetIndex &/*index*/)
			{
				return 0;
			}
			virtual size_t GetAntenna2(const ImageSetIndex &/*index*/)
			{
				return 0;
			}
			size_t GetTimeIndexCount()
			{
				return _loader.TimestepsCount();
			}
			size_t GetFrequencyCount()
			{
				return _loader.ChannelCount();
			}
			virtual void AddReadRequest(const ImageSetIndex &index)
			{
				_baseline.push(BaselineData(index));
			}
			virtual void PerformReadRequests()
			{
				TimeFrequencyData *data = LoadData(_baseline.top().Index());
				_baseline.top().SetData(*data);
				_baseline.top().SetMetaData(TimeFrequencyMetaDataPtr());
				delete data;
			}
			virtual BaselineData *GetNextRequested()
			{
				BaselineData data = _baseline.top();
				_baseline.pop();
				return new BaselineData(data);
			}
			virtual void AddWriteFlagsTask(const ImageSetIndex &, std::vector<Mask2DCPtr> &)
			{
				throw std::runtime_error("Not implemented");
			}
			virtual void PerformWriteFlagsTask()
			{
				throw std::runtime_error("Not implemented");
			}
			virtual void PerformWriteDataTask(const ImageSetIndex &, std::vector<Image2DCPtr>, std::vector<Image2DCPtr>)
			{
				throw std::runtime_error("Not implemented");
			}
		private:
			MeasurementSet _set;
			SpatialTimeLoader _loader;
			std::stack<BaselineData> _baseline;
			size_t _cachedTimeIndex;
	};

	void SpatialTimeImageSetIndex::LargeStepPrevious()
	{
		_isValid = false;
	}

	void SpatialTimeImageSetIndex::LargeStepNext()
	{
		_isValid = false;
	}

	void SpatialTimeImageSetIndex::Previous()
	{
		if(_channelIndex > 0)
			--_channelIndex;
		else
		{
			_channelIndex = STMSSet().GetFrequencyCount()-1;
			_isValid = false;
		}
	}

	void SpatialTimeImageSetIndex::Next()
	{
		++_channelIndex;
		if(_channelIndex == STMSSet().GetFrequencyCount())
		{
			_channelIndex = 0;
			_isValid = false;
		}
	}

	class SpatialTimeImageSet &SpatialTimeImageSetIndex::STMSSet() const
	{
		return static_cast<SpatialTimeImageSet&>(imageSet());
	}
}

#endif
