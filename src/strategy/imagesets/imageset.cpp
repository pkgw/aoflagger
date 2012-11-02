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

#include "imageset.h"

#include "fitsimageset.h"
#include "harishreader.h"
#include "msimageset.h"
#include "noisestatimageset.h"
#include "parmimageset.h"
#include "rawimageset.h"
#include "rawdescimageset.h"
#include "rspimageset.h"
#include "timefrequencystatimageset.h"

namespace rfiStrategy {
	ImageSet *ImageSet::Create(const std::string &file, BaselineIOMode ioMode, bool readUVW)
	{
		if(IsFitsFile(file))
			return new FitsImageSet(file);
		else if(IsRCPRawFile(file))
			return new RSPImageSet(file);
		else if(IsTKPRawFile(file))
			return new RawImageSet(file);
		else if(IsRawDescFile(file))
			return new RawDescImageSet(file);
		else if(IsParmFile(file))
			return new ParmImageSet(file);
		else if(IsTimeFrequencyStatFile(file))
			return new TimeFrequencyStatImageSet(file);
		else if(IsNoiseStatFile(file))
			return new NoiseStatImageSet(file);
		else if(IsHarishFile(file))
			return new HarishReader(file);
		else {
			MSImageSet *set = new MSImageSet(file, ioMode);
			set->SetReadUVW(readUVW);
			return set;
		}
	}
	
	bool ImageSet::IsFitsFile(const std::string &file)
	{
		return
		(file.size() > 4 && file.substr(file.size()- 4) == ".UVF")
		||
		(file.size() > 5 && file.substr(file.size() - 5) == ".fits" )
		||
		(file.size() > 7 && file.substr(file.size() - 7) == ".sdfits" ); // Parkes raw files are named like this
	}
	
	bool ImageSet::IsRCPRawFile(const std::string &file)
	{
		return file.size() > 4 && file.substr(file.size()-4) == ".raw";
	}
	
	bool ImageSet::IsTKPRawFile(const std::string &file)
	{
		return file.size() > 4 && file.substr(file.size()-4) == ".1ch";
	}
	
	bool ImageSet::IsRawDescFile(const std::string &file)
	{
		return file.size() > 8 && file.substr(file.size()-8) == ".rawdesc";
	}
	
	bool ImageSet::IsParmFile(const std::string &file)
	{
		return file.size() >= 10 && file.substr(file.size()-10) == "instrument";
	}
	
	bool ImageSet::IsTimeFrequencyStatFile(const std::string &file)
	{
		return
		(file.size()>=24 && file.substr(file.size()-24) == "counts-timefreq-auto.txt")
		||
		(file.size()>=25 && file.substr(file.size()-25) == "counts-timefreq-cross.txt");
	}
	
	bool ImageSet::IsNoiseStatFile(const std::string &file)
	{
		return
		file.find("noise-statistics-tf") != std::string::npos &&
		file.find("txt") != std::string::npos;
	}
	
	bool ImageSet::IsHarishFile(const std::string &file)
	{
		return file.substr(file.size()-4) == ".har";
	}
	
	bool ImageSet::IsMSFile(const std::string &file)
	{
		return (!IsFitsFile(file)) && (!IsRCPRawFile(file)) && (!IsTKPRawFile(file)) && (!IsRawDescFile(file)) && (!IsParmFile(file)) && (!IsTimeFrequencyStatFile(file)) && (!IsNoiseStatFile(file));
	}
}
