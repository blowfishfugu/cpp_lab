#include "TypeConfigs.h"
#include "ReadToToken.h"
#include "DataPath.h"
#include "LineIterator.h"
#include <iostream>
#include <fstream>
#include <string>

#include <array>
#include <sstream>
#include <functional>
#include <execution>

#include <future>

__int64 _loadByIfStream(data_type& data)
{
	const auto dataPath = prepareDataPath();
	if (!dataPath)
	{
		return 0LL;
	}
	// data.reserve(500000); //kein voranlegen.. mal die "spec" gelesen ;)
	__int64 cnt = 0LL;
	std::ifstream input(*dataPath);
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
	const auto dataPath = prepareDataPath();
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
	const auto dataPath = prepareDataPath();
	if (!dataPath) { return 0LL; }

	std::ifstream input( *dataPath, std::ifstream::binary);
	if (!input) { return 0LL; }

	input.seekg(0, std::ios::end);
	const auto size = input.tellg();
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


__int64 _loadStringBuffered2(data_type& data)
{
	const auto dataPath = prepareDataPath();
	if (!dataPath) { return 0LL; }

	std::ifstream input(*dataPath, std::ifstream::binary);
	if (!input) { return 0LL; }

	const auto size = fs::file_size(*dataPath);
	std::string strBuf(size, '\0');
	input.read(strBuf.data(), size);
	input.close();

	std::string_view strRef(strBuf.data(), strBuf.size());

	size_t lineCount = std::count(strRef.cbegin(), strRef.cend(), '\n');
	data.reserve(lineCount);
	my_lines bufferView(strRef);
	
	//achtung: std::copy + backinserter -> GeoLoc benötigt einen operator=(string_view)
	for( const auto& line: bufferView )
	{
		data.emplace_back(line);
	};

	
	return data.size();
}

/*
__int64 _loadStringFutured(data_type& data)
{
	const auto dataPath = prepareDataPath();
	if (!dataPath) { return 0LL; }

	std::ifstream input(*dataPath, std::ifstream::binary);
	if (!input) { return 0LL; }

	const auto size = fs::file_size(*dataPath);
	std::string strBuf(size, '\0');
	input.read(strBuf.data(), size);
	input.close();

	std::string_view strRef(strBuf.data(), strBuf.size());

	size_t lineCount = std::count(strRef.cbegin(), strRef.cend(), '\n');
	data.resize(lineCount);
	my_lines bufferView(strRef);
	//TODO: copy bufferView to lines ->back_inserter ans laufen bekommen.
	std::vector<my_line_count> lines;
	lines.reserve(lineCount);

	constexpr int iTasks = 16;
	std::vector<std::future<bool>> tasks(iTasks);

	auto r_func = [&lines, &data](auto start, auto end) {
		for (; start < end; ++start)
		{
			data.emplace_back(lines[start]);
		}
		return true;
	};

	for (auto[i, Cnt] = std::make_pair(0u, data.size()); i < iTasks; ++i)
	{
		tasks[i] = std::async(r_func, i*Cnt / iTasks, ((i+1)==iTasks)?Cnt:(i+1)*Cnt/iTasks );
	}

	tasks[0].get();

	return data.size();
}
*/