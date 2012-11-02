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

#include "aologger.h"

enum AOLogger::AOLoggerLevel AOLogger::_coutLevel = AOLogger::InfoLevel;

AOLogger::LogWriter<AOLogger::DebugLevel> AOLogger::Debug;

AOLogger::LogWriter<AOLogger::InfoLevel> AOLogger::Info;

AOLogger::LogWriter<AOLogger::WarningLevel> AOLogger::Warn;

AOLogger::LogWriter<AOLogger::ErrorLevel> AOLogger::Error;

AOLogger::LogWriter<AOLogger::FatalLevel> AOLogger::Fatal;

AOLogger::LogWriter<AOLogger::NoLevel, true> AOLogger::Progress;

void AOLogger::Init(const std::string &name, bool useLogger, bool verbose)
{
	Debug.SetUseLogger(useLogger && verbose);
	Info.SetUseLogger(useLogger);
	Warn.SetUseLogger(useLogger);
	Error.SetUseLogger(useLogger);
	Fatal.SetUseLogger(useLogger);
	Debug.SetUseLogger(useLogger && verbose);

	if(useLogger) {
		_coutLevel = ErrorLevel;
		//INIT_LOGGER(name);
	}
	else {
		if(verbose)
			_coutLevel = DebugLevel;
		else
			_coutLevel = InfoLevel;
	}
}
