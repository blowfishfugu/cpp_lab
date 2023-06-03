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
#include <source_location>
#include <iostream>
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
	odbc_exception( std::string msg ) : std::exception(msg.c_str())
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

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode, const std::source_location& loc);
void DisplayResults(HSTMT hStmt, SQLSMALLINT cCols);
void AllocateBindings(HSTMT hStmt, SQLSMALLINT cCols, BINDING** ppBinding, SQLSMALLINT* pDisplay);
void DisplayTitles(HSTMT hStmt, DWORD cDisplaySize, BINDING* pBinding);
void SetConsole(DWORD cDisplaySize, BOOL fInvert);

// Takes handle, handle type, and stmt
auto TRYODBC = [](SQLHANDLE context, SQLSMALLINT handletype, SQLRETURN rc, std::source_location loc=std::source_location::current()){
	if (rc != SQL_SUCCESS)
	{
		HandleDiagnosticRecord(context, handletype, rc, loc);
	}
	if (rc == SQL_ERROR)
	{
		throw odbc_exception(std::format("Error in {} : {}", loc.file_name(), loc.line()));
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
struct OdbcHandleOwner
{
	SQLHANDLE handle = SQL_NULL_HANDLE;
	SQLSMALLINT type;
	OdbcHandleOwner(SQLHANDLE parentHandle=SQL_NULL_HANDLE)
		: type{handleType}
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
	
	//nicht kopierbar, ansonsten benötigt man bool im destructor, ob freehandle aufgerufen wird
	OdbcHandleOwner(const OdbcHandleOwner&) = delete;
	OdbcHandleOwner(const OdbcHandleOwner&&) = delete;
	OdbcHandleOwner operator=(const OdbcHandleOwner&) = delete;
	OdbcHandleOwner operator=(const OdbcHandleOwner&&) = delete;
	
	operator SQLHANDLE() { return handle; }

	~OdbcHandleOwner()
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


	void TryConnect(const TCHAR* pwszConnStr)
	{
		static_assert(handleType == SQL_HANDLE_DBC, "TryConnect only callable on hdbc-Handles");
		SQLRETURN rc = SQLDriverConnectA(
			handle, GetDesktopWindow(),
			(SQLTCHAR*)pwszConnStr, //const_cast, weil sqldriverconnect es so möchte
			SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

		TRYODBC(handle, handleType, rc);
	}

	void TryExec(TCHAR* wszInput)
	{
		static_assert(handleType == SQL_HANDLE_STMT, "TryExec only callable on STMT-Handles");
		SQLRETURN RetCode = SQLExecDirect(handle, (SQLTCHAR*)wszInput, SQL_NTS);

		switch (RetCode)
		{
		case SQL_SUCCESS_WITH_INFO:
		{
			HandleDiagnosticRecord(handle, handleType, RetCode, std::source_location::current());
			[[fallthrough]];
		}
		case SQL_SUCCESS:
		{
			// If this is a row-returning query, display
			// results
			SQLSMALLINT sNumResults;
			SQLRETURN rc = SQLNumResultCols(handle, &sNumResults);
			TRYODBC(handle, handleType, rc);

			if (sNumResults > 0)
			{
				DisplayResults(handle, sNumResults);
			}
			else
			{
				SQLLEN cRowCount;
				SQLRETURN rc = SQLRowCount(handle, &cRowCount);
				TRYODBC(handle, handleType, rc);
				if (cRowCount >= 0)
				{
					_tprintf(_T("% Id % s affected\n"), cRowCount, cRowCount == 1 ? _T("row") : _T("rows") );
				}
			}
			break;
		}
		case SQL_ERROR:
		{
			HandleDiagnosticRecord(handle, handleType, RetCode, std::source_location::current());
			break;
		}
		default:
			_ftprintf(stderr, _T("Unexpected return code % hd!\n"), RetCode);

		}
		TRYODBC(handle, handleType, SQLFreeStmt(handle, SQL_CLOSE));
	}
};

int __cdecl _tmain(int argc, _In_reads_(argc) TCHAR** argv)
{
	const TCHAR* pwszConnStr = 
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
	try {
		OdbcHandleOwner<SQL_HANDLE_ENV> env;
		OdbcHandleOwner<SQL_HANDLE_DBC> hDbc(env);

		// Connect to the driver. Use the connection string if supplied
		// on the input, otherwise let the driver manager prompt for input.
		hDbc.TryConnect(pwszConnStr);
		_ftprintf(stderr, _T("Connected!\n"));

		OdbcHandleOwner<SQL_HANDLE_STMT> hStmt(hDbc);
		_tprintf(_T("Enter SQL commands, type(control)Z to exit\nSQL COMMAND>"));

		TCHAR wszInput[SQL_QUERY_SIZE]{};
		// Loop to get input and execute queries
		while (_fgetts(wszInput, SQL_QUERY_SIZE - 1, stdin))
		{
			// Execute the query
			if (!(*wszInput))
			{
				_tprintf(_T("SQL COMMAND>"));
				continue;
			}
			if (!_tcsicmp(wszInput, _T("exit\n"))
				||!_tcsicmp(wszInput, _T("quit\n"))
				||!_tcsicmp(wszInput, _T("logout\n")))
			{
				break;
			}
			hStmt.TryExec(wszInput);
			_tprintf(_T("SQL COMMAND>"));
		}
	}
	catch (const odbc_exception& exc)
	{
		std::cerr << exc.what() << "\n";
	}
	// Free ODBC handles and exit
	_tprintf(_T("\nDisconnected."));
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
				_tprintf(_T(" "));
				SetConsole(cDisplaySize + 2, TRUE);
				_tprintf(_T("Press ENTER to continue, Q to quit(height: % hd)"), gHeight);
				SetConsole(cDisplaySize + 2, FALSE);

				nInputChar = _getch();
				_tprintf(_T("\n"));
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
					_tprintf(pThisBinding->fChar ? DISPLAY_FORMAT_C : DISPLAY_FORMAT,
						PIPE,
						pThisBinding->cDisplaySize,
						pThisBinding->cDisplaySize,
						pThisBinding->wszBuffer);
				}
				else
				{
					_tprintf(DISPLAY_FORMAT_C,
						PIPE,
						pThisBinding->cDisplaySize,
						pThisBinding->cDisplaySize,
						_T("<NULL>"));
				}
			}
			_tprintf(_T(" %c\n"), PIPE);
		}
	} while (!fNoData);

	SetConsole(cDisplaySize + 2, TRUE);
	_tprintf(_T("%*.*s"), cDisplaySize + 2, cDisplaySize + 2, _T(" "));
	SetConsole(cDisplaySize + 2, FALSE);
	_tprintf(_T("\n"));

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
			_ftprintf(stderr, _T("Out of memory!\n"));
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
			_ftprintf(stderr, _T("Out of memory!\n"));
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
		_tprintf(DISPLAY_FORMAT_C, PIPE, pBinding->cDisplaySize, pBinding->cDisplaySize, wszTitle);
	}
	_tprintf(_T(" %c"), PIPE);
	SetConsole(cDisplaySize + 2, FALSE);
	_tprintf(_T("\n"));
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
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode, const std::source_location& loc)
{

	if (RetCode == SQL_INVALID_HANDLE)
	{
		_ftprintf(stderr, _T("Invalid handle! %s : %d\n"), loc.file_name(), loc.line());
		return;
	}
	if (hHandle==SQL_NULL_HANDLE)
	{
		return;
	}

	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	SQLTCHAR wszMessage[1000]{};
	TCHAR wszState[SQL_SQLSTATE_SIZE + 1]{};
	while (SQLGetDiagRecA(hType, hHandle, ++iRec /*startindex=1*/, (SQLTCHAR*)wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(SQLTCHAR)), (SQLSMALLINT*)NULL)
		== SQL_SUCCESS
		)
	{
		// Hide data truncated..
		if (_tcsncmp(wszState, _T("01004"), 5))
		{
			_ftprintf(stderr, _T("[%5.5s] %s (%d) at %s : %d\n"), wszState, wszMessage, iError, loc.file_name(),loc.line());
		}
	}
}