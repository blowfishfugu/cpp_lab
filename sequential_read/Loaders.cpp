#include "TypeConfigs.h"
#include "DataPath.h"
#include "ReadToToken.h"
#include <iostream>
#include <fstream>
#include <string>

#include <array>
#include <sstream>
#include <functional>
#include <execution>

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

__int64 _loadStringBuffered(data_type& data)
{
	auto dataPath = prepareDataPath();
	if (!dataPath) { return 0LL; }

	std::ifstream input( *dataPath, std::ifstream::binary);
	if (!input) { return 0LL; }

	input.seekg(0, std::ios::end);
	auto size = input.tellg();
	input.seekg(0, std::ios::beg);

	std::string strBuf(size, '\0');
	input.read(strBuf.data(), size);
	input.close();
	
	size_t lineCount = std::count(std::execution::seq, strBuf.cbegin(),strBuf.cend(), '\n');
	//size_t lineCount = std::count(std::execution::par, strBuf.cbegin(),strBuf.cend(), '\n');
	data.reserve(lineCount);

	__int64 cnt = 0LL;
	readInfos in{ strBuf,0LL };
	__int64 lastpos = 0;
	range pos=readTo<'\n', true>(in);
	
	while(lastpos!=in.pos)
	{
		if (pos.len>0)
		{
			data.emplace_back(in.all.substr(pos.begin,pos.len));
			++cnt;
		}
		lastpos = in.pos;
		pos = readTo<'\n', true>(in);
	}
	return cnt;
}