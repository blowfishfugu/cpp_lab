#include "DataPath.h"
#include <iostream> //std::cerr
#include <Windows.h> //GetModuleFileName

const std::optional<fs::path> prepareDataPath()
{
	std::string strFile;
	strFile.resize(MAX_PATH, '\0');
	GetModuleFileName(NULL, strFile.data(), MAX_PATH);

	fs::path exePath(strFile);
	fs::path parentPath(exePath.parent_path());
	fs::path dataPath = parentPath.parent_path() / "Data" / "berlin_infos.dat";
	if (!fs::exists(dataPath))
	{
#if _MSC_VER>=1916 //im vs2017 ist fs::path::value_type ein char .. irgendwo danach aber wchar!
		std::wcerr << "'" << dataPath.c_str() << "' not found\n";
#else
		std::cerr << "'" << dataPath.c_str() << "' not found\n";
#endif
		return {};
	}
	return dataPath;
}
