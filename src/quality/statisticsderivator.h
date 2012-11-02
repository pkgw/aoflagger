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
#ifndef QUALITY__STATISTICS_DERIVATOR_H
#define QUALITY__STATISTICS_DERIVATOR_H

#include "statisticscollection.h"

#include <complex>
#include <set>

#include "../msio/antennainfo.h"
#include "../msio/timefrequencydata.h"

#include "../strategy/imagesets/noisestatimageset.h"

struct stat;
class StatisticsDerivator
{
	public:
		StatisticsDerivator(const StatisticsCollection &collection) :
			_collection(collection)
		{
		}
		
		std::complex<long double> GetComplexBaselineStatistic(QualityTablesFormatter::StatisticKind kind, unsigned antenna1, unsigned antenna2, unsigned polarization) const
		{
			const DefaultStatistics &statistics = _collection.BaselineStatistics().GetStatistics(antenna1, antenna2);
			return deriveComplex<long double>(kind, statistics, polarization);
		}
		
		std::complex<long double> GetComplexTimeStatistic(QualityTablesFormatter::StatisticKind kind, double time, unsigned polarization) const
		{
			const DefaultStatistics &statistics = _collection.TimeStatistics().find(time)->second;
			return deriveComplex<long double>(kind, statistics, polarization);
		}
	
		std::complex<long double> GetComplexFrequencyStatistic(QualityTablesFormatter::StatisticKind kind, double frequency, unsigned polarization) const
		{
			const DefaultStatistics &statistics = _collection.FrequencyStatistics().find(frequency)->second;
			return deriveComplex<long double>(kind, statistics, polarization);
		}
	
		static std::complex<long double> GetComplexStatistic(QualityTablesFormatter::StatisticKind kind, const DefaultStatistics &statistics, unsigned polarization)
		{
			return deriveComplex<long double>(kind, statistics, polarization);
		}
		
		static long double GetStatisticAmplitude(QualityTablesFormatter::StatisticKind kind, const DefaultStatistics &statistics, unsigned polarization)
		{
			const std::complex<long double> val = GetComplexStatistic(kind, statistics, polarization);
			return sqrtl(val.real()*val.real() + val.imag()*val.imag());
		}
		
		static std::complex<long double> Variance(unsigned long n, std::complex<long double> sum, std::complex<long double> sumP2)
		{
			return deriveVariance<long double>(n, sum, sumP2);
		}
		
		static long double VarianceAmplitude(unsigned long n, std::complex<long double> sum, std::complex<long double> sumP2)
		{
			const std::complex<long double> variance = deriveVariance<long double>(n, sum, sumP2);
			return sqrtl(variance.real()*variance.real() + variance.imag()*variance.imag());
		}
		
		static long double StandardDeviationAmplitude(unsigned long n, std::complex<long double> sum, std::complex<long double> sumP2)
		{
			const std::complex<long double> stdDev = deriveStandardDeviation<long double>(n, sum, sumP2);
			return sqrtl(stdDev.real()*stdDev.real() + stdDev.imag()*stdDev.imag());
		}
		
