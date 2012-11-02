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
#include "plot.h"

#include <stdlib.h>
#include <sstream>
#include <iostream>

Plot::Plot(const std::string &pdfFile)
	: _pdfFile(pdfFile),
		_xAxisText("x"),
		_yAxisText("y"),
		_zAxisText("z"),
		_xRangeHasMin(false), _xRangeHasMax(false),
		_yRangeHasMin(false), _yRangeHasMax(false),
		_zRangeHasMin(false), _zRangeHasMax(false),
		_cbRangeHasMin(false), _cbRangeHasMax(false),
		_clipZ(false),
		_logX(false), _logY(false), _logZ(false),
		_hasBoxes(false),
		_fontSize(24)
{
	_open = true;
	_curLineFd = -1;
}


Plot::~Plot()
{
	Close();
}

void Plot::Close()
{
	if(_open)
	{
		CloseCurFd();
		char tmpPlotFile[] = "/tmp/plot.plt-XXXXXX";
		int fd = mkstemp(tmpPlotFile);
		if(fd == -1)
		{
			std::cerr << "mkstemp returned -1" << std::endl;
			throw;
		}
		std::stringstream header;
		header
			<< "set term postscript enhanced color"; // font \"Helvetica"; <-- that did not work with some gnuplots.
		//if(_fontSize>0)
		//	header << "," << _fontSize;
		// header << "\"";
		header
			<< "\nset title \"" << _title << '\"'
		  << "\nset pm3d map" // at s
			<< "\nset palette rgbformulae 33,13,10"
			<< "\nset xlabel \"" << _xAxisText << '\"'
			<< "\nset ylabel \"" << _yAxisText << '\"'
			<< "\nset cblabel \"" << _zAxisText << '\"'
			<< "\nset datafile missing \"?\""
			<< "\nset origin -0.05,-0.1"
			<< "\nset size 1.0,1.2"
			<< "\nset output \"" << _pdfFile << "\"";
		for(std::vector<std::string>::const_iterator i=_extraHeaders.begin();i!=_extraHeaders.end();++i)
			header << *i;
		if(_logX)
			header << "\nset log x";
		if(_logY)
			header << "\nset log y";
		if(_logZ)
			header << "\nset log z";
		if(_xRangeHasMin || _xRangeHasMax)
		{
			header << "\nset xrange [";
			if(_xRangeHasMin) header << _xRangeMin;
			header << ":";
			if(_xRangeHasMax) header << _xRangeMax;
			header << "]";
		}
		if(_yRangeHasMin || _yRangeHasMax)
		{
			header << "\nset yrange [";
			if(_yRangeHasMin) header << _yRangeMin;
			header << ":";
			if(_yRangeHasMax) header << _yRangeMax;
			header << "]";
		}
		if(_zRangeHasMin || _zRangeHasMax)
		{
			header << "\nset zrange [";
			if(_zRangeHasMin) header << _zRangeMin;
			header << ":";
			if(_zRangeHasMax) header << _zRangeMax;
			header << "]";
		}
		if(_cbRangeHasMin || _cbRangeHasMax)
		{
			header << "\nset cbrange [";
			if(_cbRangeHasMin) header << _cbRangeMin;
			header << ":";
			if(_cbRangeHasMax) header << _cbRangeMax;
			header << "]";
		}
		if(_hasBoxes)
		{
			header << "\nset boxwidth 0.3";
		}
		if(_lineTypes.size() > 0)
		{
			switch(_lineTypes[0]) {
				case Line:
				case Scatter:
				case Boxes:
					header << "\nplot \\";
					break;
				case Grid:
					header << "\nsplot \\";
					break;
			}
			for(unsigned i=0;i<_lineFiles.size();++i)
			{
				if(i!=0) header << ",\\";
				switch(_lineTypes[i]) {
					case Line:
						header << "\n\"" << _lineFiles[i] << "\" using 1:2 title \"" << _lineTitles[i] << "\" with lines lw 3";
						break;
					case Scatter:
						header << "\n\"" << _lineFiles[i] << "\" using 1:2 title \"" << _lineTitles[i] << "\" with points";
						break;
					case Boxes:
						header << "\n\"" << _lineFiles[i] << "\" using 1:2 title \"" << _lineTitles[i] << "\" with boxes fs solid 1.0";
						break;
					case Grid:
						header << "\n\"" << _lineFiles[i] << "\" using ($1):($2):($3) title \"" << _lineTitles[i] << "\" with pm3d lw 3";
						break;
				}
			}
			header << "\n";
			Write(fd, header.str());
			std::cout << "gnuplot" << std::endl;
			ExecuteCmd((std::string("gnuplot ") + tmpPlotFile).c_str());
			std::cout << "mv" << std::endl;
			ExecuteCmd((std::string("mv ") + _pdfFile + " " + _pdfFile + ".ps").c_str());
			std::cout << "ps2pdf" << std::endl;
			ExecuteCmd((std::string("ps2pdf ") + _pdfFile + ".ps " + _pdfFile).c_str());
			std::cout << "rm" << std::endl;
			ExecuteCmd((std::string("rm ") + _pdfFile + ".ps").c_str());
		}
		_open = false;
	}
}

