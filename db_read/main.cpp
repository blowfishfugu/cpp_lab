#include <Windows.h>
#include <sal.h>
#include <tchar.h>
#include "bff_odbc.h"
#include "sample.h"
int __cdecl _tmain(int argc, _In_reads_(argc) TCHAR** argv)
{
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
	std::cerr << "try connect\n";
	db.TryConnect(pwszConnStr);
	std::cerr << (db ?(SQLHANDLE)db:"no db") << " connected=" << db.connected << "\n";
	std::cout << db.GetInfo(SQL_SPECIAL_CHARACTERS) << "\n";
	std::vector<InfoReturn> infos=db.GetRegisteredInfos();
	for (const auto& info : infos)
	{
		std::cout << info << "\n";
	}
	return 0;

	return sample(argc, argv);
}
