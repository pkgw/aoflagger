#include <iostream>

#include <fftw3.h>

#include "remote/clusteredobservation.h"
#include "remote/observationtimerange.h"
#include "remote/processcommander.h"

#include "msio/system.h"

#include "util/lane.h"

#include "imaging/uvimager.h"

#include "strategy/algorithms/convolutions.h"

using namespace std;
using namespace aoRemote;

lane<ObservationTimerange*> *readLane;
lane<ObservationTimerange*> *writeLane;

boost::mutex commanderMutex;
ProcessCommander *commander;

fftw_plan fftPlanForward, fftPlanBackward;
const size_t rowCountPerRequest = 128;

// fringe size is given in units of wavelength / fringe. Fringes smaller than that will be filtered.
double filterFringeSize;

bool isFilterSizeInChannels;

/**
 * This function returns the distance between the u,v points of the highest and lowest frequencies.
 * The returned distance is in wavelengths.
 */
double uvDist(double u, double v, double frequencyWidth)
{
	const double
		ud = frequencyWidth * u,
		vd = frequencyWidth * v;
	return sqrt(ud * ud + vd * vd) / UVImager::SpeedOfLight();
}

void workThread()
{
	ObservationTimerange *timerange;
	
	// These are for diagnostic info
	double maxFilterSizeInChannels = 0.0, minFilterSizeInChannels = 1e100;
	
	if(readLane->read(timerange))
	{
		const size_t channelCount = timerange->ChannelCount();
		const unsigned polarizationCount = timerange->PolarizationCount();
		fftw_complex
			*fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount),
			*fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount);
		do
		{
			for(size_t t=0;t<timerange->TimestepCount();++t)
			{
				if(timerange->Antenna1(t) != timerange->Antenna2(t))
				{
					// Calculate the frequencies to filter
					double u = timerange->U(t), v = timerange->V(t);
					double limitFrequency = isFilterSizeInChannels ?
						(channelCount / filterFringeSize) :
						(uvDist(u, v, timerange->FrequencyWidth()) / filterFringeSize);
					
					if(limitFrequency*2 <= channelCount) // otherwise no frequencies had to be removed
					{
						if(limitFrequency > maxFilterSizeInChannels) maxFilterSizeInChannels = limitFrequency;
						if(limitFrequency < minFilterSizeInChannels) minFilterSizeInChannels = limitFrequency;
						for(unsigned p=0;p<polarizationCount;++p)
						{
							// Copy data in buffer
							num_t *realPtr = timerange->RealData(t) + p;
							num_t *imagPtr = timerange->ImagData(t) + p;
							
							for(size_t c=0;c<channelCount;++c)
							{
								fftIn[c][0] = *realPtr;
								fftIn[c][1] = *imagPtr;
								realPtr += polarizationCount;
								imagPtr += polarizationCount;
							}
							
							fftw_execute_dft(fftPlanForward, fftIn, fftOut);
							size_t filterIndexSize = (limitFrequency > 1.0) ? (size_t) ceil(limitFrequency/2.0) : 1;
							// Remove the high frequencies [filterIndexSize : n-filterIndexSize]
							for(size_t f=filterIndexSize;f<channelCount - filterIndexSize;++f)
							{
								fftOut[f][0] = 0.0;
								fftOut[f][1] = 0.0;
							}
							fftw_execute_dft(fftPlanBackward, fftOut, fftIn);

							// Copy data back; fftw multiplies data with n, so divide by n.
							double factor = 1.0 / (double) channelCount;
							realPtr = timerange->RealData(t) + p;
							imagPtr = timerange->ImagData(t) + p;
							for(size_t c=0;c<channelCount;++c)
							{
								*realPtr = fftIn[c][0] * factor;
								*imagPtr = fftIn[c][1] * factor;
								realPtr += polarizationCount;
								imagPtr += polarizationCount;
							}
						}
					}
				}
			}
			writeLane->write(timerange);
		} while(readLane->read(timerange));
		fftw_free(fftIn);
		fftw_free(fftOut);
	}
	std::cout << "Worker finished. Filtersize range in channel: " << minFilterSizeInChannels << "-" << maxFilterSizeInChannels << '\n';
}

void readThreadFunction(ObservationTimerange &timerange, const size_t &totalRows)
{
	std::vector<MSRowDataExt*> rowBuffer(commander->Observation().Size());
	for(size_t i=0;i<commander->Observation().Size();++i)
		rowBuffer[i] = new MSRowDataExt[rowCountPerRequest];

	size_t currentRow = 0;
	while(currentRow < totalRows)
	{
		size_t currentRowCount = rowCountPerRequest;
		if(currentRow + currentRowCount > totalRows)
			currentRowCount = totalRows - currentRow;
		timerange.SetZero();
		
		boost::mutex::scoped_lock lock(commanderMutex);
		std::cout << "Reading... " << std::flush;
		commander->PushReadDataRowsTask(timerange, currentRow, currentRowCount, &rowBuffer[0]);
		commander->Run(false);
		commander->CheckErrors();
		std::cout << "Done.\n" << std::flush;
		lock.unlock();
		
		currentRow += currentRowCount;
		cout << "Read " << currentRow << '/' << totalRows << '\n';
		readLane->write(new ObservationTimerange(timerange));
	}
	for(size_t i=0;i<commander->Observation().Size();++i)
		delete[] rowBuffer[i];
}

