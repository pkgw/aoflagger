#ifndef OBSERVATORIUM_H
#define OBSERVATORIUM_H

#include <vector>

#include "../msio/antennainfo.h"

class Observatorium
{
	public:
		void AddAntenna(AntennaInfo &antenna)
		{
			_antennae.push_back(antenna);
		}
		size_t AntennaCount() const { return _antennae.size(); }
		const AntennaInfo &GetAntenna(size_t index) const { return _antennae[index]; }
		
		void SetChannelWidthHz(double channelWidthHz)
		{
			_channelWidthHz = channelWidthHz;
		}
		double ChannelWidthHz() const { return _channelWidthHz; }
		
		const class BandInfo &BandInfo() const
		{
			return _bandInfo;
		}
	protected:
		class BandInfo &GetBandInfo() { return _bandInfo; }
	private:
		std::vector<AntennaInfo> _antennae;
		double _channelWidthHz;
		class BandInfo _bandInfo;
};

struct WSRTObservatorium : public Observatorium
{
	explicit WSRTObservatorium(size_t channelCount = 16*4, double bandwidthHz = 2500000.0 * 16.0)
	{
		AntennaInfo antennas[14];
		for(unsigned i=0;i<14;++i)
		{
			WSRTCommon(antennas[i]);
			WSRTn(i, antennas[i]);
			AddAntenna(antennas[i]);
		}
		SetChannelWidthHz(bandwidthHz / channelCount);
		initBand(channelCount);
	}
	explicit WSRTObservatorium(size_t antenna1, size_t antenna2, size_t channelCount = 16*4)
	{
		AntennaInfo antennas[2];
		WSRTCommon(antennas[0]);
		WSRTCommon(antennas[1]);
		WSRTn(antenna1, antennas[0]);
		WSRTn(antenna2, antennas[1]);
		AddAntenna(antennas[0]);
		AddAntenna(antennas[1]);
		SetChannelWidthHz(10000.0 * 256.0 * 16.0 / channelCount);
		initBand(channelCount);
	}

	private:
		void WSRTCommon(AntennaInfo &antenna)
		{
			antenna.diameter = 25;
			antenna.mount = "equatorial";
			antenna.station = "WSRT";
		}
		void WSRTn(size_t antennaIndex, AntennaInfo &antenna)
		{
			switch(antennaIndex)
			{
				case 0:
					antenna.id = 0;
					antenna.name = "RT0";
					antenna.position.x = 3.82876e+06;
					antenna.position.y = 442449;
					antenna.position.z = 5.06492e+06;
				break;
				case 1:
					antenna.id = 1;
					antenna.name = "RT1";
					antenna.position.x = 3.82875e+06;
					antenna.position.y = 442592;
					antenna.position.z = 5.06492e+06;
				break;
				case 2:
					antenna.id = 2;
					antenna.name = "RT2";
					antenna.position.x = 3.82873e+06;
					antenna.position.y = 442735;
					antenna.position.z = 5.06492e+06;
				break;
				case 3:
					antenna.id = 3;
					antenna.name = "RT3";
					antenna.position.x = 3.82871e+06;
					antenna.position.y = 442878;
					antenna.position.z = 5.06492e+06;
				break;
				case 4:
					antenna.id = 4;
					antenna.name = "RT4";
					antenna.position.x = 3.8287e+06;
					antenna.position.y = 443021;
					antenna.position.z = 5.06492e+06;
				break;
				case 5:
					antenna.id = 5;
					antenna.name = "RT5";
					antenna.position.x = 3.82868e+06;
					antenna.position.y = 443164;
					antenna.position.z = 5.06492e+06;
				break;
				case 6:
					antenna.id = 6;
					antenna.name = "RT6";
					antenna.position.x = 3.82866e+06;
					antenna.position.y = 443307;
					antenna.position.z = 5.06492e+06;
				break;
				case 7:
					antenna.id = 7;
					antenna.name = "RT7";
					antenna.position.x = 3.82865e+06;
					antenna.position.y = 443450;
					antenna.position.z = 5.06492e+06;
				break;
				case 8:
					antenna.id = 8;
					antenna.name = "RT8";
					antenna.position.x = 3.82863e+06;
					antenna.position.y = 443593;
					antenna.position.z = 5.06492e+06;
				break;
				case 9:
					antenna.id = 9;
					antenna.name = "RT9";
					antenna.position.x = 3.82861e+06;
					antenna.position.y = 443736;
					antenna.position.z = 5.06492e+06;
				break;
				case 10:
					antenna.id = 10;
					antenna.name = "RTA";
					antenna.position.x = 3.8286e+06;
					antenna.position.y = 443832;
					antenna.position.z = 5.06492e+06;
				break;
				case 11:
					antenna.id = 11;
					antenna.name = "RTB";
					antenna.position.x = 3.82859e+06;
					antenna.position.y = 443903;
					antenna.position.z = 5.06492e+06;
				break;
				case 12:
					antenna.id = 12;
					antenna.name = "RTC";
					antenna.position.x = 3.82845e+06;
					antenna.position.y = 445119;
					antenna.position.z = 5.06492e+06;
				break;
				case 13:
					antenna.id = 13;
					antenna.name = "RTD";
					antenna.position.x = 3.82845e+06;
					antenna.position.y = 445191;
					antenna.position.z = 5.06492e+06;
				break;
			}
		}
		void initBand(size_t channelCount)
		{
			GetBandInfo().windowIndex = 0;
			for(size_t i=0;i<channelCount;++i)
			{
				ChannelInfo channel;
				channel.frequencyIndex = i;
				channel.channelWidthHz = ChannelWidthHz();
				channel.frequencyHz = 147000000.0 - 20000000.0 + ChannelWidthHz() * i;
				GetBandInfo().channels.push_back(channel);
			}
		}
};

#endif
