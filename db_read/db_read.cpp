//Sample: Entnahme von https://learn.microsoft.com/en-us/sql/connect/odbc/cpp-code-example-app-connect-access-sql-db?view=sql-server-ver16#b-odbcsqlcpp-code
//ODBCSQL: a sample program that implements an ODBC command line interpreter.
//USAGE: ODBCSQL DSN=<dsn name> or
//ODBCSQL FILEDSN=<file dsn> or
//ODBCSQL DRIVER={driver name}
//
//Copyright(c) Microsoft Corporation. This is a WDAC sample program and
//is not suitable for use in production environments.
//
//Modules:
//Main Main driver loop, executes queries.
//DisplayResults Display the results of the query if any
//AllocateBindings Bind column data
//DisplayTitles Print column titles
//SetConsole Set console display mode
//HandleError Show ODBC error messages
#include "TMyCredentials.h"

#define NOMINMAX
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <stdlib.h>
#include <sal.h>

#include <numeric>
#include <charconv>
#include <string>
#include <string_view>

#ifdef UNICODE
using String = std::wstring;
using StringView = std::wstring_view;
#else
using String = std::string;
using StringView = std::string_view;
#endif

using LoginData = TMyCredentials;

struct odbc_exception : public std::exception
{
	std::string _msg;
	odbc_exception(const char* const msg) : _msg(msg), std::exception(msg)
	{}
	odbc_exception( const std::string& msg) : _msg(msg), std::exception(_msg.c_str())
	{}
};

// Structure to store information about a column.
struct BINDING {
	SQLSMALLINT cDisplaySize{}; // size to display
	TCHAR* wszBuffer=nullptr; //display buffer
	SQLLEN indPtr{}; //size or null
	BOOL fChar{}; //character col?
	BINDING* sNext=nullptr; //linked list
};

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
void DisplayResults(HSTMT hStmt, SQLSMALLINT cCols);
void AllocateBindings(HSTMT hStmt, SQLSMALLINT cCols, BINDING** ppBinding, SQLSMALLINT* pDisplay);
void DisplayTitles(HSTMT hStmt, DWORD cDisplaySize, BINDING* pBinding);
void SetConsole(DWORD cDisplaySize, BOOL fInvert);

// Takes handle, handle type, and stmt
auto TRYODBC = [](SQLHANDLE context, SQLSMALLINT handletype, SQLRETURN rc) {
	if (rc != SQL_SUCCESS)
	{
		HandleDiagnosticRecord(context, handletype, rc);
	}
	if (rc == SQL_ERROR)
	{
		throw odbc_exception(std::format("Error in"));
	}
};



constexpr SQLLEN DISPLAY_MAX = 50; // Arbitrary limit on column width to display;
constexpr SQLSMALLINT DISPLAY_FORMAT_EXTRA = 3; // Per column extra display bytes (| <data> );
constexpr const TCHAR* DISPLAY_FORMAT = _T("%c %*.*s ");
constexpr const TCHAR* DISPLAY_FORMAT_C = _T("%c %-*.*s ");
constexpr const size_t NULL_SIZE = 6; //<NULL>
constexpr const size_t SQL_QUERY_SIZE = 1000; // Max. Num characters for SQL Query passed in.
constexpr const TCHAR PIPE = _T('|');

SHORT gHeight = 80; // Users screen height (adapts)