		std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> CreateTFData(QualityTablesFormatter::StatisticKind kind)
		{
			const std::map<double, std::map<double, DefaultStatistics> > &map = _collection.AllTimeStatistics();
			std::set<double> frequencies;
			std::set<double> timesteps;
			// List the frequencies and timesteps
			for(std::map<double, std::map<double, DefaultStatistics> >::const_iterator i=map.begin();i!=map.end();++i)
			{
				const double frequency = i->first;
				frequencies.insert(frequency);
				
				const std::map<double, DefaultStatistics> &innerMap = i->second;
				for(std::map<double, DefaultStatistics>::const_iterator j=innerMap.begin();j!=innerMap.end();++j)
				{
					const double time = j->first;
					timesteps.insert(time);
				}
			}
			std::map<double, size_t> freqIndices;
			std::map<double, size_t> timeIndices;
			std::vector<double> observationTimes;
			BandInfo band;
			size_t index = 0;
			for(std::set<double>::const_iterator i=frequencies.begin();i!=frequencies.end();++i)
			{
				freqIndices.insert(std::pair<double, size_t>(*i, index));
				ChannelInfo channel;
				channel.frequencyIndex = index;
				channel.frequencyHz = *i;
				band.channels.push_back(channel);
				++index;
			}
			index = 0;
			for(std::set<double>::const_iterator i=timesteps.begin();i!=timesteps.end();++i)
			{
				timeIndices.insert(std::pair<double, size_t>(*i, index));
				observationTimes.push_back(*i);
				++index;
			}
			
			// create the images
			const size_t pCount = _collection.PolarizationCount();
			Image2DPtr realImage[pCount], imagImage[pCount];
			Mask2DPtr mask = Mask2D::CreateSetMaskPtr<true>(timesteps.size(), frequencies.size());
			for(size_t p=0;p<pCount;++p)
			{
				realImage[p] = Image2D::CreateZeroImagePtr(timesteps.size(), frequencies.size());
				imagImage[p] = Image2D::CreateZeroImagePtr(timesteps.size(), frequencies.size());
			}

			// add the statistis
			for(std::map<double, std::map<double, DefaultStatistics> >::const_iterator i=map.begin();i!=map.end();++i)
			{
				const double frequency = i->first;
				const size_t freqIndex = freqIndices.find(frequency)->second;
				
				const std::map<double, DefaultStatistics> &innerMap = i->second;
				for(std::map<double, DefaultStatistics>::const_iterator j=innerMap.begin();j!=innerMap.end();++j)
				{
					const double time = j->first;
					const size_t timeIndex = timeIndices.find(time)->second;
					
					mask->SetValue(timeIndex, freqIndex, false);
					for(size_t p=0;p<pCount;++p)
					{
						const std::complex<num_t> statValue = deriveComplex<num_t>(kind, j->second, p);
						realImage[p]->SetValue(timeIndex, freqIndex, statValue.real());
						imagImage[p]->SetValue(timeIndex, freqIndex, statValue.imag());
					}
				}
			}
			TimeFrequencyData data = TimeFrequencyData::CreateComplexTFData(pCount, (Image2DCPtr*) realImage, (Image2DCPtr*) imagImage);
			data.SetGlobalMask(mask);
			TimeFrequencyMetaDataPtr metaData(new TimeFrequencyMetaData());
			metaData->SetObservationTimes(observationTimes);
			metaData->SetBand(band);
			return std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr>(data, metaData);
		}
		static std::string GetDescription(QualityTablesFormatter::StatisticKind kind)
		{
			switch(kind)
			{
				case QualityTablesFormatter::MeanStatistic: return "Mean";
				case QualityTablesFormatter::VarianceStatistic: return "Variance";
				case QualityTablesFormatter::StandardDeviationStatistic: return "Standard deviation";
				case QualityTablesFormatter::CountStatistic: return "Sample count";
				case QualityTablesFormatter::DMeanStatistic: return "Differential mean";
				case QualityTablesFormatter::DVarianceStatistic: return "Differential variance";
				case QualityTablesFormatter::DStandardDeviationStatistic: return "Differential standard deviation";
				case QualityTablesFormatter::DCountStatistic: return "Sample count in differential statistics";
				case QualityTablesFormatter::RFICountStatistic: return "Sample count affected by RFI";
				case QualityTablesFormatter::RFIRatioStatistic: return "RFI";
				case QualityTablesFormatter::RFIPercentageStatistic: return "RFI";
				case QualityTablesFormatter::SignalToNoiseStatistic: return "SNR";
				default: return "Value";
			}
		}
		static bool HasUnits(QualityTablesFormatter::StatisticKind kind)
		{
			return !GetUnits(kind).empty();
		}
		static std::string GetUnits(QualityTablesFormatter::StatisticKind kind)
		{
			switch(kind)
			{
				case QualityTablesFormatter::MeanStatistic:
				case QualityTablesFormatter::VarianceStatistic:
				case QualityTablesFormatter::StandardDeviationStatistic:
				case QualityTablesFormatter::DMeanStatistic:
				case QualityTablesFormatter::DVarianceStatistic:
				case QualityTablesFormatter::DStandardDeviationStatistic:
					return "arbitrary units";
				case QualityTablesFormatter::RFIPercentageStatistic:
					return "%";
				case QualityTablesFormatter::CountStatistic:
				case QualityTablesFormatter::DCountStatistic:
				case QualityTablesFormatter::RFICountStatistic:
				case QualityTablesFormatter::RFIRatioStatistic:
				case QualityTablesFormatter::SignalToNoiseStatistic:
				default:
					return "";
			}
		}
		static std::string GetDescWithUnits(QualityTablesFormatter::StatisticKind kind)
		{
			std::ostringstream str;
			str << GetDescription(kind);
			if(HasUnits(kind))
				str << " (" << GetUnits(kind) << ")";
			return str.str();
		}
	private:
		template<typename T>
		static std::complex<T> deriveComplex(QualityTablesFormatter::StatisticKind kind, const DefaultStatistics &statistics, unsigned polarization)
		{
			switch(kind)
			{
				case QualityTablesFormatter::CountStatistic:
					return std::complex<T>(statistics.count[polarization], 0.0);
					break;
				case QualityTablesFormatter::MeanStatistic:
					return statistics.Mean<T>(polarization);
					break;
				case QualityTablesFormatter::SumStatistic:
					return statistics.Sum<T>(polarization);
					break;
				case QualityTablesFormatter::SumP2Statistic:
					return statistics.SumP2<T>(polarization);
					break;
				case QualityTablesFormatter::VarianceStatistic:
					return deriveVariance<T>(statistics.count[polarization],
															 	statistics.sum[polarization],
																statistics.sumP2[polarization]);
					break;
				case QualityTablesFormatter::StandardDeviationStatistic:
					return deriveStandardDeviation<T>(statistics.count[polarization],
															 	statistics.sum[polarization],
																statistics.sumP2[polarization]);
					break;
				case QualityTablesFormatter::DCountStatistic:
					return std::complex<T>(statistics.dCount[polarization], 0.0f);
					break;
				case QualityTablesFormatter::DMeanStatistic:
					return statistics.DMean<T>(polarization);
					break;
				case QualityTablesFormatter::DSumStatistic:
					return statistics.DSum<T>(polarization);
					break;
				case QualityTablesFormatter::DSumP2Statistic:
					return statistics.DSumP2<T>(polarization);
					break;
				case QualityTablesFormatter::DVarianceStatistic:
					return deriveVariance<T>(statistics.dCount[polarization],
																statistics.dSum[polarization],
																statistics.dSumP2[polarization]);
					break;
				case QualityTablesFormatter::DStandardDeviationStatistic:
					return deriveStandardDeviation<T>(statistics.dCount[polarization],
																statistics.dSum[polarization],
																statistics.dSumP2[polarization]);
					break;
				case QualityTablesFormatter::RFIRatioStatistic:
					return std::complex<T>((double) statistics.rfiCount[polarization] / (statistics.count[polarization] + statistics.rfiCount[polarization]), 0.0f);
					break;
				case QualityTablesFormatter::RFIPercentageStatistic:
					return std::complex<T>(100.0 * (double) statistics.rfiCount[polarization] / (statistics.count[polarization] + statistics.rfiCount[polarization]), 0.0f);
					break;
				case QualityTablesFormatter::RFICountStatistic:
					return std::complex<T>(statistics.rfiCount[polarization], 0.0f);
					break;
				case QualityTablesFormatter::SignalToNoiseStatistic:
				{
					const std::complex<T> stddev =
						deriveComplex<T>(QualityTablesFormatter::DStandardDeviationStatistic, statistics, polarization);
					return std::complex<T>(fabsl(statistics.Mean<T>(polarization).real() / stddev.real()), fabsl(statistics.Mean<T>(polarization).imag() / stddev.imag()));
					break;
				}
				default:
					throw std::runtime_error("Can not derive requested statistic");
			}
		}
		
		template<typename T>
		static std::complex<T> deriveVariance(unsigned long n, std::complex<long double> sum, std::complex<long double> sumP2)
		{
			return std::complex<T>(deriveVarianceSingle(n, sum.real(), sumP2.real()),
															 deriveVarianceSingle(n, sum.imag(), sumP2.imag()));
		}
		
		template<typename T>
		static std::complex<T> deriveStandardDeviation(unsigned long n, std::complex<long double> sum, std::complex<long double> sumP2)
		{
			return std::complex<T>(deriveStandardDeviationSingle(n, sum.real(), sumP2.real()),
															 deriveStandardDeviationSingle(n, sum.imag(), sumP2.imag()));
		}
		
		template<typename T>
		static T deriveVarianceSingle(unsigned long n, T sum, T sumP2)
		{
			T sumMeanSquared = sum * sum / n;
			return (sumP2 - sumMeanSquared) / (n-1.0);
		}
		
		template<typename T>
		static T deriveStandardDeviationSingle(unsigned long n, T sum, T sumP2)
		{
			T sumMeanSquared = sum * sum / n;
			return sqrt((sumP2 - sumMeanSquared) / n);
		}
		
		const StatisticsCollection &_collection;
};

#endif
