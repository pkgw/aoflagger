#ifndef VDS_FILE_H
#define VDS_FILE_H

#include <string>
#include <map>

class VdsFile
{
	public:
		VdsFile(const std::string &path);
		
		size_t NParts() const;
		std::string Host(size_t partIndex) const;
		const std::string &Filename(size_t partIndex) const;
	private:
		void parseItem(const std::string &line);
		static std::string trim(const std::string &str, bool lower);
		const std::string &value(const std::string &name) const
		{
			return _items.find(name)->second;
		}
		
		std::map<std::string, std::string> _items;
};

#endif