void Plot::Show()
{
	ExecuteCmd((std::string("./kpdf ") + _pdfFile).c_str());
}

void Plot::CloseCurFd()
{
	if(_curLineFd != -1)
	{
		close(_curLineFd);
	}
}

void Plot::StartLine(const std::string &lineTitle)
{
	CloseCurFd();
	char tmpLineFile[] = "/tmp/line.txt-XXXXXX";
	_curLineFd = mkstemp(tmpLineFile);
	if(_curLineFd == -1)
	{
		std::cerr << "mkstemp returned -1" << std::endl;
		throw;
	}
	_lineFiles.push_back(tmpLineFile);
	_lineTitles.push_back(lineTitle);
	_lineTypes.push_back(Line);
	_curType = Line;
}

void Plot::StartScatter(const std::string &lineTitle)
{
	CloseCurFd();
	char tmpLineFile[] = "/tmp/line.txt-XXXXXX";
	_curLineFd = mkstemp(tmpLineFile);
	if(_curLineFd == -1)
	{
		std::cerr << "mkstemp returned -1" << std::endl;
		throw;
	}
	_lineFiles.push_back(tmpLineFile);
	_lineTitles.push_back(lineTitle);
	_lineTypes.push_back(Scatter);
	_curType = Scatter;
}

void Plot::StartBoxes(const std::string &lineTitle)
{
	_hasBoxes = true;
	CloseCurFd();
	char tmpLineFile[] = "/tmp/line.txt-XXXXXX";
	_curLineFd = mkstemp(tmpLineFile);
	if(_curLineFd == -1)
	{
		std::cerr << "mkstemp returned -1" << std::endl;
		throw;
	}
	_lineFiles.push_back(tmpLineFile);
	_lineTitles.push_back(lineTitle);
	_lineTypes.push_back(Boxes);
	_curType = Boxes;
}

void Plot::StartGrid(const std::string &lineTitle)
{
	CloseCurFd();
	char tmpLineFile[] = "/tmp/line.txt-XXXXXX";
	_curLineFd = mkstemp(tmpLineFile);
	if(_curLineFd == -1)
	{
		std::cerr << "mkstemp returned -1" << std::endl;
		throw;
	}
	_lineFiles.push_back(tmpLineFile);
	_lineTitles.push_back(lineTitle);
	_lineTypes.push_back(Grid);
	_curType = Grid;
}

void Plot::PushDataPoint(long double x, long double y)
{
	if(_curLineFd == -1) throw;
	std::stringstream s;
	s << x << "\t" << y << std::endl;
	Write(_curLineFd, s.str());
}

void Plot::PushDataPoint(long double x, long double y, long double z)
{
	if(_curLineFd == -1) throw;
	if(_clipZ)
	{
		if(_zRangeHasMin && z < _zRangeMin)
			z = _zRangeMin;
		if(_zRangeHasMax && z > _zRangeMax)
			z = _zRangeMax;
	}
	std::stringstream s;
	s << x << "\t" << y << "\t" << z << std::endl;
	Write(_curLineFd, s.str());
}

void Plot::PushUnknownDataPoint(long double x, long double y)
{
	if(_curLineFd == -1) throw;
	std::stringstream s;
	s << x << "\t" << y << "\t?" << std::endl;
	Write(_curLineFd, s.str());
}

void Plot::PushDataBlockEnd()
{
	Write(_curLineFd, "\n");
}

void Plot::AddRectangle(long double x1, double y1, double x2, double y2)
{
	std::stringstream s;
	s << "\nset object " << (_extraHeaders.size()+1) << " "
		"rectangle from " << x1 << "," << y1 << " to " << x2 << "," << y2 << " front lw 0 fc rgb \"#FF00FF\" fillstyle solid 1.0 noborder";
	_extraHeaders.push_back(s.str());
}

void Plot::ExecuteCmd(const std::string &cmd) const
{
	if(system(cmd.c_str()) != 0)
		throw std::runtime_error("system() returned non-zero");
}

