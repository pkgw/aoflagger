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
#ifndef RFISTATISTICS_H
#define RFISTATISTICS_H

#include <cstring>
#include <map>
#include <set>

#include <boost/thread/mutex.hpp>

#include "../../msio/image2d.h"
#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"
#include "../../msio/mask2d.h"
#include "../../msio/segmentedimage.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class RFIStatistics {
	public:
		class ChannelInfo {
		public:
			ChannelInfo() : frequencyHz(0), totalCount(0), totalAmplitude(0.0), rfiCount(0), rfiAmplitude(0.0), broadbandRfiCount(0), lineRfiCount(0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0), falsePositiveCount(0), falseNegativeCount(0), truePositiveCount(0), trueNegativeCount(0), falsePositiveAmplitude(0), falseNegativeAmplitude(0)
			{
			}
			ChannelInfo(double _frequencyHz) : frequencyHz(_frequencyHz), totalCount(0), totalAmplitude(0.0), rfiCount(0), rfiAmplitude(0.0), broadbandRfiCount(0), lineRfiCount(0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0), falsePositiveCount(0), falseNegativeCount(0), truePositiveCount(0), trueNegativeCount(0), falsePositiveAmplitude(0), falseNegativeAmplitude(0)
			{
			}
			double frequencyHz;
			long unsigned totalCount;
			long double totalAmplitude;
			long unsigned rfiCount;
			long double rfiAmplitude;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
			long unsigned falsePositiveCount;
			long unsigned falseNegativeCount;
			long unsigned truePositiveCount;
			long unsigned trueNegativeCount;
			long double falsePositiveAmplitude;
			long double falseNegativeAmplitude;
		};
		class TimestepInfo {
		public:
			TimestepInfo() : time(0), totalCount(0), totalAmplitude(0.0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiAmplitude(0.0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0)
			{
			}
			TimestepInfo(double _time) : time(_time), totalCount(0), totalAmplitude(0.0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiAmplitude(0.0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0)
			{
			}
			double time;
			long unsigned totalCount;
			long double totalAmplitude;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double rfiAmplitude;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
		};
		class AmplitudeBin {
		public:
			AmplitudeBin() : centralAmplitude(0.0), count(0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), featureAvgCount(0), featureMaxCount(0), featureIntCount(0), xxCount(0), xyCount(0), yxCount(0), yyCount(0), xxRfiCount(0), xyRfiCount(0), yxRfiCount(0), yyRfiCount(0), stokesQCount(0), stokesUCount(0), stokesVCount(0), falsePositiveCount(0), falseNegativeCount(0), truePositiveCount(0), trueNegativeCount(0)
			{
			}
			double centralAmplitude;
			long unsigned count;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long unsigned featureAvgCount;
			long unsigned featureMaxCount;
			long unsigned featureIntCount;
			long unsigned xxCount;
			long unsigned xyCount;
			long unsigned yxCount;
			long unsigned yyCount;
			long unsigned xxRfiCount;
			long unsigned xyRfiCount;
			long unsigned yxRfiCount;
			long unsigned yyRfiCount;
			long unsigned stokesQCount;
			long unsigned stokesUCount;
			long unsigned stokesVCount;
			long unsigned falsePositiveCount;
			long unsigned falseNegativeCount;
			long unsigned truePositiveCount;
			long unsigned trueNegativeCount;
		};

		class IndexTriple
		{
		public:
			IndexTriple() :
			antenna1Index(0), antenna2Index(0), thirdIndex(0)
			{
			}
			bool operator<(const IndexTriple &rhs) const
			{
				if(thirdIndex == rhs.thirdIndex)
				{
					if(antenna1Index == rhs.antenna1Index)
					{
						return antenna2Index < rhs.antenna2Index;
					}
					else return antenna1Index < rhs.antenna1Index;
				}
				else return thirdIndex < rhs.thirdIndex;
			}
			bool operator==(const IndexTriple &rhs) const
			{
				return thirdIndex == rhs.thirdIndex && antenna1Index == rhs.antenna1Index && antenna2Index == rhs.antenna2Index;
			}
			
			unsigned antenna1Index, antenna2Index;
			double thirdIndex;
		};

		class BaselineTimeInfo
		{
		public:
			BaselineTimeInfo() :
			antenna1Index(0), antenna2Index(0),
				time(0), totalCount(0), rfiCount(0)
			{
			}
			unsigned antenna1Index, antenna2Index;
			double time;
			long unsigned totalCount;
			long unsigned rfiCount;
		};
		
		class BaselineFrequencyInfo
		{
		public:
		BaselineFrequencyInfo() :
			antenna1Index(0), antenna2Index(0),
				centralFrequency(0), totalCount(0),
				rfiCount(0)
			{
			}
			unsigned antenna1Index, antenna2Index;
			double centralFrequency;
			long unsigned totalCount;
			long unsigned rfiCount;
		};
		
		class TimeFrequencyInfo
		{
		public:
			TimeFrequencyInfo() :
				time(0.0), centralFrequency(0.0),
				totalCount(0), rfiCount(0),
				totalAmplitude(0.0), rfiAmplitude(0.0)
			{ }
			double time;
			double centralFrequency;
			long unsigned totalCount;
			long unsigned rfiCount;
			double totalAmplitude;
			double rfiAmplitude;
		};

		class BaselineInfo {
		public:
			BaselineInfo() : antenna1(0), antenna2(0), antenna1Name(), antenna2Name(), baselineLength(0.0), baselineAngle(0.0), count(0), totalAmplitude(0.0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiAmplitude(0.0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0), baselineStatistics(0)
			{
			}
			BaselineInfo(const BaselineInfo &source) : antenna1(source.antenna1), antenna2(source.antenna2), antenna1Name(source.antenna1Name), antenna2Name(source.antenna2Name), baselineLength(source.baselineLength), baselineAngle(source.baselineAngle), count(source.count), totalAmplitude(source.totalAmplitude), rfiCount(source.rfiCount), broadbandRfiCount(source.broadbandRfiCount), lineRfiCount(source.lineRfiCount), rfiAmplitude(source.rfiAmplitude), broadbandRfiAmplitude(source.broadbandRfiAmplitude), lineRfiAmplitude(source.lineRfiAmplitude), baselineStatistics(source.baselineStatistics)
			{
			}
			void operator=(const BaselineInfo &rhs)
			{
				antenna1 = rhs.antenna1;
				antenna2 = rhs.antenna2;
				antenna1Name = rhs.antenna1Name;
				antenna2Name = rhs.antenna2Name;
				baselineLength = rhs.baselineLength;
				baselineAngle = rhs.baselineAngle;
				count = rhs.count;
				totalAmplitude = rhs.totalAmplitude;
				rfiCount = rhs.rfiCount;
				broadbandRfiCount = rhs.broadbandRfiCount;
				lineRfiCount = rhs.lineRfiCount;
				rfiAmplitude = rhs.rfiAmplitude;
				broadbandRfiAmplitude = rhs.broadbandRfiAmplitude;
				lineRfiAmplitude = rhs.lineRfiAmplitude;
				baselineStatistics = rhs.baselineStatistics;
			}
			bool operator<(const BaselineInfo &rhs) const
			{
				return baselineLength < rhs.baselineLength;
			}

			int antenna1, antenna2;
			std::string antenna1Name, antenna2Name;
			double baselineLength;
			double baselineAngle;
			long unsigned count;
			double totalAmplitude;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double rfiAmplitude;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
			RFIStatistics *baselineStatistics;
		};

		RFIStatistics() : _separateBaselineStatistics(false), _compareFlags(false), _polarizationAmplitudeStatistics(false), _filePrefix(""), _channelCountPerSubband(256), _ignoreFirstChannel(true), _performClassification(true), _writeImmediately(false) { }
		~RFIStatistics() { }
		
		void Add(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData);
		void Add(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Mask2DCPtr groundTruthFlagging)
		{
			Add(data, metaData);
			if(metaData->Antenna1().id == metaData->Antenna2().id) {
				boost::mutex::scoped_lock freqLock(_frequencyMapMutex);
				addChannelComparison(_autoChannels, data, metaData, groundTruthFlagging);
				saveChannels(_autoChannels, _filePrefix + "counts-channels-auto.txt");
				freqLock.unlock();
				
				boost::mutex::scoped_lock genLock(_genericMutex);
				addAmplitudeComparison(_autoAmplitudes, data, metaData, groundTruthFlagging);
				saveAmplitudes(_autoAmplitudes, _filePrefix + "counts-amplitudes-auto.txt");
				genLock.unlock();
			} else {
				boost::mutex::scoped_lock freqLock(_frequencyMapMutex);
				addChannelComparison(_crossChannels, data, metaData, groundTruthFlagging);
				saveChannels(_crossChannels, _filePrefix + "counts-channels-cross.txt");
				freqLock.unlock();

				boost::mutex::scoped_lock genLock(_genericMutex);
				addAmplitudeComparison(_crossAmplitudes, data, metaData, groundTruthFlagging);
				saveAmplitudes(_crossAmplitudes, _filePrefix + "counts-amplitudes-cross.txt");
				genLock.unlock();
			}
		}
		void Add(const ChannelInfo &channel, bool autocorrelation);
		void Add(const TimestepInfo &timestep, bool autocorrelation);
		void Add(const AmplitudeBin &amplitudeBin, bool autocorrelation);
		void Add(const BaselineInfo &baseline);
		void Add(const BaselineFrequencyInfo &entry);
		void Add(const BaselineTimeInfo &entry);
		void Add(const TimeFrequencyInfo &entry, bool autocorrelation);

		static long double FitScore(const Image2D &image, const Image2D &fit, Mask2DCPtr mask);
		static long double FitScore(Image2DCPtr image, Image2DCPtr fit, Mask2DCPtr mask);

		static num_t DataQuality(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned startX, unsigned endX);

		static num_t FrequencySNR(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned channel);
		
		void Save()
		{
			save(_filePrefix);
		}
		const std::map<double, class AmplitudeBin> &GetAutoAmplitudes() const
		{
			return _autoAmplitudes;
		}
		const std::map<double, class AmplitudeBin> &GetCrossAmplitudes() const
		{
			return _crossAmplitudes;
		}
		double RFIFractionInCrossChannels() const
		{
			return rfiFraction(_crossChannels);
		}
		double RFIFractionInAutoChannels() const
		{
			return rfiFraction(_autoChannels);
		}
		double RFIFractionInCrossTimeSteps() const
		{
			return rfiFraction(_crossTimesteps);
		}
		double RFIFractionInAutoTimeSteps() const
		{
			return rfiFraction(_autoTimesteps);
		}
		double AmplitudeCrossSlope(double startAmplitude, double endAmplitude) const
		{
			return amplitudeSlope(_crossAmplitudes, startAmplitude, endAmplitude);
		}
		long unsigned AmplitudeCrossCount(double startAmplitude, double endAmplitude)
		{
			return count(_crossAmplitudes, startAmplitude, endAmplitude);
		}
		void SetSeparateBaselineStatistics(bool haveSeparateBaselineStatistics)
		{
			_separateBaselineStatistics = haveSeparateBaselineStatistics;
		}
		bool SeparateBaselineStatistics() const { return _separateBaselineStatistics; }
		void SetPerformClassification(bool performClassification)
		{
			_performClassification = performClassification;
		}
		bool PerformClassification() const { return _performClassification; }

		const std::string &FilePrefix() const { return _filePrefix; }
		void SetFilePrefix(const std::string &filePrefix) { _filePrefix = filePrefix; }

		unsigned ChannelCountPerSubband() const { return _channelCountPerSubband; }
		void SetChannelCountPerSubband(unsigned count) { _channelCountPerSubband = count; }

		bool IgnoreFirstChannel() const { return _ignoreFirstChannel; }
		void SetIgnoreFirstChannel(bool value) { _ignoreFirstChannel = value; }
		
		bool WriteImmediately() const { return _writeImmediately; }
		void SetWriteImmediately(bool value) { _writeImmediately = value; }
	private:
		class FeatureInfo {
		public:
			long double amplitudeSum;
			num_t amplitudeMax;
			size_t sampleCount;
		};
		struct StationInfo {
			StationInfo() : totalRfi(0.0), count(0) { };
			StationInfo(const StationInfo &source) : index(source.index), name(source.name), totalRfi(source.totalRfi), count(source.count) { }
			void operator=(const StationInfo &rhs) { index=rhs.index; name=rhs.name; totalRfi=rhs.totalRfi; count=rhs.count; }
			size_t index;
			std::string name;
			double totalRfi;
			size_t count;
			bool operator<(const StationInfo &rhs) const
			{
				return (totalRfi / (double) count) < (rhs.totalRfi / (double) count);
			}
		};
		typedef std::map<size_t, class FeatureInfo> FeatureMap;
		typedef std::map<int, std::map<int, BaselineInfo> > BaselineMatrix;
		typedef std::map<std::pair<double, double>, TimeFrequencyInfo > TimeFrequencyInfoMap;
		typedef std::map<class IndexTriple, class BaselineFrequencyInfo> BaselineFrequencyInfoMap;
		typedef std::map<class IndexTriple, class BaselineTimeInfo> BaselineTimeInfoMap;
		
		std::map<double, class ChannelInfo> _autoChannels, _crossChannels;
		std::map<double, class TimestepInfo> _autoTimesteps, _crossTimesteps;
		std::map<double, class AmplitudeBin> _autoAmplitudes, _crossAmplitudes;
		TimeFrequencyInfoMap _autoTimeFrequencyInfo, _crossTimeFrequencyInfo;
		BaselineMatrix _baselines;
		BaselineFrequencyInfoMap _baselineFrequencyInfo;
		BaselineTimeInfoMap _baselineTimeInfo;
		bool _separateBaselineStatistics;
		bool _compareFlags;
		bool _polarizationAmplitudeStatistics;
		std::string _filePrefix;
		unsigned _channelCountPerSubband;
		bool _ignoreFirstChannel, _performClassification;
		bool _writeImmediately;
		boost::mutex _genericMutex, _timeMapMutex, _baselineMapMutex, _frequencyMapMutex, _tfMapMutex, _taMapMutex, _afMapMutex;
		
		void addEverything(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask, SegmentedImagePtr segmentedMask, SegmentedImagePtr classifiedMask);
		void addSingleBaseline(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask, SegmentedImagePtr segmentedMask, SegmentedImagePtr classifiedMask, bool save);
		void save(const std::string &baseName)
		{
			saveWithoutBaselines(baseName);
			boost::mutex::scoped_lock genLock(_genericMutex);
			saveBaselines(baseName+"counts-baselines.txt");
			saveBaselinesOrdered(baseName+"counts-obaselines.txt");
			if(_crossChannels.size() > _channelCountPerSubband)
			{
				saveMetaData(baseName+"metadata.tex");
				savePlots(baseName);
			}
			saveBaselineTimeInfo(baseName + "counts-baseltime.txt");
			saveBaselineFrequencyInfo(baseName + "counts-baselfreq.txt");
			saveStationTimeInfo(baseName + "counts-stationstime.txt", baseName + "StationsTime.plt");
		}
		void saveWithoutBaselines(const std::string &baseName)
		{
			boost::mutex::scoped_lock genLock(_genericMutex);
			saveChannels(_autoChannels, baseName+"counts-channels-auto.txt");
			saveTimesteps(_autoTimesteps, baseName+"counts-timesteps-auto.txt");
			saveAmplitudes(_autoAmplitudes, baseName+"counts-amplitudes-auto.txt");
			saveChannels(_crossChannels, baseName+"counts-channels-cross.txt");
			saveTimesteps(_crossTimesteps, baseName+"counts-timesteps-cross.txt");
			saveAmplitudes(_crossAmplitudes, baseName+"counts-amplitudes-cross.txt");
			saveSubbands(_autoChannels, baseName+"counts-subbands-auto.txt");
			saveSubbands(_crossChannels, baseName+"counts-subbands-cross.txt");
			saveTimeIntegrated(_autoTimesteps, baseName+"counts-timeint-auto.txt");
			saveTimeIntegrated(_crossTimesteps, baseName+"counts-timeint-cross.txt");
			saveTimeFrequencyInfo(_autoTimeFrequencyInfo, baseName + "counts-timefreq-auto.txt");
			saveTimeFrequencyInfo(_crossTimeFrequencyInfo, baseName + "counts-timefreq-cross.txt");
		}

		void addChannels(std::map<double, class ChannelInfo> &channels, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addTimesteps(std::map<double, class TimestepInfo> &timesteps, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addBaselines(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask, SegmentedImagePtr segmentedMask, SegmentedImagePtr classifiedMask);

		void addFeatures(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addPolarisations(std::map<double, class AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData);
		void addStokes(std::map<double, class AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData);

		void addChannelComparison(std::map<double, class ChannelInfo> &channels, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Mask2DCPtr groundTruthFlagging);
		void addAmplitudeComparison(std::map<double, class AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, Mask2DCPtr groundTruthFlagging);
		
		void addBaselineFrequencyInfo(TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask);
		void addBaselineTimeInfo(TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask);
		void addTimeFrequencyInfo(TimeFrequencyInfoMap &map, TimeFrequencyMetaDataCPtr metaData, Image2DCPtr image, Mask2DCPtr mask);

		void saveChannels(const std::map<double, class ChannelInfo> &channels, const std::string &filename);
		void saveTimesteps(const std::map<double, class TimestepInfo> &timesteps, const std::string &filename);
		void saveAmplitudes(const std::map<double, class AmplitudeBin> &amplitudes, const std::string &filename);
		void saveBaselines(const std::string &filename);
		void saveBaselinesOrdered(const std::string &filename);
		void saveStations(const std::string &filename);
		void saveSubbands(const std::map<double, class ChannelInfo> &channels, const std::string &filename);
		void saveTimeIntegrated(const std::map<double, class TimestepInfo> &timesteps, const std::string &filename);
		void saveMetaData(const std::string &filename) const;
		void savePlots(const std::string &basename) const;
		void saveBaselineTimeInfo(const std::string &filename);
		void saveBaselineFrequencyInfo(const std::string &filename);
		void saveTimeFrequencyInfo(TimeFrequencyInfoMap &map, const std::string &filename);
		void saveStationTimeInfo(const std::string &filename, const std::string &plotname);
		void saveStationTimeInfoRow(std::ostream &stream, bool &firstRow, std::vector<unsigned long> &totals, std::vector<unsigned long> &rfis, double time);
		void createStationData(std::vector<StationInfo> &stations) const;
		
		std::string getStationName(unsigned index) const
		{
			BaselineMatrix::const_iterator rowIter = _baselines.find(index);
			if(rowIter != _baselines.end())
			{
				const std::map<int, BaselineInfo> &row = rowIter->second;
				if(row.size() > 0)
					return row.begin()->second.antenna1Name;
			}
			for(rowIter=_baselines.begin();rowIter!=_baselines.end();++rowIter)
			{
				const std::map<int, BaselineInfo> &row = rowIter->second;
				std::map<int, BaselineInfo>::const_iterator elementIter = row.find(index);
				if(elementIter != row.end())
					return elementIter->second.antenna2Name;
			}
			std::stringstream s;
			s << "Station " << index;
			return s.str();
		}
	
		double getCentralAmplitude(double amplitude)
		{
			//double decimals = pow(10.0, floor(log10(amplitude)));
			//return round(100.0 * amplitude / decimals) / 100.0 * decimals;
			if(amplitude>=0.0)
				return pow(10.0, round(100.0*log10(amplitude))/100.0);
			else
				return -pow(10.0, round(100.0*log10(-amplitude))/100.0);
		}
		
		double rfiFraction(const std::map<double, class ChannelInfo> &channels) const
		{
			unsigned long totalRFI = 0, total = 0;
			for(std::map<double, class ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
			{
				totalRFI += i->second.rfiCount;
				total += i->second.totalCount;
			}
			return (long double) totalRFI / (long double) total;
		}
		double rfiFraction(const std::map<double, class TimestepInfo> &timesteps) const
		{
			unsigned long totalRFI = 0, total = 0;
			for(std::map<double, class TimestepInfo>::const_iterator i=timesteps.begin();i!=timesteps.end();++i)
			{
				totalRFI += i->second.rfiCount;
				total += i->second.totalCount;
			}
			return (long double) totalRFI / (long double) total;
		}
		double amplitudeSlope(const std::map<double, class AmplitudeBin> &amplitudes, double startAmplitude, double endAmplitude) const
		{
			unsigned long n = 0;
			long double sumX = 0.0, sumXY = 0.0, sumY = 0.0, sumXSquare = 0.0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
			{
				if(i->first > startAmplitude && i->first<endAmplitude)
				{
					double x = log10(i->first);
					double y = log10(i->second.count/i->first);
					++n;
					sumX += x;
					sumXSquare += x * x;
					sumY += y;
					sumXY += x * y;
				}
			}
			return (sumXY - sumX*sumY/n)/(sumXSquare - (sumX*sumX/n));
		}
		long unsigned count(const std::map<double, class AmplitudeBin> &amplitudes, double startAmplitude, double endAmplitude) const
		{
			unsigned long n = 0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
			{
				if(i->first > startAmplitude && i->first<endAmplitude)
					n+=i->second.count;
			}
			return n;
		}
		double lowerQuartile(const std::multiset<double> &numbers)
		{
			double mid = round((double) numbers.size()/2.0);
			size_t lq = (size_t) round(mid/2.0);
			std::multiset<double>::const_iterator iter = numbers.begin();
			for(size_t i=0;i<lq;++i)
				++iter;
			return *iter;
		}
		double upperQuartile(const std::multiset<double> &numbers)
		{
			double mid = round((double) numbers.size()/2.0);
			size_t lq = (size_t) round(mid/2.0);
			std::multiset<double>::const_reverse_iterator iter = numbers.rbegin();
			for(size_t i=0;i<lq;++i)
				++iter;
			return *iter;
		}
		double median(const std::multiset<double> &numbers)
		{
			if(numbers.size()%2==0)
			{
				if(numbers.size()==0) return 0.0;
				size_t midL = (numbers.size()/2)-1;
				std::multiset<double>::const_reverse_iterator iter = numbers.rbegin();
				for(size_t i=0;i<midL;++i)
					++iter;
				double leftValue = *iter;
				++iter;
				return (leftValue + *iter)/2.0;
			} else {
				size_t mid = (size_t) (numbers.size()/2);
				std::multiset<double>::const_reverse_iterator iter = numbers.rbegin();
				for(size_t i=0;i<mid;++i)
					++iter;
				return *iter;
			}
		}
		double sum(const std::multiset<double> &numbers)
		{
			double sum = 0.0;
			for(std::multiset<double>::const_iterator i=numbers.begin();i!=numbers.end();++i)
				sum += *i;
			return sum;
		}
		double avg(const std::multiset<double> &numbers)
		{
			return sum(numbers) / numbers.size();
		}
		std::string toTex(const std::string str) const
		{
			std::string copy = str;
			for(unsigned i=0;i<str.size();++i)
			{
				if(copy[i] == '_') copy[i]=' ';
				if(copy[i] == '\\') copy[i]=' ';
			}
			return copy;
		}

		unsigned startChannel(unsigned imageHeight) const {
			if(imageHeight <= 16)
				return 0;
			else
				return 1;
		}		
};

#endif
