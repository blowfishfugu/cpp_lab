#include <Windows.h>
#include <sal.h>
#include <tchar.h>
#include "bff_odbc.h"
#include "sample.h"
int __cdecl _tmain(int argc, _In_reads_(argc) TCHAR** argv)
{
	const char* pwszConnStr =
		_T("DRIVER={ODBC Driver 18 for SQL Server}")
		_T(";SERVER=MENACE\\SQL2012")
		_T(";DATABASE=destatis")
		_T(";Trusted_Connection=YES")
		_T(";Encrypt=YES")
		_T(";TrustServerCertificate=YES");
	if (argc > 1)
	{
		pwszConnStr = *++argv;
	}

	HDbc db;
	db.TryConnect(pwszConnStr);
	return 0;

	return sample(argc, argv);
}
