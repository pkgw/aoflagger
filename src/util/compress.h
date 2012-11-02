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
#ifndef COMPRESS_H
#define COMPRESS_H

#include <fstream>

#include "../msio/timefrequencydata.h"

class Compress
{
	public:
		Compress(const TimeFrequencyData &data) : _data(data), _isInitialized(false)
		{
		}
		~Compress()
		{
			Deinitialize();
		}
		void Initialize();
		void Deinitialize();
		
		unsigned long RawSize();
		unsigned long FlacSize();
		unsigned long ZipSize();
		unsigned long GzSize();
		unsigned long Bz2Size();
		void AllToStdOut()
		{
			RawSize();
			FlacSize();
			ZipSize();
			GzSize();
			Bz2Size();
		}
	private:
		void ExecuteCmd(const std::string &str);

		const TimeFrequencyData _data;
		bool _isInitialized;

		unsigned long Size(const std::string &file);
		void Write(std::ofstream &stream, Image2DCPtr image, Mask2DCPtr mask);
		void WriteSubtractFrequencies(std::ofstream &stream, Image2DCPtr image, Mask2DCPtr mask);
		Image2DPtr Read(std::ifstream &stream, Image2DPtr image, Mask2DCPtr mask);
};

#endif // COMPRESS_H

