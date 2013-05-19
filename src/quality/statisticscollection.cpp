#include "statisticscollection.h"

template<bool IsDiff>
void StatisticsCollection::addTimeAndBaseline(unsigned antenna1, unsigned antenna2, double time, double centralFrequency, int polarization, const float *reals, const float *imags, const bool *isRFI, const bool* origFlags, unsigned nsamples, unsigned step, unsigned stepRFI, unsigned stepFlags)
{
	unsigned long rfiCount = 0;
	unsigned long count = 0;
	long double sum_R = 0.0, sum_I = 0.0;
	long double sumP2_R = 0.0, sumP2_I = 0.0;
	for(unsigned j=0;j<nsamples;++j)
	{
		if (!*origFlags) {
			if(std::isfinite(*reals) && std::isfinite(*imags))
			{
				if(*isRFI)
				{
					++rfiCount;
				} else {
					const long double rVal = *reals;
					const long double iVal = *imags;
					++count;
					sum_R += rVal;
					sum_I += iVal;
					sumP2_R += rVal*rVal;
					sumP2_I += iVal*iVal;
				}
			}
		}
		reals += step;
		imags += step;
		isRFI += stepRFI;
		origFlags += stepFlags;
	}
	
	if(antenna1 != antenna2)
	{
		DefaultStatistics &timeStat = getTimeStatistic(time, centralFrequency);
		addToStatistic<IsDiff>(timeStat, polarization, count, sum_R, sum_I, sumP2_R, sumP2_I, rfiCount);
	}
	DefaultStatistics &baselineStat = getBaselineStatistic(antenna1, antenna2, centralFrequency);
	addToStatistic<IsDiff>(baselineStat, polarization, count, sum_R, sum_I, sumP2_R, sumP2_I, rfiCount);
}

template<bool IsDiff>
void StatisticsCollection::addFrequency(unsigned band, int polarization, const float *reals, const float *imags, const bool *isRFI, const bool *origFlags, unsigned nsamples, unsigned step, unsigned stepRFI, unsigned stepFlags, bool shiftOneUp)
{
	std::vector<DefaultStatistics *> &bandStats = _bands.find(band)->second;
	const unsigned fAdd = shiftOneUp ? 1 : 0;
	for(unsigned j=0;j<nsamples;++j)
	{
		if (!*origFlags)
		{
			if(std::isfinite(*reals) && std::isfinite(*imags))
			{
				DefaultStatistics &freqStat = *bandStats[j + fAdd];
				if(*isRFI)
				{
					addToStatistic<IsDiff>(freqStat, polarization, 0, 0.0, 0.0, 0.0, 0.0, 1);
				} else {
					const long double r = *reals, i = *imags;
					addToStatistic<IsDiff>(freqStat, polarization, 1, r, i, r*r, i*i, 0);
				}
			}
		}
		isRFI += stepRFI;
		origFlags += stepFlags;
		reals += step;
		imags += step;
	}
}

void StatisticsCollection::Add(unsigned antenna1, unsigned antenna2, double time, unsigned band, int polarization, const float *reals, const float *imags, const bool *isRFI, const bool* origFlags, unsigned nsamples, unsigned step, unsigned stepRFI, unsigned stepFlags)
{
	if(nsamples == 0) return;
	
	const double centralFrequency = _centralFrequencies.find(band)->second;
	
	addTimeAndBaseline<false>(antenna1, antenna2, time, centralFrequency, polarization, reals, imags, isRFI, origFlags, nsamples, step, stepRFI, stepFlags);
	if(antenna1 != antenna2)
		addFrequency<false>(band, polarization, reals, imags, isRFI, origFlags, nsamples, step, stepRFI, stepFlags, false);
	
	// Allocate vector with length nsamples, so there is
	// a diff element, even if nsamples=1.
	std::vector<float> diffReals(nsamples);
	std::vector<float> diffImags(nsamples);
	bool *diffRFIFlags  = new bool[nsamples];
	bool *diffOrigFlags = new bool[nsamples];
	for (unsigned i=0;i<nsamples-1;++i)
	{
		diffReals[i] = (reals[(i+1)*step] - reals[i*step]) * M_SQRT1_2;
		diffImags[i] = (imags[(i+1)*step] - imags[i*step]) * M_SQRT1_2;
		diffRFIFlags[i] = isRFI[i*stepRFI] | isRFI[(i+1)*stepRFI];
		diffOrigFlags[i] = origFlags[i*stepFlags] | origFlags[(i+1)*stepFlags];
	}
	addTimeAndBaseline<true>(antenna1, antenna2, time, centralFrequency, polarization, &(diffReals[0]), &(diffImags[0]), diffRFIFlags, diffOrigFlags, nsamples-1, 1, 1, 1);
	if(antenna1 != antenna2)
	{
		addFrequency<true>(band, polarization, &(diffReals[0]), &(diffImags[0]), diffRFIFlags, diffOrigFlags, nsamples-1, 1, 1, 1, false);
		addFrequency<true>(band, polarization, &(diffReals[0]), &(diffImags[0]), diffRFIFlags, diffOrigFlags, nsamples-1, 1, 1, 1, true);
	}
	delete[] diffRFIFlags;
	delete[] diffOrigFlags;
}

