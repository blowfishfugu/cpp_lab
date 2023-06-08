#pragma once
//odbc-headerfiles
#include <odbcinst.h>
#include <Sql.h>
#include <Sqlext.h>
#include <Sqltypes.h>
#include <Sqlucode.h>
#include <Msdasql.h>
#include <Msdadc.h>
#include <tuple>
#include <source_location>
#include <iostream>
#include <format>
#include <array>
#include <numeric>
struct odbc_exception : public std::exception
{
	odbc_exception(std::string msg) : std::exception(msg.c_str())
	{}
};



std::tuple<SQLHANDLE, SQLRETURN, bool> getHenv(bool release)
{
	thread_local SQLHANDLE handle = SQL_NULL_HANDLE;
	thread_local SQLRETURN rc = SQL_SUCCESS;
	bool wasNew = false;
	if (!handle)
	{
		rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &handle);
		if (SQL_SUCCEEDED(rc))
		{
			rc = SQLSetEnvAttr(handle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			wasNew = true;
		}
	}
	else if( release )
	{
		rc = SQLFreeHandle(SQL_HANDLE_ENV, handle);
		handle = SQL_NULL_HANDLE;
		wasNew = false;
	}
	return { handle,rc,wasNew };
}

template<bool shouldThrow=false>
void printDiagnostics(SQLRETURN RetCode, SQLSMALLINT handleType, SQLHANDLE context, const std::source_location& loc)
{
	if (RetCode != SQL_SUCCESS)
	{
		if (RetCode == SQL_INVALID_HANDLE)
		{
			std::cerr << std::format("Invalid handle! {} : {}\n", loc.file_name(), loc.line());
			return;
		}
		if (context == SQL_NULL_HANDLE)
		{
			return;
		}

		
		//[ vendor-identifier ][ ODBC-component-identifier ] component-supplied-text
		//[ vendor-identifier ][ ODBC-component-identifier ][ data-source-identifier ] data-source-supplied-text
		SQLSMALLINT returnedMessageSize = 0;
		std::array<char, 1024> wszMessage{};
		SQLINTEGER iError = 0;
		std::array<char, SQL_SQLSTATE_SIZE + 1> wszState{};

		SQLSMALLINT iRec = 0;
		while (SQLGetDiagRec(handleType, context, ++iRec /*startindex=1*/,
			(SQLCHAR*)wszState.data(), &iError, (SQLCHAR*)wszMessage.data(),
			(SQLSMALLINT)wszMessage.size(), //<- ist einfach Anzahl Zeichen, nicht anzahl bytes
			&returnedMessageSize)
			== SQL_SUCCESS
			)
		{
			if (std::string_view stateID{ wszState.data(),5 }; stateID.compare( _T("01004"))!=0 )
			{
				const size_t messageLen = std::min(wszMessage.size() - 1, (size_t)returnedMessageSize);
				std::string_view fullMessage{ wszMessage.data(), messageLen };
				std::cerr << std::format("[{}] {} (NativeError: {}) at {} :Z{}\n", wszState.data(), fullMessage.data(), iError, loc.file_name(), loc.line());
			}
			//reset
			wszMessage.fill(0);//<-memset(&message,0,count)
			wszState.fill(0);
			returnedMessageSize = 0;
			iError = 0;
		}

	}

	if constexpr (shouldThrow)
	{
		if (RetCode == SQL_ERROR)
		{
			throw odbc_exception(std::format("Error in {} : {}", loc.file_name(), loc.line()));
		}
	}
}

struct HEnv {
	SQLHANDLE henv = SQL_NULL_HANDLE;
	bool needsFree = false;
	HEnv() {
		auto [env, lastrc, wasNew] = getHenv(false); //SqlAllocHandle
		henv = env;
		needsFree = wasNew;
		printDiagnostics(lastrc, SQL_HANDLE_ENV, henv, std::source_location::current());
	}
	~HEnv()
	{
		if (needsFree)
		{
			getHenv(true); //SqlFreeHandle
		}
		henv = SQL_NULL_HANDLE;
	}
	operator bool() { return henv != SQL_NULL_HANDLE; }
	operator SQLHANDLE() { return henv; }
};

struct HDbc {
	SQLHANDLE hdbc = SQL_NULL_HANDLE;
	HEnv _env;
	HDbc()
	{
		if (_env)
		{
			SQLRETURN rc=SQLAllocHandle(SQL_HANDLE_DBC, _env, &hdbc);
			printDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		}
	}
	~HDbc()
	{
		if (hdbc)
		{
			SQLDisconnect(hdbc);
			SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			hdbc = SQL_NULL_HANDLE;
		}
	}

	void TryConnect(const TCHAR* pwszConnStr)
	{
		SQLRETURN rc = SQLDriverConnect(
			hdbc, GetDesktopWindow(),
			(SQLTCHAR*)pwszConnStr, //const_cast, weil sqldriverconnect es so möchte
			SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

		printDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
	}

	operator bool() { return hdbc != SQL_NULL_HANDLE; }
	operator SQLHANDLE() { return hdbc; }
};
