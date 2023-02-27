#include "TypeConfigs.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <optional>

#include <Windows.h>

namespace fs = std::filesystem;

std::optional<fs::path> prepareDataPath()
{
	std::string strFile;
	strFile.resize(MAX_PATH, '\0');
	GetModuleFileName(NULL, strFile.data(), MAX_PATH);

	fs::path exePath(strFile);
	fs::path parentPath(exePath.parent_path());
	fs::path dataPath = parentPath.parent_path() / "Data" / "berlin_infos.dat";
	if( !fs::exists(dataPath))
	{
#if _MSC_VER>1900 //im vs2017 ist fs::path::value_type ein char .. irgendwo danach aber wchar!
			std::wcerr << "'" << dataPath.c_str() << "' not found\n";
#else
			std::cerr << "'" << dataPath.c_str() << "' not found\n";
#endif
		return {};
	}
	return dataPath;
}

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
