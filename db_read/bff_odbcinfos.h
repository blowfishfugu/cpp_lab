#pragma once
#include "bff_diagnostics.h"
#include <variant>
#include <tuple>
#include <array>
#include <string>
#include <functional>
#include <map>
#include <iostream>
#include <sqlext.h>
#include <format>

//Name,ReturnValue
using InfoValType = std::variant<std::nullptr_t, std::string, SQLUSMALLINT, SQLUINTEGER>;
using InfoReturn = std::tuple<std::string, InfoValType>;

std::ostream& operator<<(std::ostream& os, const InfoReturn& infoVal)
{
	const auto& [name, info] = infoVal;
	if (std::holds_alternative<std::nullptr_t>(info))
	{
		os << std::format("{}=(null)", name);
	}
	else if (std::holds_alternative<std::string>(info))
	{
		os << std::format("{}={}", name, std::get<std::string>(info));
	}
	else if (std::holds_alternative<SQLUSMALLINT>(info))
	{
		os << std::format("{}={}", name, std::get<SQLUSMALLINT>(info));
	}
	else if (std::holds_alternative<SQLUINTEGER>(info))
	{
		os << std::format("{}={}", name, std::get<SQLUINTEGER>(info));
	}
	return os;
}

namespace Getters
{
	using GetterFunc = std::function<InfoReturn(HDBC, SQLUSMALLINT, const std::string&)>;
	
	InfoReturn _getInfoString(HDBC hdbc, SQLUSMALLINT infoType, const std::string& name)
	{
		std::array<char, 32> infoValue{};
		SQLSMALLINT stringLen = 0;
		SQLRETURN rc = SQLGetInfo(hdbc, infoType, (SQLPOINTER)infoValue.data(), (SQLSMALLINT)infoValue.size(), &stringLen);
		printDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		if (stringLen <= infoValue.size())
		{
			if (SQL_SUCCEEDED(rc))
			{
				std::string_view trimmed{ infoValue.data(),(size_t)stringLen };
				std::string val{ trimmed };
				return { name,val };
			}
			return { name, std::nullptr_t{} };
		}
		
		std::string buf((const size_t)stringLen + 1, '\0');
		stringLen = 0;
		rc = SQLGetInfo(hdbc, infoType, (SQLPOINTER)buf.data(), (SQLSMALLINT)buf.capacity(), &stringLen);
		printDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		if (SQL_SUCCEEDED(rc))
		{
			std::string_view trimmed{ buf.data(),(size_t)stringLen };
			std::string val{ trimmed };
			return { name,val };
		}
		return { name, std::nullptr_t{} };

	};

	template<typename NumType>
	InfoReturn _getNumeric(HDBC hdbc, SQLUSMALLINT infoType, const std::string& name)
	{
		NumType infoValue{};
		SQLRETURN rc = SQLGetInfo(hdbc, infoType, &infoValue, sizeof(NumType), nullptr);

		printDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		if (SQL_SUCCEEDED(rc))
		{
			return { name,infoValue };
		}
		return { name,std::nullptr_t{} };
	}

	//Sammlungen aus https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqlgetinfo-function?view=sql-server-ver16
	static std::vector<SQLUSMALLINT> driverInfos =
	{
SQL_ACTIVE_ENVIRONMENTS					,
SQL_ASYNC_DBC_FUNCTIONS					,
SQL_ASYNC_MODE							,
SQL_ASYNC_NOTIFICATION					,
SQL_BATCH_ROW_COUNT						,
SQL_BATCH_SUPPORT						,
SQL_DATA_SOURCE_NAME					,
SQL_DRIVER_AWARE_POOLING_SUPPORTED		,
SQL_DRIVER_HDBC							,
SQL_DRIVER_HDESC						,
SQL_DRIVER_HENV							,
SQL_DRIVER_HLIB							,
SQL_DRIVER_HSTMT						,
SQL_DRIVER_NAME							,
SQL_DRIVER_ODBC_VER						,
SQL_DRIVER_VER							,
SQL_DYNAMIC_CURSOR_ATTRIBUTES1			,
SQL_DYNAMIC_CURSOR_ATTRIBUTES2			,
SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1		,
SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2		,
SQL_FILE_USAGE							,
SQL_GETDATA_EXTENSIONS					,
SQL_INFO_SCHEMA_VIEWS					,
SQL_KEYSET_CURSOR_ATTRIBUTES1			,
SQL_KEYSET_CURSOR_ATTRIBUTES2			,
SQL_MAX_ASYNC_CONCURRENT_STATEMENTS		,
SQL_MAX_CONCURRENT_ACTIVITIES			,
SQL_MAX_DRIVER_CONNECTIONS				,
SQL_ODBC_INTERFACE_CONFORMANCE			,
SQL_ODBC_VER							,
SQL_PARAM_ARRAY_ROW_COUNTS				,
SQL_PARAM_ARRAY_SELECTS					,
SQL_ROW_UPDATES							,
SQL_SEARCH_PATTERN_ESCAPE				,
SQL_SERVER_NAME							,
SQL_STATIC_CURSOR_ATTRIBUTES1			,
SQL_STATIC_CURSOR_ATTRIBUTES2			,
	};