template<SQLSMALLINT handleType=SQL_HANDLE_ENV>
struct OdbcHandle
{
	SQLHANDLE handle = SQL_NULL_HANDLE;
	OdbcHandle(SQLHANDLE parentHandle=SQL_NULL_HANDLE)
	{
		if constexpr (handleType != SQL_HANDLE_ENV)
		{
			if (parentHandle == SQL_NULL_HANDLE)
			{
				throw odbc_exception("Handletype needs a parentHandle to allocate");
			}
		}
		
		SQLRETURN rc = SQLAllocHandle(handleType, parentHandle, &handle);
		TRYODBC(parentHandle, handleType, rc);
		
		if constexpr ( handleType==SQL_HANDLE_ENV)
		{ 
			// Register this as an application that expects 3.x behavior,
			rc = SQLSetEnvAttr(handle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			TRYODBC(handle, handleType, rc);
		}
	}
	
	~OdbcHandle()
	{
		if (handle)
		{
			if constexpr (handleType == SQL_HANDLE_DBC)
			{
				SQLDisconnect(handle);
			}
			SQLFreeHandle(handleType, handle);
		}
	}

	operator SQLHANDLE() { return handle; }

	void TryConnect(const TCHAR* pwszConnStr)
	{
		static_assert(handleType == SQL_HANDLE_DBC, "TryConnect only callable on hdbc-Handles");
		SQLRETURN rc = SQLDriverConnect(
			handle, GetDesktopWindow(),
			const_cast<TCHAR*>(pwszConnStr), //const_cast, weil sqldriverconnect es so m�chte
			SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

		TRYODBC(handle, handleType, rc);
	}
};

int __cdecl _tmain(int argc, _In_reads_(argc) TCHAR** argv)
{
	const TCHAR* pwszConnStr = _T("Driver=SQL Server;Server=menace\\SQL2012;Database=destatis;Integrated Security=true");
	if (argc > 1)
	{
		pwszConnStr = *++argv;
	}

	OdbcHandle<SQL_HANDLE_ENV> env;
	OdbcHandle<SQL_HANDLE_DBC> hDbc(env);
	
	// Connect to the driver. Use the connection string if supplied
	// on the input, otherwise let the driver manager prompt for input.
	hDbc.TryConnect(pwszConnStr);
	fwprintf(stderr, L"Connected!\n");

	OdbcHandle<SQL_HANDLE_STMT> hStmt(hDbc);
	wprintf(L"Enter SQL commands, type (control)Z to exit\nSQL COMMAND>");

	TCHAR wszInput[SQL_QUERY_SIZE]{};
	// Loop to get input and execute queries
	while (_fgetts(wszInput, SQL_QUERY_SIZE - 1, stdin))
	{
		// Execute the query
		if (!(*wszInput))
		{
			wprintf(L"SQL COMMAND>");
			continue;
		}
		SQLRETURN RetCode = SQLExecDirect(hStmt, wszInput, SQL_NTS);

		switch (RetCode)
		{
		case SQL_SUCCESS_WITH_INFO:
		{
			HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
			[[fallthrough]];
		}
		case SQL_SUCCESS:
		{
			// If this is a row-returning query, display
			// results
			SQLSMALLINT sNumResults;
			TRYODBC(hStmt, SQL_HANDLE_STMT, SQLNumResultCols(hStmt, &sNumResults));

			if (sNumResults > 0)
			{
				DisplayResults(hStmt, sNumResults);
			}
			else
			{
				SQLLEN cRowCount;
				TRYODBC(hStmt, SQL_HANDLE_STMT, SQLRowCount(hStmt, &cRowCount));
				if (cRowCount >= 0)
				{
					wprintf(L"%Id %s affected\n", cRowCount, cRowCount == 1 ? L"row" : L"rows");
				}
			}
			break;
		}
		case SQL_ERROR:
		{
			HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
			break;
		}
		default:
			fwprintf(stderr, L"Unexpected return code %hd!\n", RetCode);

		}
		TRYODBC(hStmt, SQL_HANDLE_STMT, SQLFreeStmt(hStmt, SQL_CLOSE));
		wprintf(L"SQL COMMAND>");
	}

	// Free ODBC handles and exit
	wprintf(L"\nDisconnected.");
	return 0;
}

//DisplayResults: display results of a select query
//Parameters:
//hStmt ODBC statement handle
//cCols Count of columns
void DisplayResults(HSTMT hStmt, SQLSMALLINT cCols)
{
	BINDING* pFirstBinding, * pThisBinding;
	SQLSMALLINT cDisplaySize;
	RETCODE RetCode = SQL_SUCCESS;
	int iCount = 0;

	// Allocate memory for each column
	AllocateBindings(hStmt, cCols, &pFirstBinding, &cDisplaySize);

	// Set the display mode and write the titles
	DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);

	// Fetch and display the data
	bool fNoData = false;
	do {
		// Fetch a row
		if (iCount++ >= gHeight - 2)
		{
			int nInputChar;
			bool fEnterReceived = false;

			while (!fEnterReceived)
			{
				wprintf(L" ");
				SetConsole(cDisplaySize + 2, TRUE);
				wprintf(L" Press ENTER to continue, Q to quit (height:%hd)", gHeight);
				SetConsole(cDisplaySize + 2, FALSE);

				nInputChar = _getch();
				wprintf(L"\n");
				if ((nInputChar == 'Q') || (nInputChar == 'q'))
				{
					goto Exit;
				}
				else if ('\r' == nInputChar)
				{
					fEnterReceived = true;
				}
				// else loop back to display prompt again
			}

			iCount = 1;
			DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);
		}

		TRYODBC(hStmt, SQL_HANDLE_STMT, RetCode = SQLFetch(hStmt));
		if (RetCode == SQL_NO_DATA_FOUND)
		{
			fNoData = true;
		}
		else
		{
			// Display the data. Ignore truncations
			for (pThisBinding = pFirstBinding; pThisBinding; pThisBinding = pThisBinding->sNext)
			{
				if (pThisBinding->indPtr != SQL_NULL_DATA)
				{
					wprintf(pThisBinding->fChar ? DISPLAY_FORMAT_C : DISPLAY_FORMAT,
						PIPE,
						pThisBinding->cDisplaySize,
						pThisBinding->cDisplaySize,
						pThisBinding->wszBuffer);
				}
				else
				{
					wprintf(DISPLAY_FORMAT_C,
						PIPE,
						pThisBinding->cDisplaySize,
						pThisBinding->cDisplaySize,
						_T("<NULL>"));
				}
			}
			wprintf(L" %c\n", PIPE);
		}
	} while (!fNoData);