void StatisticsCollection::AddImage(unsigned antenna1, unsigned antenna2, const double *times, unsigned band, int polarization, const Image2DCPtr &realImage, const Image2DCPtr &imagImage, const Mask2DCPtr &rfiMask, const Mask2DCPtr &correlatorMask)
{
	if(realImage->Width() == 0 || realImage->Height() == 0) return;

	const double centralFrequency = _centralFrequencies.find(band)->second;
	DefaultStatistics &baselineStat = getBaselineStatistic(antenna1, antenna2, centralFrequency);
	std::vector<DefaultStatistics *> &bandStats = _bands.find(band)->second;
	std::vector<DefaultStatistics *> timeStats(realImage->Width());
	
	for(size_t t=0; t!=realImage->Width(); ++t)
		timeStats[t] = &getTimeStatistic(times[t], centralFrequency);
	
	for(size_t f=0; f<realImage->Height(); ++f)
	{
		DefaultStatistics &freqStat = *bandStats[f];
		const bool
			*origFlags = correlatorMask->ValuePtr(0, f),
			*nextOrigFlags = origFlags + correlatorMask->Stride(),
			*isRFI = rfiMask->ValuePtr(0, f),
			*isNextRFI = isRFI + rfiMask->Stride();
		const float
			*reals = realImage->ValuePtr(0, f),
			*imags = imagImage->ValuePtr(0, f),
			*nextReal = reals + realImage->Stride(),
			*nextImag = imags + imagImage->Stride();
		for(size_t t=0; t<realImage->Width(); ++t) 
		{
			if (!*origFlags && std::isfinite(*reals) && std::isfinite(*imags))
			{
				long double real = *reals, imag = *imags;
				
				if(*isRFI)
				{
					if(antenna1 != antenna2)
					{
						++timeStats[t]->rfiCount[polarization];
						++freqStat.rfiCount[polarization];
					}
					++baselineStat.rfiCount[polarization];
				} else {
					long double
						realSq = real*real,
						imagSq = imag*imag;
					
					if(antenna1 != antenna2)
					{
						addSingleNonRFISampleToStatistic<false>(*timeStats[t], polarization, real, imag, realSq, imagSq);
						addSingleNonRFISampleToStatistic<false>(freqStat, polarization, real, imag, realSq, imagSq);
					}
					addSingleNonRFISampleToStatistic<false>(baselineStat, polarization, real, imag, realSq, imagSq);
				}
				
				if(f != realImage->Height()-1)
				{
					DefaultStatistics &nextFreqStat = *bandStats[f+1];
					if(!*nextOrigFlags && std::isfinite(*nextReal) && std::isfinite(*nextImag))
					{
						real = (*nextReal - *reals) * M_SQRT1_2;
						imag = (*nextImag - *imags) * M_SQRT1_2;
						
						if(!(*isRFI || *isNextRFI))
						{
							long double
								realSq = real*real,
								imagSq = imag*imag;
							
							if(antenna1 != antenna2)
							{
								addSingleNonRFISampleToStatistic<true>(*timeStats[t], polarization, real, imag, realSq, imagSq);
								addSingleNonRFISampleToStatistic<true>(freqStat, polarization, real, imag, realSq, imagSq);
								addSingleNonRFISampleToStatistic<true>(nextFreqStat, polarization, real, imag, realSq, imagSq);
							}

							addSingleNonRFISampleToStatistic<true>(baselineStat, polarization, real, imag, realSq, imagSq);
						}
					}
				}
			}
			
			++origFlags;
			++isRFI;
			++reals;
			++imags;
			
			++nextOrigFlags;
			++isNextRFI;
			++nextReal;
			++nextImag;
		}
	}
}