	static std::vector<SQLUSMALLINT> dbmsInfos =
	{
SQL_DATABASE_NAME						,
SQL_DBMS_NAME							,
SQL_DBMS_VER							,
	};

	static std::vector<SQLUSMALLINT> dataSourceInfos =
	{
SQL_ACCESSIBLE_PROCEDURES				,
SQL_ACCESSIBLE_TABLES					,
SQL_BOOKMARK_PERSISTENCE				,
SQL_CATALOG_TERM						,
SQL_COLLATION_SEQ						,
SQL_CONCAT_NULL_BEHAVIOR				,
SQL_CURSOR_COMMIT_BEHAVIOR				,
SQL_CURSOR_ROLLBACK_BEHAVIOR			,
SQL_CURSOR_SENSITIVITY					,
SQL_DATA_SOURCE_READ_ONLY				,
SQL_DEFAULT_TXN_ISOLATION				,
SQL_DESCRIBE_PARAMETER					,
SQL_MULT_RESULT_SETS					,
SQL_MULTIPLE_ACTIVE_TXN					,
SQL_NEED_LONG_DATA_LEN					,
SQL_NULL_COLLATION						,
SQL_PROCEDURE_TERM						,
SQL_SCHEMA_TERM							,
SQL_SCROLL_OPTIONS						,
SQL_TABLE_TERM							,
SQL_TXN_CAPABLE							,
SQL_TXN_ISOLATION_OPTION				,
SQL_USER_NAME							,
	};

	static std::vector<SQLUSMALLINT> infoSqlCapabilities =
	{
SQL_AGGREGATE_FUNCTIONS					 ,
SQL_ALTER_DOMAIN						 ,
SQL_ALTER_TABLE							 ,
SQL_CATALOG_LOCATION					 ,
SQL_CATALOG_NAME						 ,
SQL_CATALOG_NAME_SEPARATOR				 ,
SQL_CATALOG_USAGE						 ,
SQL_COLUMN_ALIAS						 ,
SQL_CORRELATION_NAME					 ,
SQL_CREATE_ASSERTION					 ,
SQL_CREATE_CHARACTER_SET				 ,
SQL_CREATE_COLLATION					 ,
SQL_CREATE_DOMAIN						 ,
SQL_CREATE_SCHEMA						 ,
SQL_CREATE_TABLE						 ,
SQL_CREATE_TRANSLATION					 ,
SQL_DDL_INDEX							 ,
SQL_DROP_ASSERTION						 ,
SQL_DROP_CHARACTER_SET					 ,
SQL_DROP_COLLATION						 ,
SQL_DROP_DOMAIN							 ,
SQL_DROP_SCHEMA							 ,
SQL_DROP_TABLE							 ,
SQL_DROP_TRANSLATION					 ,
SQL_DROP_VIEW							 ,
SQL_EXPRESSIONS_IN_ORDERBY				 ,
SQL_GROUP_BY							 ,
SQL_IDENTIFIER_CASE						 ,
SQL_IDENTIFIER_QUOTE_CHAR				 ,
SQL_INDEX_KEYWORDS						 ,
SQL_INSERT_STATEMENT					 ,
SQL_INTEGRITY							 ,
SQL_KEYWORDS							 ,
SQL_LIKE_ESCAPE_CLAUSE					 ,
SQL_NON_NULLABLE_COLUMNS				 ,
SQL_OJ_CAPABILITIES						 ,
SQL_ORDER_BY_COLUMNS_IN_SELECT			 ,
SQL_OUTER_JOINS							 ,
SQL_PROCEDURES							 ,
SQL_QUOTED_IDENTIFIER_CASE				 ,
SQL_SCHEMA_USAGE						 ,
SQL_SPECIAL_CHARACTERS					 ,
SQL_SQL_CONFORMANCE						 ,
SQL_SUBQUERIES							 ,
SQL_UNION								 ,
	};

