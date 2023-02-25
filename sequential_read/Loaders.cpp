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
		std::cerr << "'" << dataPath.c_str() << "' not found\n";
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
	return 0LL;
}
