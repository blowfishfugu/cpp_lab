//parsing https://raw.githubusercontent.com/MicrosoftDocs/sql-docs/live/docs/odbc/reference/syntax/sqlgetinfo-function.md

#include <optional>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <ranges>
#include <string_view>
#include <tuple>
#include <format>
#include <Windows.h> //GetModuleFileName

namespace fs = std::filesystem;

const std::optional<fs::path> prepareDataPath()
{
	std::string strFile;
	strFile.resize(MAX_PATH, '\0');
	GetModuleFileName(NULL, strFile.data(), MAX_PATH);

	fs::path exePath(strFile);
	fs::path parentPath(exePath.parent_path());
	fs::path dataPath = parentPath.parent_path() / "Data" / "GetInfos.txt";
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

constexpr const std::string_view start = "|Information Type|ODBC Version|Description|";
using row=std::tuple<std::string, std::string, std::string>;

std::vector<row> read(fs::path fileName)
{
	std::vector<row> data;
	__int64 cnt = 0LL;
	std::ifstream input(fileName);
	int state = 0;
	for (std::string str; std::getline(input, str);)
	{
		//search
		if( state==0)
		{ 
			if (str.find(start, 0) != std::string::npos)
			{
				state = 1;
			}
		}
		//capture
		else if( state==1 )
		{ 
			if (str.length() == 0)
			{
				break;
			}
			row line{};
			int index = 0;
			for ( const auto item: std::views::split(str, '|') )
			{
				std::string_view val{ item.begin(),item.end() };
				if (index == 1) { std::get<0>(line) = val; }
				if (index == 2) { std::get<1>(line) = val; }
				if (index == 3) { 
					std::get<2>(line) = val;
					data.emplace_back(line);
					break;
				}
				index++;
			}
		}

	}
	return data;
}

void outputRow(std::ostream& os, const row& item)
{
	const auto& [name, odbcver, info] = item;
	std::string getFunc = "_get***()";
	if (info.find("SQLUSMALLINT") != std::string::npos)
	{
		getFunc = "_getNumeric<SQLUSMALLINT>";
	}
	//SQLUINTEGER bitmask, bekam eigenen struct für speziellen cout
	else if (info.find("SQLUINTEGER bitmask") != std::string::npos)
	{
		getFunc = "_getNumeric<BitMask>";
	}
	else if (info.find("SQLUINTEGER") != std::string::npos)
	{
		getFunc = "_getNumeric<SQLUINTEGER>";
	}
	else if (info.find("SQLINTEGER") != std::string::npos) //gibts nur einen (SQL_POS_OPERATIONS)
	{
		getFunc = "_getNumeric<SQLUINTEGER>";
	}
	else if (info.find("character string") != std::string::npos)
	{
		getFunc = "_getInfoString";
	}
	else if (info.find("SQLULEN") != std::string::npos) //returns HANDLE
	{
		getFunc = "_getNumeric<SQLULEN>";
	}
	os << std::format("{{ {}, {{ \"{}\", {} }} }},\n", name, name, getFunc);
}

// { SQL_DATABASE_NAME, { "SQL_DATABASE_NAME", _getInfoString } }
std::ostream& operator<<(std::ostream& os, const row& item)
{
	const auto& [name, odbcver, info] = item;
	if (name.compare("-") == 0)
	{
		return os;
	}
	if (name.find("<br/>") != std::string::npos) //Sql_Convert_* kommen in einer Zelle
	{
		std::string_view splitter{ "<br/>" };
		for (const auto names : std::views::split(name, splitter))
		{
			std::string_view itemName{ names.begin(),names.end() };
			row tmp{ itemName,odbcver,info };
			outputRow(os, tmp);
		}
	}
	else
	{
		outputRow(os, item);
	}
	return os;
}

int main(int argc, char** argv)
{
	auto dataPath = prepareDataPath();
	if (!dataPath) { return 0; }
	fs::path dataFile = dataPath.value();
	std::vector<row> data = read(dataFile);

	auto outPath = dataFile.parent_path() / "OutMap.cpp";
	std::ofstream ofs(outPath, std::ofstream::trunc);
	ofs << "static std::map < SQLUSMALLINT, std::tuple<std::string, GetterFunc>> infoGetters =\n{\n";
	for (const row item : data)
	{
		ofs << item;
	}
	ofs << "};\n";
	return 0;
}