	SetConsole(cDisplaySize + 2, TRUE);
	wprintf(L"%*.*s", cDisplaySize + 2, cDisplaySize + 2, L" ");
	SetConsole(cDisplaySize + 2, FALSE);
	wprintf(L"\n");

Exit:
	// Clean up the allocated buffers
	while (pFirstBinding)
	{
		pThisBinding = pFirstBinding->sNext;
		free(pFirstBinding->wszBuffer);
		free(pFirstBinding);
		pFirstBinding = pThisBinding;
	}
}

//AllocateBindings: Get column information and allocate bindings
//for each column.
//Parameters:
//hStmt Statement handle
//cCols Number of columns in the result set
//*lppBinding Binding pointer (returned)
//lpDisplay Display size of one line
void AllocateBindings(HSTMT hStmt, SQLSMALLINT cCols, BINDING** ppBinding, SQLSMALLINT* pDisplay)
{
	SQLSMALLINT iCol;
	BINDING* pThisBinding, * pLastBinding = NULL;
	SQLLEN cchDisplay, ssType;
	SQLSMALLINT cchColumnNameLength;
	*pDisplay = 0;
	for (iCol = 1; iCol <= cCols; iCol++)
	{
		pThisBinding = (BINDING*)(malloc(sizeof(BINDING)));
		if (!(pThisBinding))
		{
			fwprintf(stderr, L"Out of memory!\n");
			exit(-100);
		}

		if (iCol == 1)
		{
			*ppBinding = pThisBinding;
		}
		else
		{
			pLastBinding->sNext = pThisBinding;
		}
		pLastBinding = pThisBinding;

		// Figure out the display length of the column (we will
		// bind to char since we are only displaying data, in general
		// you should bind to the appropriate C type if you are going
		// to manipulate data since it is much faster...)
		TRYODBC(hStmt, SQL_HANDLE_STMT,
			SQLColAttribute(hStmt, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &cchDisplay));

		// Figure out if this is a character or numeric column; this is
		// used to determine if we want to display the data left- or right-
		// aligned.
		// SQL_DESC_CONCISE_TYPE maps to the 1.x SQL_COLUMN_TYPE.
		// This is what you must use if you want to work
		// against a 2.x driver.
		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLColAttribute(hStmt, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &ssType));

		pThisBinding->fChar = (ssType == SQL_CHAR || ssType == SQL_VARCHAR || ssType == SQL_LONGVARCHAR);
		pThisBinding->sNext = NULL;

		// Arbitrary limit on display size
		if (cchDisplay > DISPLAY_MAX)
		{
			cchDisplay = DISPLAY_MAX;
		}

		// Allocate a buffer big enough to hold the text representation
		// of the data. Add one character for the null terminator
		pThisBinding->wszBuffer = (TCHAR*)malloc((cchDisplay + 1) * sizeof(TCHAR));
		if (!(pThisBinding->wszBuffer))
		{
			fwprintf(stderr, L"Out of memory!\n");
			exit(-100);
		}

		// Map this buffer to the driver's buffer. At Fetch time,
		// the driver will fill in this data. Note that the size is
		// count of bytes (for Unicode). All ODBC functions that take
		// SQLPOINTER use count of bytes; all functions that take only
		// strings use count of characters.
		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLBindCol(hStmt, iCol, SQL_C_TCHAR,
				(SQLPOINTER)pThisBinding->wszBuffer,
				(cchDisplay + 1) * sizeof(TCHAR),
				&pThisBinding->indPtr));

		// Now set the display size that we will use to display
		// the data. Figure out the length of the column name
		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLColAttribute(hStmt, iCol, SQL_DESC_NAME, NULL, 0, &cchColumnNameLength, NULL));

		pThisBinding->cDisplaySize = std::max((SQLSMALLINT)cchDisplay, cchColumnNameLength);
		if (pThisBinding->cDisplaySize < NULL_SIZE)
		{
			pThisBinding->cDisplaySize = NULL_SIZE;
		}

		*pDisplay += pThisBinding->cDisplaySize + DISPLAY_FORMAT_EXTRA;
	}
}