	static std::vector<SQLUSMALLINT> infoSqlLimits =
	{
SQL_MAX_BINARY_LITERAL_LEN				  ,
SQL_MAX_CATALOG_NAME_LEN				  ,
SQL_MAX_CHAR_LITERAL_LEN				  ,
SQL_MAX_COLUMN_NAME_LEN					  ,
SQL_MAX_COLUMNS_IN_GROUP_BY				  ,
SQL_MAX_COLUMNS_IN_INDEX				  ,
SQL_MAX_COLUMNS_IN_ORDER_BY				  ,
SQL_MAX_COLUMNS_IN_SELECT				  ,
SQL_MAX_COLUMNS_IN_TABLE				  ,
SQL_MAX_CURSOR_NAME_LEN					  ,
SQL_MAX_IDENTIFIER_LEN					  ,
SQL_MAX_INDEX_SIZE						  ,
SQL_MAX_PROCEDURE_NAME_LEN				  ,
SQL_MAX_ROW_SIZE						  ,
SQL_MAX_ROW_SIZE_INCLUDES_LONG			  ,
SQL_MAX_SCHEMA_NAME_LEN					  ,
SQL_MAX_STATEMENT_LEN					  ,
SQL_MAX_TABLE_NAME_LEN					  ,
SQL_MAX_TABLES_IN_SELECT				  ,
SQL_MAX_USER_NAME_LEN					  ,
	};

	static std::vector<SQLUSMALLINT> infoScalarFunctions =
	{
SQL_CONVERT_FUNCTIONS					,
SQL_NUMERIC_FUNCTIONS					,
SQL_STRING_FUNCTIONS					,
SQL_SYSTEM_FUNCTIONS					,
SQL_TIMEDATE_ADD_INTERVALS				,
SQL_TIMEDATE_DIFF_INTERVALS				,
SQL_TIMEDATE_FUNCTIONS					,
	};

	static std::vector<SQLUSMALLINT> infoConvertibles =
	{
SQL_CONVERT_BIGINT						 ,
SQL_CONVERT_BINARY						 ,
SQL_CONVERT_BIT							 ,
SQL_CONVERT_CHAR						 ,
SQL_CONVERT_DATE						 ,
SQL_CONVERT_DECIMAL						 ,
SQL_CONVERT_DOUBLE						 ,
SQL_CONVERT_FLOAT						 ,
SQL_CONVERT_INTEGER						 ,
SQL_CONVERT_INTERVAL_DAY_TIME			 ,
SQL_CONVERT_INTERVAL_YEAR_MONTH			 ,
SQL_CONVERT_LONGVARBINARY				 ,
SQL_CONVERT_LONGVARCHAR					 ,
SQL_CONVERT_NUMERIC						 ,
SQL_CONVERT_REAL						 ,
SQL_CONVERT_SMALLINT					 ,
SQL_CONVERT_TIME						 ,
SQL_CONVERT_TIMESTAMP					 ,
SQL_CONVERT_TINYINT						 ,
SQL_CONVERT_VARBINARY					 ,
SQL_CONVERT_VARCHAR						 ,
	};

	static std::map < SQLUSMALLINT, std::tuple<std::string, GetterFunc>> infoGetters =
	{
		{ SQL_DATABASE_NAME, { "SQL_DATABASE_NAME", _getInfoString } },
		{ SQL_DBMS_VER, { "SQL_DBMS_VER", _getInfoString } },
		{ SQL_DBMS_NAME, { "SQL_DBMS_NAME", _getInfoString } },
		{ SQL_ACTIVE_ENVIRONMENTS, { "SQL_ACTIVE_ENVIRONMENTS", _getNumeric<SQLUSMALLINT>} },
		{ SQL_MAX_CONCURRENT_ACTIVITIES, {"SQL_MAX_CONCURRENT_ACTIVITIES (max active statements per connection)", _getNumeric<SQLUSMALLINT>} },
		{ SQL_KEYWORDS, {"SQL_KEYWORDS", _getInfoString}},
	};

}