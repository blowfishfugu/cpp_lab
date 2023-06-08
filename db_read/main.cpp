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
	db.TryConnect(pwszConnStr);
	auto info=db.GetInfo(SQL_DATABASE_NAME);
	std::cout << info << "\n";
	auto info2 = db.GetInfo(SQL_ACTIVE_ENVIRONMENTS);
	std::cout << info2 << "\n";
	
	return 0;

	return sample(argc, argv);
}
