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
#ifndef AOFLAGGER
#define AOFLAGGER

#include <sstream>
#include <iostream>

#include <boost/thread/mutex.hpp>

class AOLogger
{
	public:
		enum AOLoggerLevel { NoLevel=5, FatalLevel=4, ErrorLevel=3, WarningLevel=2, InfoLevel=1, DebugLevel=0 };

		template<enum AOLoggerLevel Level, bool ToStdErr=false>
		class LogWriter
		{
			public:
				LogWriter() : _useLogger(false) { }
				~LogWriter()
				{
					if(_useLogger && _buffer.str().size() != 0)
						Log(_buffer.str());
				}
				LogWriter &operator<<(const std::string &str)
				{
					boost::mutex::scoped_lock lock(_mutex);
					if(_useLogger)
					{
						size_t start = 0, end;
						while(std::string::npos != (end = str.find('\n', start)))
						{
							_buffer << str.substr(start, end - start);
							start = end+1;
							Log(_buffer.str());
							_buffer.str(std::string());
						}
						_buffer << str.substr(start, str.size() - start);
					}
					ToStdOut(str);
					return *this;
				}
				LogWriter &operator<<(const char *str)
				{
					(*this) << std::string(str);
					return *this;
				}
				LogWriter &operator<<(const char c)
				{
					boost::mutex::scoped_lock lock(_mutex);
					if(_useLogger)
					{
						if(c == '\n')
						{
							Log(_buffer.str());
							_buffer.str(std::string());
						} else {
							_buffer << c;
						}
					}
					ToStdOut(c);
					return *this;
				}
				template<typename S>
				LogWriter &operator<<(const S &str)
				{
					boost::mutex::scoped_lock lock(_mutex);
					if(_useLogger)
					{
						_buffer << str;
					}
					ToStdOut(str);
					return *this;
				}
				void Flush()
				{
					boost::mutex::scoped_lock lock(_mutex);
					std::cout.flush();
				}
				void SetUseLogger(bool useLogger)
				{
					boost::mutex::scoped_lock lock(_mutex);
					_useLogger = useLogger;
				}
			private:
				bool _useLogger;
				std::stringstream _buffer;
				boost::mutex _mutex;

				void Log(const std::string &str)
				{
					/*switch(Level)
					{
						case NoLevel: break;
						case DebugLevel:   LOG_DEBUG(str); break;
						case InfoLevel:    LOG_INFO(str); break;
						case WarningLevel: LOG_WARN(str); break;
						case ErrorLevel:   LOG_ERROR(str); break;
						case FatalLevel:   LOG_FATAL(str); break;
					}*/
				}
				template<typename S>
				void ToStdOut(const S &str)
				{
					if((int) _coutLevel <= (int) Level)
					{
						if(ToStdErr)
							std::cerr << str;
						else
							std::cout << str;
					}
				}
		};

		static void Init(const std::string &name, bool useLogger=false, bool verbose=false);

		static class LogWriter<DebugLevel> Debug;
		static class LogWriter<InfoLevel> Info;
		static class LogWriter<WarningLevel> Warn;
		static class LogWriter<ErrorLevel> Error;
		static class LogWriter<FatalLevel> Fatal;
		static class LogWriter<NoLevel, true> Progress;
	private:
		AOLogger()
		{
		}

		static enum AOLoggerLevel _coutLevel;
};

#endif
