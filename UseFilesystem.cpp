#include "pch.h"

#include <filesystem>
#include <string>
#include <iostream>

#include <Windows.h>

namespace fs = std::filesystem;

TEST(UseFileSystem, check_relative)
{
	std::string strFile;
	strFile.resize(MAX_PATH, '\0');
	GetModuleFileName(NULL, strFile.data(), MAX_PATH);

	fs::path exePath(strFile);
	fs::path parentPath(exePath.parent_path());
	fs::path assetsPath = parentPath.parent_path() / "assets";

	std::cout << exePath.string() << "\n"
		<< parentPath.string() << "\n"
		<< assetsPath.string() << "\n";
	
	if (!fs::exists(assetsPath))
	{
		fs::create_directory(assetsPath);
	}
	ASSERT_TRUE(fs::exists(assetsPath));
}