#include <Windows.h>
#include <sal.h>
#include <tchar.h>
#include "bff_odbc.h"
#include "sample.h"
int __cdecl _tmain(int argc, _In_reads_(argc) TCHAR** argv)
{
	HDbc db;
	return sample(argc, argv);
}
