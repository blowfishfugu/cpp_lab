#include "TypeConfigs.h"
#include "DataPath.h"
#include <iostream>
#include <fstream>
#include <string>
#include <array>


__int64 _loadByIfStream(data_type& data)
{
	auto dataPath = prepareDataPath();
	if (!dataPath)
	{
		return 0LL;
	}
	// data.reserve(500000); //kein voranlegen.. mal die "spec" gelesen ;)
	__int64 cnt = 0LL;
	std::ifstream input(dataPath->c_str());
	for (std::string str; std::getline(input, str);)
	{
		if (str.length() > 0)
		{
			//GeoLoc loc(str);
			//catch incomplete_line_error.. ungültige einfach weglassen?
			data.emplace_back(str);
			++cnt;
		}
	}
	return cnt;
}

__int64 _loadBuffered(data_type& data)
{
	auto dataPath = prepareDataPath();
	if (!dataPath) { return 0LL; }
	__int64 cnt = 0LL;
	//4K pagesize mal irgendwas
	constexpr size_t bufSize = (2ull << 11ull) * 16ull;
	std::vector<char> buf(bufSize);

	std::ifstream input(dataPath->c_str());
	input.rdbuf()->pubsetbuf(buf.data(), buf.size());
	for (std::string str; std::getline(input, str);)
	{
		if (str.length() > 0)
		{
			data.emplace_back(str);
			++cnt;
		}
	}
	return cnt;
}