//DisplayTitles: print the titles of all the columns and set the shell window's width
//Parameters:
//hStmt Statement handle
//cDisplaySize Total display size
//pBinding list of binding information
void DisplayTitles(HSTMT hStmt, DWORD cDisplaySize, BINDING* pBinding)
{
	TCHAR wszTitle[DISPLAY_MAX];
	SQLSMALLINT iCol = 1;
	SetConsole(cDisplaySize + 2, TRUE);

	for (; pBinding; pBinding = pBinding->sNext)
	{
		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLColAttribute(hStmt, iCol++, SQL_DESC_NAME, wszTitle,
				sizeof(wszTitle), // Note count of bytes!
				NULL, NULL));
		wprintf(DISPLAY_FORMAT_C, PIPE, pBinding->cDisplaySize, pBinding->cDisplaySize, wszTitle);
	}
	wprintf(L" %c", PIPE);
	SetConsole(cDisplaySize + 2, FALSE);
	wprintf(L"\n");
}

//SetConsole: sets console display size and video mode
//Parameters
//siDisplaySize Console display size
//fInvert Invert video?
void SetConsole(DWORD dwDisplaySize, BOOL fInvert)
{
	HANDLE hConsole;
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;

	// Reset the console screen buffer size if necessary
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo(hConsole, &csbInfo))
		{
			if (csbInfo.dwSize.X < (SHORT)dwDisplaySize)
			{
				csbInfo.dwSize.X = (SHORT)dwDisplaySize;
				SetConsoleScreenBufferSize(hConsole, csbInfo.dwSize);
			}
			gHeight = csbInfo.dwSize.Y;
		}

		if (fInvert)
		{
			SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes | BACKGROUND_BLUE));
		}
		else
		{
			SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes & ~(BACKGROUND_BLUE)));
		}
	}
}

//HandleDiagnosticRecord : display error/warning information
//
//Parameters:
//hHandle ODBC handle
//hType Type of handle (HANDLE_STMT, HANDLE_ENV, HANDLE_DBC)
//RetCode Return code of failing command
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	if (hHandle==SQL_NULL_HANDLE)
	{
		return;
	}

	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	TCHAR wszMessage[1000];
	TCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(TCHAR)), (SQLSMALLINT*)NULL)
		== SQL_SUCCESS
		)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}