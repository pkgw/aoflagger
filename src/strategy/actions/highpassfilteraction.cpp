#include "highpassfilteraction.h"

#include "../algorithms/highpassfilter.h"

namespace rfiStrategy {

void HighPassFilterAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
{
	TimeFrequencyData &data = artifacts.ContaminatedData();
	if(data.PolarisationCount() != 1)
		throw std::runtime_error("High-pass filtering needs single polarization");
	HighPassFilter filter;
	filter.SetHKernelSigmaSq(_hKernelSigmaSq);
	filter.SetHWindowSize(_windowWidth);
	filter.SetVKernelSigmaSq(_vKernelSigmaSq);
	filter.SetVWindowSize(_windowHeight);
	Mask2DCPtr mask = data.GetSingleMask();
	size_t imageCount = data.ImageCount();
	
	switch(_mode)
	{
	case StoreContaminated:
		for(size_t i=0;i<imageCount;++i)
		{
			data.SetImage(i, filter.ApplyHighPass(data.GetImage(i), mask));
		}
		break;
		
	case StoreRevised:
		TimeFrequencyData revisedData = data;
		for(size_t i=0;i<imageCount;++i)
		{
			revisedData.SetImage(i, filter.ApplyLowPass(revisedData.GetImage(i), mask));
		}
		artifacts.SetRevisedData(revisedData);
		break;
	}
}

}
