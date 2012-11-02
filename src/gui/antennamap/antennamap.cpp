#include "antennamap.h"
#include "clock.h"
#include "spectrumanalyzer.h"

namespace antennaMap
{

AntennaMap::AntennaMap(MeasurementSet &set)
: _clock(0), _analyzer(0), _statFile(0), _lastFactor(0.0), _lastFactorChangeTime(0.0)
{
	const size_t antennaCount = set.AntennaCount();
	for(unsigned i=0;i<antennaCount;++i)
	{
		AntennaInfo info = set.GetAntennaInfo(i);
		_antennas.push_back(new Antenna(*this, info));
	}
	_analyzer = new SpectrumAnalyzer(32);
}

AntennaMap::~AntennaMap()
{
	for(std::vector<Antenna*>::iterator i=_antennas.begin(); i!=_antennas.end(); ++i)
		delete *i;
	if(_statFile != 0)
		delete _statFile;
	if(_clock != 0)
		delete _clock;
	if(_analyzer != 0)
		delete _analyzer;
}

void AntennaMap::Draw(Cairo::RefPtr<Cairo::Context> cairo, int width, int height, double timeInFrameSeconds)
{
	_widthInPixels = width;
	_heightInPixels = height;
	initMetrics();
	
	cairo->rectangle(0.0, 0.0, width, height);
	cairo->set_source_rgba(255, 255, 255, 1);
	cairo->fill();
	
	double analyzerHeight = 100.0;

	double
		offsetX = Antenna::GetMaxWidthInPixels()/2.0,
		offsetY = Antenna::GetMaxHeightInPixels()/2.0;
	bool moreLayers;
	int layer = 0;
	do {
		 moreLayers = false;
		for(std::vector<Antenna*>::iterator i=_antennas.begin();i!=_antennas.end();++i)
		{
			if((*i)->Draw(cairo, offsetX, offsetY, layer, timeInFrameSeconds))
				moreLayers = true;
		}
		++layer;
	} while(moreLayers);

	double clockWidth = 0.0;
	if(_clock != 0)
	{
		_clock->Draw(cairo, 0.0, 0.0);
		clockWidth = _clock->Width();
	}
	if(_analyzer != 0)
		_analyzer->Draw(cairo, clockWidth, 0.0, width - clockWidth - 2, analyzerHeight, timeInFrameSeconds);
}

void AntennaMap::initMetrics()
{
	const double analyzerHeight = 100.0;
	
	if(_antennas.size() == 0)
	{
		_xUCSToPixelsFactor = 0.0;
		_xUCSToPixelsOffset = 0.0;
		_yUCSToPixelsFactor = 0.0;
		_yUCSToPixelsOffset = 0.0;
	} else {
		std::vector<Antenna*>::const_iterator i = _antennas.begin();
		
		double
			minX = (*i)->GetXInUCS(),
			minY = (*i)->GetYInUCS(),
			maxX = (*i)->GetXInUCS(),
			maxY = (*i)->GetYInUCS();
			
		++i;
		
		while(i!=_antennas.end())
		{
			const double 
				x = (*i)->GetXInUCS(),
				y = (*i)->GetYInUCS();
			if(x < minX) minX = x;
			if(y < minY) minY = y;
			if(x > maxX) maxX = x;
			if(y > maxY) maxY = y;
			++i;
		}
		if(maxX == minX)
		{
			maxX += 1;
			minX -= 1;
		}
		if(maxY == minY)
		{
			maxY += 1;
			minY -= 1;
		}
		
		double
			availableWidth = _widthInPixels - Antenna::GetMaxWidthInPixels(),
			availableHeight = _heightInPixels - Antenna::GetMaxHeightInPixels() - analyzerHeight;
			
		_xUCSToPixelsFactor = availableWidth / (maxX - minX);
		_xUCSToPixelsOffset = -minX * _xUCSToPixelsFactor;
		_yUCSToPixelsFactor = availableHeight / (maxY - minY);
		_yUCSToPixelsOffset = -minY * _yUCSToPixelsFactor + analyzerHeight;
	}
}

void AntennaMap::OpenStatFile(const std::string &statFilename)
{
	if(_statFile != 0)
		delete _statFile;
	_statFile = new std::ifstream(statFilename.c_str());
	string headers;
	getline(*_statFile, headers);
	_clock = new Clock();
}

void AntennaMap::OpenAntennaFile(const std::string &antennaFile, double timeInFrameSeconds)
{
	if(_statFile != 0)
		delete _statFile;
	const unsigned size = _antennas.size();
	
	std::ifstream f(antennaFile.c_str());
	string headers;
	getline(f, headers);
	Image2DPtr
		weights = Image2D::CreateZeroImagePtr(size, size),
		values = Image2D::CreateZeroImagePtr(size, size);
	while(!f.eof())
	{
		int antenna1Index, antenna2Index;
		unsigned long totalCount, rfiCount;
		std::string tmpString;
		double tmp;
		f
		>> antenna1Index;
		if(f.eof()) break;
		f
		>> antenna2Index
		>> tmpString
		>> tmpString
		>> tmp
		>> tmp
		>> totalCount
		>> tmp
		>> rfiCount
		>> tmp
		>> tmp
		>> tmp
		>> tmp
		>> tmp;
		
		if(antenna2Index > antenna1Index)
		{
			weights->AddValue(antenna1Index, antenna2Index, totalCount);
			values->AddValue(antenna1Index, antenna2Index, rfiCount);
		} else {
			weights->AddValue(antenna2Index, antenna1Index, totalCount);
			values->AddValue(antenna2Index, antenna1Index, rfiCount);
		}
	}
	setValues(values, weights, timeInFrameSeconds);
}

void AntennaMap::ReadStatFile(int timeSteps, double timeInFrameSeconds)
{
	const unsigned count = _antennas.size();
	Image2DPtr
		weights = Image2D::CreateZeroImagePtr(count, count),
		values = Image2D::CreateZeroImagePtr(count, count);
	std::streampos matrixStart = _statFile->tellg();
	double currentTime = 0.0;
	while(!_statFile->eof() && timeSteps >= 0)
	{
		int antenna1Index, antenna2Index;
		double time, totalCount, rfiCount;
		std::streampos rowStart = _statFile->tellg();
		(*_statFile)
		>> antenna1Index;
		if(_statFile->eof()) break;
		(*_statFile)
		>> antenna2Index
		>> time
		>> totalCount
		>> rfiCount;
		if(time != currentTime)
		{
			--timeSteps;
			matrixStart = rowStart;
			currentTime = time;
		}
		if(timeSteps >= 0)
		{
			if(antenna2Index > antenna1Index)
			{
				weights->AddValue(antenna1Index, antenna2Index, totalCount);
				values->AddValue(antenna1Index, antenna2Index, rfiCount);
			} else {
				weights->AddValue(antenna2Index, antenna1Index, totalCount);
				values->AddValue(antenna2Index, antenna1Index, rfiCount);
			}
		}
	}
	_clock->SetTime(currentTime);
	_statFile->seekg(matrixStart);
	setValues(values, weights, timeInFrameSeconds);
}

void AntennaMap::OpenSpectrumFile(const std::string &spectrumFile, double timeInFrameSeconds)
{
	std::ifstream f(spectrumFile.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		unsigned index;
		double frequencyStart, frequencyEnd;
		long double totalCount, rfiCount;
		double tmp;
		f
		>> index;
		if(f.eof()) break;
		f
		>> frequencyStart
		>> frequencyEnd
		>> totalCount
		>> tmp
		>> rfiCount;
		for(unsigned i=0;i<11 + 14*3;++i)
		{
			f >> tmp;
			std::cout << tmp << ' ';
		}
		std::cout << index << '\n';
		_analyzer->SetValue(index/8, (long double) rfiCount / (long double) totalCount, timeInFrameSeconds);
	}
}

}