void writeThreadFunction()
{
	const ClusteredObservation &obs = commander->Observation();
	std::vector<MSRowDataExt*> rowBuffer(obs.Size());
	for(size_t i=0;i<obs.Size();++i)
		rowBuffer[i] = new MSRowDataExt[rowCountPerRequest];
		
	ObservationTimerange *timerange;
	if(writeLane->read(timerange))
	{
		for(size_t i=0;i<obs.Size();++i)
		{
			for(size_t row=0;row<rowCountPerRequest;++row)
				rowBuffer[i][row] = MSRowDataExt(timerange->PolarizationCount(), timerange->Band(i).channels.size());
		}
		do {
			boost::mutex::scoped_lock lock(commanderMutex);
			std::cout << "Writing... " << std::flush;
			commander->PushWriteDataRowsTask(*timerange, &rowBuffer[0]);
			commander->Run(false);
			commander->CheckErrors();
			std::cout << "Done.\n" << std::flush;
			lock.unlock();
			
			delete timerange;
		} while(writeLane->read(timerange));
	}
	std::cout << "Writer thread finished.\n";
}

void initializeFFTW(size_t channelCount)
{
	fftw_complex *fftIn, *fftOut;
	
	fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount);
	fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount);
	fftPlanForward = fftw_plan_dft_1d(channelCount, fftIn, fftOut, FFTW_FORWARD, FFTW_MEASURE);
	fftPlanBackward = fftw_plan_dft_1d(channelCount, fftIn, fftOut, FFTW_BACKWARD, FFTW_MEASURE);

	fftw_free(fftIn);
	fftw_free(fftOut);
}

void deinitializeFFTW()
{
	fftw_destroy_plan(fftPlanForward);
	fftw_destroy_plan(fftPlanBackward);
}

int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		cerr << "Usage: aofrequencyfilter <reffile> <mode> <filterfringesize>\n"
		"\tmode can be 'inChannels' (CH) or in uv wavelengths (UV)\n";
	}
	else {
		string modeStr(argv[2]);
		if(modeStr == "CH")
			isFilterSizeInChannels = true;
		else if(modeStr == "UV")
			isFilterSizeInChannels = false;
		else throw std::runtime_error("Bad mode");
		
		filterFringeSize = atof(argv[3]);
		ClusteredObservation *obs = ClusteredObservation::Load(argv[1]);
		commander = new ProcessCommander(*obs);
		commander->PushReadAntennaTablesTask();
		commander->PushReadBandTablesTask();
		commander->Run(false);
		commander->CheckErrors();
		
		ObservationTimerange timerange(*obs);
		const std::vector<BandInfo> &bands = commander->Bands();
		for(size_t i=0; i!=bands.size(); ++i)
			timerange.SetBandInfo(i, bands[i]);
		
		const unsigned processorCount = System::ProcessorCount();
		cout << "CPUs: " << processorCount << '\n';
		unsigned polarizationCount = commander->PolarizationCount();
		cout << "Polarization count: " << polarizationCount << '\n';
		
		timerange.Initialize(polarizationCount, rowCountPerRequest);
		
		cout << "Initializing FFTW..." << std::flush;
		initializeFFTW(timerange.ChannelCount());
		cout << " Done.\n";
		
		// We ask for "0" rows, which means we will ask for the total number of rows
		commander->PushReadDataRowsTask(timerange, 0, 0, 0);
		commander->Run(false);
		commander->CheckErrors();
		const size_t totalRows = commander->RowsTotal();
		cout << "Total rows to filter: " << totalRows << '\n';
		
		readLane = new lane<ObservationTimerange*>(processorCount);
		writeLane = new lane<ObservationTimerange*>(processorCount);
		
		// Start worker threads
		std::vector<boost::thread*> threads(processorCount);
		for(size_t i=0; i<processorCount; ++i)
		{
			threads[i] = new boost::thread(&workThread);
		}
		boost::thread writeThread(&writeThreadFunction);
		
		readThreadFunction(timerange, totalRows);
		
		// Shut down read workers
		readLane->write_end();
		for(size_t i=0; i<processorCount; ++i)
		{
			threads[i]->join();
		}
		delete readLane;
		
		// Shut down write worker
		writeLane->write_end();
		writeThread.join();
		delete writeLane;
		
		// Clean
		delete commander;
		delete obs;
	}
}
