#pragma once
#include "bff_diagnostics.h"
#include "bff_odbcinfos.h"
#include "bff_stringquery.h"
#include "bff_boundquery.h"

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
#include <variant>
#include <map>
#include <functional>


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
	else if (release)
	{
		rc = SQLFreeHandle(SQL_HANDLE_ENV, handle);
		handle = SQL_NULL_HANDLE;
		wasNew = false;
	}
	return { handle,rc,wasNew };
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

	using DriverInfo = std::tuple<std::string, SQLSMALLINT, std::string, SQLSMALLINT>;
	std::vector<DriverInfo> GetDrivers()
	{
		if (!henv) { return {}; }

		std::vector<DriverInfo> infos;

		SQLRETURN rc = SQL_SUCCESS;
		SQLUSMALLINT direction = SQL_FETCH_FIRST;
		while (SQL_SUCCEEDED(rc))
		{
			DriverInfo info{};
			SQLSMALLINT& descLen = std::get<1>(info);
			SQLSMALLINT& attrLen = std::get<3>(info);
			rc=SQLDrivers(henv, direction, NULL, NULL, &descLen, NULL, NULL, &attrLen);
			direction = SQL_FETCH_NEXT;
			printDiagnostics(rc, SQL_HANDLE_ENV, henv, std::source_location::current());
			if (!SQL_SUCCEEDED(rc) || rc == SQL_NO_DATA)
			{
				break;
			}
			descLen++; //trailing 0
			attrLen++;
			descLen += (descLen & 0x01); //make even
			attrLen += (attrLen & 0x01);
			infos.emplace_back(info);
		}

		rc = SQL_SUCCESS;
		direction = SQL_FETCH_FIRST;
		size_t pos = 0LL;
		while (SQL_SUCCEEDED(rc) && pos<infos.size())
		{
			DriverInfo& info = infos[pos]; 
			++pos;
			auto& [desc, descLen, attr, attrLen] = info;
			desc.resize(descLen, '\0'); // fuer wchar?: bytes to charcount -> sizeof(decltype(desc)::value_type);
			attr.resize(attrLen, '\0');
			rc = SQLDrivers(henv, direction, 
				(SQLCHAR*)desc.data(), descLen, &descLen,
				(SQLCHAR*)attr.data(), attrLen, &attrLen);
			direction = SQL_FETCH_NEXT;
			printDiagnostics(rc, SQL_HANDLE_ENV, henv, std::source_location::current());
			if (!SQL_SUCCEEDED(rc) || rc == SQL_NO_DATA)
			{
				break;
			}
		}
		return infos;
	}
};



struct HDbc {
	SQLHANDLE hdbc = SQL_NULL_HANDLE;
	HEnv _env;
	bool connected = false;
	//HSTMT, executioncontext.. benötigt alloc/free, aber auch ein CloseCursor nach jeder Abfrage
	//free wird ueber HDbc intern geregelt
	//CloseCursor ueber Query
	SQLHANDLE statement = SQL_NULL_HANDLE; 
	HDbc()
	{
		if (_env)
		{
			SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_DBC, _env, &hdbc);
			printDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		}
	}
	~HDbc()
	{
		if (statement)
		{
			SQLFreeHandle(SQL_HANDLE_STMT, statement);
			statement = SQL_NULL_HANDLE;
		}
		if (hdbc)
		{
			if (connected)
			{
				SQLDisconnect(hdbc);
			}
			SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			hdbc = SQL_NULL_HANDLE;
		}
	}

	void TryConnect(const TCHAR* pwszConnStr)
	{
		if (!connected)
		{
			SQLRETURN rc = SQLDriverConnect(
				hdbc, GetDesktopWindow(),
				(SQLTCHAR*)pwszConnStr, //const_cast, weil sqldriverconnect es so möchte
				SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

			printDiagnostics<true>(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
			connected = SQL_SUCCEEDED(rc);
		}

		if (connected && hdbc && !statement)
		{
			SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &statement);
			printDiagnostics<true>(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
			connected = SQL_SUCCEEDED(rc);
		}
	}
	Query CreateQuery(std::string_view sql) {
		return Query(statement, sql);
	}
	InfoReturn GetInfo(SQLUSMALLINT infoType, std::string name, Getters::GetterFunc infoFunc)
	{
		if (!hdbc) { return { name,std::nullptr_t{}, {{"hdbc is null"}} }; }
		return infoFunc(hdbc, infoType, name);
	}

	InfoReturn GetInfo(SQLUSMALLINT infoType)
	{
		if (!hdbc) { return { "",std::nullptr_t{}, {{"hdbc is null"}} }; }

		if (const auto& infoVal = Getters::infoGetters.find(infoType); 
			infoVal != Getters::infoGetters.end()
			)
		{
			const auto& [name, getter] = infoVal->second;
			return getter(hdbc, infoType, name);
		}
		return { std::format("{}",infoType),std::nullptr_t{}, {{"unknown infoType"}}};
	}

	std::vector<InfoReturn> GetRegisteredInfos()
	{
		if (!hdbc) { return {}; }
		
		std::vector<InfoReturn> infos;
		infos.reserve(Getters::infoGetters.size());
		for (const auto& infoVal : Getters::infoGetters)
		{
			const auto& infoType = infoVal.first;
			const auto& [name, getter] = infoVal.second;
			infos.emplace_back( getter(hdbc, infoType, name) );
		}
		return infos;
	}


	operator bool() { return hdbc != SQL_NULL_HANDLE; }
	operator SQLHANDLE() { return hdbc; }
};

