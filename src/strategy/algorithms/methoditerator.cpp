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
#include "methoditerator.h"
/*
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <deque>
#include <iostream>

#include "../../msio/antennainfo.h"
#include "../../msio/image2d.h"
#include "../../msio/pngfile.h"

#include "../../util/plot.h"
#include "../../util/statwriter.h"
#include "../../util/stopwatch.h"

#include "thresholdconfig.h"
#include "thresholdtools.h"
#include "surfacefitmethod.h"

MethodIterator::MethodIterator() : _writeStats(true), _savePNGs(true), _thresholdConfig(0), _verbose(false), _bandInfo(0)
{
}


MethodIterator::~MethodIterator()
{
}

struct ExecuteFitFunc {
	std::deque<unsigned> *_tasks;
	boost::mutex *_mutex;
	SurfaceFitMethod *_method;
	ExecuteFitFunc(std::deque<unsigned> *tasks, boost::mutex *mutex, SurfaceFitMethod *method)
		: _tasks(tasks), _mutex(mutex), _method(method)
	{
	}
	void operator()() {
		boost::mutex::scoped_lock lock(*_mutex);
		while(!_tasks->empty()) {
			unsigned task = _tasks->front();
			_tasks->pop_front();
			lock.unlock();
			_method->PerformFit(task);
			lock.lock();
		}
		lock.unlock();
	}
};

void MethodIterator::ExecuteBackgroundFit(SurfaceFitMethod &method, unsigned threadCount)
{
	boost::mutex mutex;
	std::deque<unsigned> tasks;
	for(unsigned i=0;i<method.TaskCount();++i)
		tasks.push_back(i);

	boost::thread_group group;
	for(unsigned i=0;i<threadCount;++i) {
		ExecuteFitFunc threadFunc(&tasks, &mutex, &method);
		group.create_thread(threadFunc);
	}
	group.join_all();
} 

void MethodIterator::IterateBackgroundFit(class SurfaceFitMethod &method, unsigned threadCount, const std::string &name, unsigned iterationCount)
{
	unsigned imageCount = 0;

	if(_verbose)
		std::cout << "Initializing background fitting method..." << std::endl;

	_statWriter = 0;
	if(_writeStats) {
		std::stringstream statname;
		statname << name << "-" << _input.ImageWidth() << "-stats";
		_statWriter = new StatWriter(statname.str());
	}

	_mask = Mask2D::CreateSetMaskPtr<false>(_input.ImageWidth(), _input.ImageHeight());
	_input.SetGlobalMask(_mask);
	method.Initialize(_input);

	if(_writeStats) {
		_statWriter->NewMethod(name + "-iteration");
		for(unsigned imageIndex=0;imageIndex < imageCount;++imageIndex) {
			char c = (char) ((int) 'A' + imageIndex);
	
			_statWriter->NewMethod(name + c + "-mean");
			_statWriter->NewMethod(name + c + "-variance");
			_statWriter->NewMethod(name + c + "-wMean");
			_statWriter->NewMethod(name + c + "-wVariance");
			_statWriter->NewMethod(name + c + "-min");
			_statWriter->NewMethod(name + c + "-max");
			_statWriter->NewMethod(name + c + "-rfi-percentage");
			_statWriter->NewMethod(name + c + "-rfi-overlap");
			_statWriter->NewMethod(name + c + "-rfi-original");
		}
		_statWriter->SetValueInc(0.0);
	}

	if(_savePNGs) {
		std::stringstream flagfn1;
		flagfn1 << name << "-" << _input.ImageWidth() << "-flag-00-original.png";
		SaveFlaggingToPng(_input.GetSingleImage(), _originalFlagging, flagfn1.str(), _bandInfo, true);
		std::stringstream flagfn2;
		flagfn2 << name << "-" << _input.ImageWidth() << "-flag-00-original-nf.png";
		SaveFlaggingToPng(_input.GetSingleImage(), _originalFlagging, flagfn2.str(), _bandInfo, false);
	}

	long double sensitivityStart = 4.0L;
	long double sensitivityStep = powl(sensitivityStart, 1.0L/iterationCount);
 
	long double sensitivity = sensitivityStart;
	if(_verbose)
		std::cout << "Performing initial threshold..." << std::endl;
	if(_input.PhaseRepresentation() == TimeFrequencyData::ComplexRepresentation) {
		_thresholdConfig->Execute(_input.GetRealPart(), _mask, false, sensitivity);
		_thresholdConfig->Execute(_input.GetImaginaryPart(), _mask, true, sensitivity);
	} else {
			Image2DCPtr image = _input.GetSingleImage();
			_thresholdConfig->Execute(image, _mask, false, sensitivity);
	}
	if(_writeStats) {
		//OutputStatistics(*images[imageIndex], *_mask);	
	}
	if(_savePNGs) {
		//char c = (char) ((int) 'A' + imageIndex);
		//std::stringstream flagfn2;
		//flagfn2 << name << c << "-" << images[0]->Width() << "-flag-00-threshold.png";
		//SaveFlaggingToPng(*images[imageIndex], *_mask, flagfn2.str(), _bandInfo);
	}

	for(unsigned i=0;i<iterationCount;++i)
	{
		if(_verbose)
			std::cout << "Performing iteration " << i << ": " << std::endl;

		if(_verbose)
			std::cout << "Fitting background.. " << std::flush;
		Stopwatch fitTimer(true);
		method.Initialize(_input);
		ExecuteBackgroundFit(method, threadCount);
		fitTimer.Pause();
		if(_writeStats) {
			_statWriter->SetValueInc(i+1);
			OutputMethodDetails(method, fitTimer, *_statWriter);
		}

		const TimeFrequencyData &background = method.Background();
		sensitivity /= sensitivityStep;

		if(_input.PhaseRepresentation() == TimeFrequencyData::ComplexRepresentation) {
			ExecuteThreshold(_input.GetRealPart(), background.GetRealPart(), 0, sensitivity);
			ExecuteThreshold(_input.GetImaginaryPart(), background.GetImaginaryPart(), 1, sensitivity);
		} else {
			Image2DCPtr image = _input.GetSingleImage();
			Image2DCPtr back = background.GetSingleImage();
			ExecuteThreshold(image, back, 0, sensitivity);
		}

		if(_savePNGs) {
			if(_verbose)
				std::cout << "Saving " << std::flush;

			if(_verbose)
				std::cout << "back" << std::flush;
			std::stringstream backfn;

			Image2DCPtr backVisualization = background.GetSingleImage();
			Image2DCPtr frontVisualization = _input.GetSingleImage();
			Image2DCPtr diff = Image2D::CreateFromDiff(frontVisualization, backVisualization);

			backfn << name << "-" << _input.ImageWidth() << "-background-" << ((i+1)/10) << ((i+1)%10) << ".png";
			SaveFlaggingToPng(backVisualization, _mask, backfn.str(), _bandInfo, false, true, false);

			if(_verbose)
				std::cout << ",diff" << std::flush;
			std::stringstream difffn;
			difffn << name << "-" << _input.ImageWidth()<< "-diff-" << ((i+1)/10) << ((i+1)%10) << ".png";
			SaveFlaggingToPng(diff, _mask, difffn.str(), _bandInfo, false);

			if(_verbose)
				std::cout << ",flag" << std::flush;
			std::stringstream flagfn;
			flagfn << name << "-" << _input.ImageWidth()<< "-flag-" << ((i+1)/10) << ((i+1)%10) << ".png";
			SaveFlaggingToPng(frontVisualization, _mask, flagfn.str(), _bandInfo);

			if(_verbose)
				std::cout << std::endl;
		}
	}

	if(_writeStats) delete _statWriter;
}

void MethodIterator::ExecuteThreshold(Image2DCPtr image, Image2DCPtr background, int imageIndex, long double sensitivity)
{
	if(image->ContainsOnlyZeros()) {
		if(_verbose)
			std::cout << "Skipping image because it contains only zeros." << std::endl;
	} else {
		char imageChar = 'A' + imageIndex;
		if(_verbose)
			std::cout << "Subtracting " << imageChar << ".. " << std::flush;
		Image2DPtr diff = Image2D::CreateFromDiff(image, background);

		if(_verbose)
			std::cout << "Thresholding " << imageChar << ".. " << std::flush;

		_thresholdConfig->Execute(diff, _mask, imageIndex!=0, sensitivity);
		if(_verbose)
			std::cout << "(" << 100.0L*_mask->GetCount<true>()/(_mask->Width()*_mask->Height()) << "%) " << std::flush;

		if(_writeStats)
			OutputStatistics(diff, _mask);
	}
}

void MethodIterator::OutputMethodDetails(const SurfaceFitMethod &, const Stopwatch &watch, StatWriter &)
{
	std::cout
		<< "Creating background in this iteration took: " << watch.ToString() 
		<< std::endl;
}

void MethodIterator::OutputStatistics(Image2DCPtr image, Mask2DCPtr mask)
{
	num_t mean, stdDev, wMean, wStdDev;
	ThresholdTools::MeanAndStdDev(image, mask, mean, stdDev);
	ThresholdTools::WinsorizedMeanAndStdDev(image, mask, wMean, wStdDev);
	unsigned long flagCount = mask->GetCount<true>();
	unsigned long orFlagCount = _originalFlagging->GetCount<true>();

	_statWriter->SetValueInc(mean);
	_statWriter->SetValueInc(stdDev);
	_statWriter->SetValueInc(wMean);
	_statWriter->SetValueInc(wStdDev);
	_statWriter->SetValueInc(ThresholdTools::MinValue(image, mask));
	_statWriter->SetValueInc(ThresholdTools::MaxValue(image, mask));
	_statWriter->SetValueInc((num_t) flagCount * 100.0 / (mask->Width()*mask->Height()), 2);
	_statWriter->SetValueInc((num_t) GetMaskOverlap(mask, _originalFlagging) * 100.0 / (mask->Width()*mask->Height()), 2);
	_statWriter->SetValueInc((num_t) orFlagCount * 100.0 / (mask->Width()*mask->Height()), 2);
}

void MethodIterator::SaveFlaggingToPng(Image2DCPtr image, Mask2DCPtr mask, const std::string &filename, const BandInfo *bandInfo, bool showFlagging)
{
	SaveFlaggingToPng(image, mask, filename, bandInfo, showFlagging, !showFlagging, !showFlagging);
}

void MethodIterator::SaveFlaggingToPng(Image2DCPtr image, Mask2DCPtr mask, const std::string &filename, const BandInfo *bandInfo, bool showFlagging, bool winsorizedStretch, bool useColor)
{
	PngFile file(filename, image->Width(), image->Height());
	file.BeginWrite();
	file.Clear(0, 0, 0, 255);

	num_t max, min;
	ColorMap *map1;

	if(winsorizedStretch) {
		num_t mean, stddev, genMax, genMin;
		if(showFlagging) {
			ThresholdTools::WinsorizedMeanAndStdDev(image, mask, mean, stddev);
			genMax = ThresholdTools::MaxValue(image, mask);
			genMin = ThresholdTools::MinValue(image, mask);
		} else {
			Mask2DPtr empty = Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
			ThresholdTools::WinsorizedMeanAndStdDev(image, empty, mean, stddev);
			genMax = image->GetMaximum();
			genMin = image->GetMinimum();
		}
		max = mean + stddev*3.0;
		min = mean - stddev*3.0;
		if(genMin > min) min = genMin;
		if(genMax < max) max = genMax;
	} else {
		max = ThresholdTools::MaxValue(image, mask);
		min = ThresholdTools::MinValue(image, mask);
	}
	if(useColor) {
		map1 = new ColdHotMap();
	} else {
		map1 = new MonochromeMap();
	}
	Plot plot(filename + ".pdf");
	plot.SetTitle("");
	plot.SetXRange(0, image->Width()-1);
	if(bandInfo != 0) {
		plot.SetYRange(bandInfo->channels[0].frequencyHz / 1000000.0L,
									bandInfo->channels[image->Height()-1].frequencyHz / 1000000.0L);
	}
	plot.SetXAxisText("Time (x10s)");
	plot.SetYAxisText("Frequency (MHz)");
	plot.SetZAxisText("Flux (Jy)");
	if(min != max) {
		plot.SetZRange(min, max);
		plot.SetCBRange(min, max);
	}
	plot.StartGrid("");
	for(unsigned y=0;y < image->Height(); ++y) {
		num_t freq;
		if(bandInfo != 0) {
			freq = bandInfo->channels[y].frequencyHz / 1000000.0L;
		} else {
			freq=y;
		}
		for(unsigned x=0;x < image->Width(); ++x) {
			if(mask->Value(x, y) == 0.0 || !showFlagging) {
				num_t val = image->Value(x, y);
				if(val > max) val = max;
				else if(val < min) val = min;
				plot.PushDataPoint(x, freq, val);

				val = (image->Value(x, y) - min) * 2.0 / (max - min) - 1.0;
				if(val < -1.0) val = -1.0;
				else if(val > 1.0) val = 1.0;
				file.PlotPixel(x, y, map1->ValueToColorR(val), map1->ValueToColorG(val), map1->ValueToColorB(val), map1->ValueToColorA(val));
			} else {
				file.PlotPixel(x, y, 255, 0, 255, 255);
				plot.PushUnknownDataPoint(x, freq);
			}
		}
		plot.PushDataBlockEnd();
	}

	if(showFlagging) {
		for(unsigned y=0;y < image->Height(); ++y) {
			num_t freq, freqdist;
			if(bandInfo != 0) {
				freq = bandInfo->channels[y].frequencyHz / 1000000.0L;
				freqdist = (bandInfo->channels[1].frequencyHz - bandInfo->channels[0].frequencyHz) / 1000000.0L;
			} else {
				freq = y;
				freqdist = 1;
			}
			unsigned x=0;
			unsigned lastX = 0;
			while(x < image->Width()) {
				if(mask->Value(x, y) == 0)
				{
					if(lastX < x)
						plot.AddRectangle(lastX, -freqdist/2.0L + freq, x, freq+freqdist/2.0L);
					lastX = x+1;
				}
				else if(lastX >= x) lastX = x;
				++x;
			}
			if(lastX < x)
				plot.AddRectangle(lastX, -freqdist/2.0L + freq, x, freq+freqdist/2.0L);
		}
	}

	file.Close();

	delete map1;
}

unsigned MethodIterator::GetMaskOverlap(Mask2DCPtr mask1, Mask2DCPtr mask2)
{
	unsigned count = 0;
	for(unsigned y=0;y<mask1->Height();++y) {
		for(unsigned x=0;x<mask1->Width();++x) {
			bool m1 = mask1->Value(x, y) != 0.0;
			bool m2 = mask2->Value(x, y) != 0.0;
			if(m1 && m2) {
				count++;
			}
		}
	}
	return count;
}
*/