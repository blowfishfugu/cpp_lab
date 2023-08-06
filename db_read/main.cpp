#include <Windows.h>
#include <sal.h>
#include <tchar.h>
#include "bff_odbc.h"
#include "sample.h"
#include <iomanip> //quoted

//microsoft-signatur, mit sal.h.. braucht man das?
//int __cdecl _tmain(int argc, _In_reads_(argc) TCHAR** argv)
int main(int argc, char** argv)
{
	//return 0;
	//extern int runHellos();
	//return runHellos();

	const char* pwszConnStr =
		"DRIVER={ODBC Driver 18 for SQL Server}"
		";SERVER=MENACE\\SQL2012"
		";DATABASE=destatis"
		";Trusted_Connection=YES"
		";Encrypt=YES"
		";TrustServerCertificate=YES";
	if (argc > 1)
	{
		pwszConnStr = *++argv;
	}

	HDbc db;
	std::vector<HEnv::DriverInfo> drivers=db._env.GetDrivers();
	for (const auto& driver : drivers)
	{
		const auto& [desc, descLen, attr, attrLen] = driver;
		std::cout << desc << "\n";
		std::cout << attr << "\n";
	}

	std::cerr << "try connect\n";
	db.TryConnect(pwszConnStr);
	std::cerr << (db ?(SQLHANDLE)db:"no db") << " connected=" << db.connected << "\n";
	std::cout << db.GetInfo(SQL_SPECIAL_CHARACTERS) << "\n";
	std::vector<InfoReturn> infos=db.GetRegisteredInfos();
	for (const auto& info : infos)
	{
		std::cout << info << "\n";
	}

	Query q = db.CreateQuery("SELECT * FROM de");
	const auto [colCount, rowCount] = q.Execute();
	if (rowCount >= 0)
	{
		std::cout << rowCount << (rowCount == 1 ? _T(" row") : _T(" rows")) << " affected\n";
	}
	if (colCount > 0)
	{
		__int64 fetchCount = 0;
		while (q.Fetch())
		{
			++fetchCount;
			std::vector<std::string> trimmedItems;
			size_t index = 0;
			for (auto& bufItem : q.buffer)
			{
				trimmedItems.emplace_back(conv(bufItem, index++));
			}

			if ( (fetchCount % 1000) == 0)
			{
				for (const auto& trimmed : trimmedItems)
				{
					std::cout << std::quoted(trimmed, '\'') << " ";
				}
				std::cout << "\n";
			}
		}
		std::cout << fetchCount << "\n";
	}
	return 0;

	return sample(argc, argv);
}
