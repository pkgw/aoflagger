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
#include <stdint.h> 

#include "../strategy/algorithms/thresholdtools.h"

#include "../msio/samplerow.h"

#include "compress.h"

void Compress::Initialize()
{
	if(!_isInitialized)
	{
		std::ofstream str("compress.bin");
		for(unsigned i=0;i<_data.ImageCount();++i)
		{
			Write(str, _data.GetImage(i), _data.GetSingleMask());
		}
		_isInitialized = true;
	}
}

void Compress::Deinitialize()
{
	if(_isInitialized)
	{
		//system("rm compress.bin");
	}
}

void Compress::Write(std::ofstream &stream, Image2DCPtr image, Mask2DCPtr mask)
{
	const num_t
		max = ThresholdTools::MaxValue(image, mask),
		min = ThresholdTools::MinValue(image, mask),
		mid = (min + max) / 2.0;
	const num_t normalizeFactor = (num_t) ((2<<22) + ((2<<22)-1)) / (max - min);
	const uint32_t
		width = image->Width(),
		height = image->Height();
	const char mode = 0;

	stream.write(reinterpret_cast<const char*>(&max), sizeof(max));
	stream.write(reinterpret_cast<const char*>(&min), sizeof(min));
	stream.write(reinterpret_cast<const char*>(&width), sizeof(width));
	stream.write(reinterpret_cast<const char*>(&height), sizeof(height));
	stream.write(&mode, sizeof(mode));

	for(unsigned y=0;y<height;++y)
	{
		for(unsigned x=0;x<width;++x)
		{
			if(!mask->Value(x, y))
			{
				int32_t value = (int32_t) round((image->Value(x, y) - mid) * normalizeFactor);
				stream.write(reinterpret_cast<char*>(&value)+1, 3);
			}
		}
	}
}

void Compress::WriteSubtractFrequencies(std::ofstream &stream, Image2DCPtr image, Mask2DCPtr mask)
{
	const num_t
		max = ThresholdTools::MaxValue(image, mask),
		min = ThresholdTools::MinValue(image, mask);
	const num_t normalizeFactor = (num_t) ((2<<22) + ((2<<22)-1)) / (max - min);
	//const num_t normalizeFactor = 256.0;
	const uint32_t
		width = image->Width(),
		height = image->Height();
	const char mode = 1;

	stream.write(reinterpret_cast<const char*>(&max), sizeof(max));
	stream.write(reinterpret_cast<const char*>(&min), sizeof(min));
	stream.write(reinterpret_cast<const char*>(&width), sizeof(width));
	stream.write(reinterpret_cast<const char*>(&height), sizeof(height));
	stream.write(&mode, sizeof(mode));

	std::vector<int32_t> basis(width);
	for(size_t x=0;x<width;++x)
	{
		SampleRowPtr row = SampleRow::CreateFromColumn(image, x);
		basis[x] = (int32_t) round(row->Median() * normalizeFactor);
	}
	stream.write(reinterpret_cast<char*>(&basis[0]), sizeof(basis));

	for(unsigned y=0;y<height;++y)
	{
		for(unsigned x=0;x<width;++x)
		{
			if(!mask->Value(x, y))
			{
				int32_t value = (int32_t) (round(image->Value(x, y) * normalizeFactor) - basis[x]);
				stream.write(reinterpret_cast<char*>(&value)+1, 3);
			}
		}
	}
}

Image2DPtr Compress::Read(std::ifstream &stream, Image2DPtr image, Mask2DCPtr mask)
{
	num_t max = 0.0, min = 0.0;
	size_t width = 0, height = 0;
	char mode = 0;
	stream.read(reinterpret_cast<char*>(&max), sizeof(max));
	stream.read(reinterpret_cast<char*>(&min), sizeof(min));
	stream.read(reinterpret_cast<char*>(&width), sizeof(width));
	stream.read(reinterpret_cast<char*>(&height), sizeof(height));
	stream.read(&mode, sizeof(mode));
	num_t normalizeFactor = (max - min) / (num_t) ((2<<22) + ((2<<22)-1));
	for(unsigned y=0;y<height;++y)
	{
		for(unsigned x=0;x<width;++x)
		{
			if(!mask->Value(x, y))
			{
				int32_t value;
				stream.read(reinterpret_cast<char*>(&value), 3);
				value >>= 8;
				image->SetValue(x, y, value / normalizeFactor + min);
			}
		}
	}
	return Image2DPtr();
}

unsigned long Compress::RawSize()
{
	Initialize();
	ExecuteCmd("cp compress.bin compress.raw");
	return Size("compress.raw");
}

unsigned long Compress::FlacSize()
{
	Initialize();
	ExecuteCmd("flac -f -8 --bps=24 --endian=little --channels=1 --sample-rate=128000 --sign=signed --lax -o compress.flac compress.bin");
	return Size("compress.flac");
}

unsigned long Compress::ZipSize()
{
	Initialize();
	ExecuteCmd("zip -9 compress.zip compress.bin");
	return Size("compress.zip");
}

unsigned long Compress::Size(const std::string &file)
{
	ExecuteCmd((std::string("ls -lh ") + file).c_str());
	ExecuteCmd((std::string("rm ") + file).c_str());
	return 0;
}

unsigned long Compress::GzSize()
{
	Initialize();
	ExecuteCmd("gzip -9 -c compress.bin > compress.gz");
	return Size("compress.gz");
}

unsigned long Compress::Bz2Size()
{
	Initialize();
	ExecuteCmd("bzip2 -9 -c compress.bin > compress.bz2");
	return Size("compress.bz2");
}

void Compress::ExecuteCmd(const std::string &str)
{
	if(system(str.c_str()) != 0)
		throw std::runtime_error("system() returned non-zero");
}

