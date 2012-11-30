#include "vdsfile.h"

#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <sstream>

VdsFile::VdsFile(const std::string& path)
{
	std::ifstream stream(path.c_str());
	while(stream.good())
	{
		std::string line;
		std::getline(stream, line);
		if(stream.good() && !line.empty())
		{
			parseItem(line);
		}
	}
}

void VdsFile::parseItem(const std::string& line)
{
	size_t eqPos = line.find('=');
	if(eqPos == std::string::npos)
		throw std::runtime_error("Error in line (no equals sign): " + line);
	std::string key = trim(line.substr(0, eqPos), true);
	std::string value = trim(line.substr(eqPos+1), false);
	_items.insert(std::pair<std::string,std::string>(key, value));
}

size_t VdsFile::NParts() const
{
	return atoi(value("nparts").c_str());
}

std::string VdsFile::trim(const std::string &str, bool lower)
{
	std::ostringstream s;
	std::string::const_iterator i=str.begin();
	size_t index = 0, lastNonWhite = 0;
	while(*i == ' ' && i!=str.end()) ++i;
	while(i!=str.end())
	{
		if(*i >= 'A' && *i <= 'Z' && lower)
			s << (char) (*i + ('a' - 'A'));
		else
			s << *i;
		if(*i != ' ') lastNonWhite = index;
		++i; ++index;
	}
	std::string o = s.str();
	if(lastNonWhite < o.size())
		return o.substr(0, lastNonWhite+1);
	else
		return std::string();
}

std::string VdsFile::Host(size_t partIndex) const
{
	std::ostringstream key;
	key << "part" << partIndex << ".filesys";
	std::string filesystem = value(key.str());
	size_t separatorPos = filesystem.find(':');
	if(separatorPos == std::string::npos || separatorPos == 0)
		throw std::runtime_error("One of the file system descriptors in the VDS file has an unexpected format");
	return filesystem.substr(0, separatorPos);
}

const std::string &VdsFile::Filename(size_t partIndex) const
{
	std::ostringstream key;
	key << "part" << partIndex << ".filename";
	return value(key.str());
}
