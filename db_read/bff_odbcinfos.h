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
#include <optional>
#include <vector>

struct BitMask
{
	SQLUINTEGER val = 0L;
	BitMask(const SQLUINTEGER v) :val(v) {};
	BitMask()=default;
	BitMask& operator=(SQLUINTEGER u) { val = u; return *this; }
	operator SQLUINTEGER() { return val; }
	bool IsSet(SQLUINTEGER flags) const { return (val & flags); }
};

using InfoValType = std::variant<std::nullptr_t, std::string, SQLUSMALLINT, SQLUINTEGER, SQLULEN,BitMask>;
//Name,ReturnValue,Diagnostic-Messages
using InfoReturn = std::tuple<std::string, InfoValType, std::optional<std::vector<std::string>> >;

//kann man da early abort machen? entfaltet VariantType �ber alle m�glichkeiten
//template<typename... VariantType>
//void CallCout(std::ostream& os, std::variant<VariantType...> const& info)
//{
//	if (std::holds_alternative<VariantType>(info))
//	{
//		_cout(os, std::get<VariantType>(info));
//	}
//}

std::ostream& operator<<(std::ostream& os, const InfoReturn& infoVal)
{
	const auto& [name, info,diagnostics] = infoVal;
	//std::visit([&os](auto const& e){ os << e; }, info);
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
	else if (std::holds_alternative<BitMask>(info))
	{
		constexpr int bits = sizeof(SQLUINTEGER)*8;
		os << std::format("{}={:#0{}b}", name, std::get<BitMask>(info).val,bits );
	}
	else if (std::holds_alternative<SQLULEN>(info))
	{
		os << std::format("{}={:#x}", name, std::get<SQLULEN>(info));
	}

	if (diagnostics)
	{
		for (const auto& diag : *diagnostics)
		{
			os << std::format("\t{}", diag);
		}
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
		auto diagnostics=fetchDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		if (stringLen <= infoValue.size())
		{
			if (SQL_SUCCEEDED(rc))
			{
				std::string_view trimmed{ infoValue.data(),(size_t)stringLen };
				std::string val{ trimmed };
				return { name,val,diagnostics };
			}
			return { name, std::nullptr_t{},diagnostics };
		}
		
		std::string buf((const size_t)stringLen + 1, '\0');
		stringLen = 0;
		rc = SQLGetInfo(hdbc, infoType, (SQLPOINTER)buf.data(), (SQLSMALLINT)buf.capacity(), &stringLen);
		diagnostics=fetchDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		if (SQL_SUCCEEDED(rc))
		{
			std::string_view trimmed{ buf.data(),(size_t)stringLen };
			std::string val{ trimmed };
			return { name,val,diagnostics };
		}
		return { name, std::nullptr_t{},diagnostics };

	};

	template<typename NumType>
	InfoReturn _getNumeric(HDBC hdbc, SQLUSMALLINT infoType, const std::string& name)
	{
		NumType infoValue{};
		SQLRETURN rc = SQLGetInfo(hdbc, infoType, &infoValue, sizeof(NumType), nullptr);

		auto diagnostics=fetchDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		if (SQL_SUCCEEDED(rc))
		{
			return { name,infoValue,diagnostics };
		}
		return { name,std::nullptr_t{},diagnostics };
	}

	template<>
	InfoReturn _getNumeric<BitMask>(HDBC hdbc, SQLUSMALLINT infoType, const std::string& name)
	{
		BitMask infoValue{};
		SQLRETURN rc = SQLGetInfo(hdbc, infoType, &infoValue.val, sizeof(infoValue.val), nullptr);

		auto diagnostics=fetchDiagnostics(rc, SQL_HANDLE_DBC, hdbc, std::source_location::current());
		if (SQL_SUCCEEDED(rc))
		{
			return { name,infoValue,diagnostics };
		}
		return { name,std::nullptr_t{},diagnostics };
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

	//Generiert per PrepInfoGetters
	static std::map < SQLUSMALLINT, std::tuple<std::string, GetterFunc>> infoGetters =
	{
	{ SQL_ACCESSIBLE_PROCEDURES, { "SQL_ACCESSIBLE_PROCEDURES", _getInfoString } },
	{ SQL_ACCESSIBLE_TABLES, { "SQL_ACCESSIBLE_TABLES", _getInfoString } },
	{ SQL_ACTIVE_ENVIRONMENTS, { "SQL_ACTIVE_ENVIRONMENTS", _getNumeric<SQLUSMALLINT> } },
	{ SQL_AGGREGATE_FUNCTIONS, { "SQL_AGGREGATE_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_ALTER_DOMAIN, { "SQL_ALTER_DOMAIN", _getNumeric<BitMask> } },
	{ SQL_ALTER_TABLE, { "SQL_ALTER_TABLE", _getNumeric<BitMask> } },
	{ SQL_ASYNC_DBC_FUNCTIONS, { "SQL_ASYNC_DBC_FUNCTIONS", _getNumeric<SQLUINTEGER> } },
	{ SQL_ASYNC_MODE, { "SQL_ASYNC_MODE", _getNumeric<SQLUINTEGER> } },
	{ SQL_ASYNC_NOTIFICATION, { "SQL_ASYNC_NOTIFICATION", _getNumeric<SQLUINTEGER> } },
	{ SQL_BATCH_ROW_COUNT, { "SQL_BATCH_ROW_COUNT", _getNumeric<BitMask> } },
	{ SQL_BATCH_SUPPORT, { "SQL_BATCH_SUPPORT", _getNumeric<BitMask> } },
	{ SQL_BOOKMARK_PERSISTENCE, { "SQL_BOOKMARK_PERSISTENCE", _getNumeric<BitMask> } },
	{ SQL_CATALOG_LOCATION, { "SQL_CATALOG_LOCATION", _getNumeric<SQLUSMALLINT> } },
	{ SQL_CATALOG_NAME, { "SQL_CATALOG_NAME", _getInfoString } },
	{ SQL_CATALOG_NAME_SEPARATOR, { "SQL_CATALOG_NAME_SEPARATOR", _getInfoString } },
	{ SQL_CATALOG_TERM, { "SQL_CATALOG_TERM", _getInfoString } },
	{ SQL_CATALOG_USAGE, { "SQL_CATALOG_USAGE", _getNumeric<BitMask> } },
	{ SQL_COLLATION_SEQ, { "SQL_COLLATION_SEQ", _getInfoString } },
	{ SQL_COLUMN_ALIAS, { "SQL_COLUMN_ALIAS", _getInfoString } },
	{ SQL_CONCAT_NULL_BEHAVIOR, { "SQL_CONCAT_NULL_BEHAVIOR", _getNumeric<SQLUSMALLINT> } },
	{ SQL_CONVERT_BIGINT, { "SQL_CONVERT_BIGINT", _getNumeric<BitMask> } },
	{ SQL_CONVERT_BINARY, { "SQL_CONVERT_BINARY", _getNumeric<BitMask> } },
	{ SQL_CONVERT_BIT, { "SQL_CONVERT_BIT", _getNumeric<BitMask> } },
	{ SQL_CONVERT_CHAR, { "SQL_CONVERT_CHAR", _getNumeric<BitMask> } },
	{ SQL_CONVERT_GUID, { "SQL_CONVERT_GUID", _getNumeric<BitMask> } },
	{ SQL_CONVERT_DATE, { "SQL_CONVERT_DATE", _getNumeric<BitMask> } },
	{ SQL_CONVERT_DECIMAL, { "SQL_CONVERT_DECIMAL", _getNumeric<BitMask> } },
	{ SQL_CONVERT_DOUBLE, { "SQL_CONVERT_DOUBLE", _getNumeric<BitMask> } },
	{ SQL_CONVERT_FLOAT, { "SQL_CONVERT_FLOAT", _getNumeric<BitMask> } },
	{ SQL_CONVERT_INTEGER, { "SQL_CONVERT_INTEGER", _getNumeric<BitMask> } },
	{ SQL_CONVERT_INTERVAL_YEAR_MONTH, { "SQL_CONVERT_INTERVAL_YEAR_MONTH", _getNumeric<BitMask> } },
	{ SQL_CONVERT_INTERVAL_DAY_TIME, { "SQL_CONVERT_INTERVAL_DAY_TIME", _getNumeric<BitMask> } },
	{ SQL_CONVERT_LONGVARBINARY, { "SQL_CONVERT_LONGVARBINARY", _getNumeric<BitMask> } },
	{ SQL_CONVERT_LONGVARCHAR, { "SQL_CONVERT_LONGVARCHAR", _getNumeric<BitMask> } },
	{ SQL_CONVERT_NUMERIC, { "SQL_CONVERT_NUMERIC", _getNumeric<BitMask> } },
	{ SQL_CONVERT_REAL, { "SQL_CONVERT_REAL", _getNumeric<BitMask> } },
	{ SQL_CONVERT_SMALLINT, { "SQL_CONVERT_SMALLINT", _getNumeric<BitMask> } },
	{ SQL_CONVERT_TIME, { "SQL_CONVERT_TIME", _getNumeric<BitMask> } },
	{ SQL_CONVERT_TIMESTAMP, { "SQL_CONVERT_TIMESTAMP", _getNumeric<BitMask> } },
	{ SQL_CONVERT_TINYINT, { "SQL_CONVERT_TINYINT", _getNumeric<BitMask> } },
	{ SQL_CONVERT_VARBINARY, { "SQL_CONVERT_VARBINARY", _getNumeric<BitMask> } },
	{ SQL_CONVERT_VARCHAR, { "SQL_CONVERT_VARCHAR", _getNumeric<BitMask> } },
	{ SQL_CONVERT_FUNCTIONS, { "SQL_CONVERT_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_CORRELATION_NAME, { "SQL_CORRELATION_NAME", _getNumeric<SQLUSMALLINT> } },
	{ SQL_CREATE_ASSERTION, { "SQL_CREATE_ASSERTION", _getNumeric<BitMask> } },
	{ SQL_CREATE_CHARACTER_SET, { "SQL_CREATE_CHARACTER_SET", _getNumeric<BitMask> } },
	{ SQL_CREATE_COLLATION, { "SQL_CREATE_COLLATION", _getNumeric<BitMask> } },
	{ SQL_CREATE_DOMAIN, { "SQL_CREATE_DOMAIN", _getNumeric<BitMask> } },
	{ SQL_CREATE_SCHEMA, { "SQL_CREATE_SCHEMA", _getNumeric<BitMask> } },
	{ SQL_CREATE_TABLE, { "SQL_CREATE_TABLE", _getNumeric<BitMask> } },
	{ SQL_CREATE_TRANSLATION, { "SQL_CREATE_TRANSLATION", _getNumeric<BitMask> } },
	{ SQL_CREATE_VIEW, { "SQL_CREATE_VIEW", _getNumeric<BitMask> } },
	{ SQL_CURSOR_COMMIT_BEHAVIOR, { "SQL_CURSOR_COMMIT_BEHAVIOR", _getNumeric<SQLUSMALLINT> } },
	{ SQL_CURSOR_ROLLBACK_BEHAVIOR, { "SQL_CURSOR_ROLLBACK_BEHAVIOR", _getNumeric<SQLUSMALLINT> } },
	{ SQL_CURSOR_SENSITIVITY, { "SQL_CURSOR_SENSITIVITY", _getNumeric<SQLUINTEGER> } },
	{ SQL_DATA_SOURCE_NAME, { "SQL_DATA_SOURCE_NAME", _getInfoString } },
	{ SQL_DATA_SOURCE_READ_ONLY, { "SQL_DATA_SOURCE_READ_ONLY", _getInfoString } },
	{ SQL_DATABASE_NAME, { "SQL_DATABASE_NAME", _getInfoString } },
	{ SQL_DATETIME_LITERALS, { "SQL_DATETIME_LITERALS", _getNumeric<BitMask> } },
	{ SQL_DBMS_NAME, { "SQL_DBMS_NAME", _getInfoString } },
	{ SQL_DBMS_VER, { "SQL_DBMS_VER", _getInfoString } },
	{ SQL_DDL_INDEX, { "SQL_DDL_INDEX", _getNumeric<SQLUINTEGER> } },
	{ SQL_DEFAULT_TXN_ISOLATION, { "SQL_DEFAULT_TXN_ISOLATION", _getNumeric<SQLUINTEGER> } },
	{ SQL_DESCRIBE_PARAMETER, { "SQL_DESCRIBE_PARAMETER", _getInfoString } },
	{ SQL_DM_VER, { "SQL_DM_VER", _getInfoString } },
	{ SQL_DRIVER_AWARE_POOLING_SUPPORTED, { "SQL_DRIVER_AWARE_POOLING_SUPPORTED", _getNumeric<SQLUINTEGER> } },
	{ SQL_DRIVER_HDBC, { "SQL_DRIVER_HDBC", _getNumeric<SQLULEN> } },
	{ SQL_DRIVER_HENV, { "SQL_DRIVER_HENV", _getNumeric<SQLULEN> } },
	{ SQL_DRIVER_HDESC, { "SQL_DRIVER_HDESC", _getNumeric<SQLULEN> } },
	{ SQL_DRIVER_HLIB, { "SQL_DRIVER_HLIB", _getNumeric<SQLULEN> } },
	{ SQL_DRIVER_HSTMT, { "SQL_DRIVER_HSTMT", _getNumeric<SQLULEN> } },
	{ SQL_DRIVER_NAME, { "SQL_DRIVER_NAME", _getInfoString } },
	{ SQL_DRIVER_ODBC_VER, { "SQL_DRIVER_ODBC_VER", _getInfoString } },
	{ SQL_DRIVER_VER, { "SQL_DRIVER_VER", _getInfoString } },
	{ SQL_DROP_ASSERTION, { "SQL_DROP_ASSERTION", _getNumeric<BitMask> } },
	{ SQL_DROP_CHARACTER_SET, { "SQL_DROP_CHARACTER_SET", _getNumeric<BitMask> } },
	{ SQL_DROP_COLLATION, { "SQL_DROP_COLLATION", _getNumeric<BitMask> } },
	{ SQL_DROP_DOMAIN, { "SQL_DROP_DOMAIN", _getNumeric<BitMask> } },
	{ SQL_DROP_SCHEMA, { "SQL_DROP_SCHEMA", _getNumeric<BitMask> } },
	{ SQL_DROP_TABLE, { "SQL_DROP_TABLE", _getNumeric<BitMask> } },
	{ SQL_DROP_TRANSLATION, { "SQL_DROP_TRANSLATION", _getNumeric<BitMask> } },
	{ SQL_DROP_VIEW, { "SQL_DROP_VIEW", _getNumeric<BitMask> } },
	{ SQL_DYNAMIC_CURSOR_ATTRIBUTES1, { "SQL_DYNAMIC_CURSOR_ATTRIBUTES1", _getNumeric<BitMask> } },
	{ SQL_DYNAMIC_CURSOR_ATTRIBUTES2, { "SQL_DYNAMIC_CURSOR_ATTRIBUTES2", _getNumeric<BitMask> } },
	{ SQL_EXPRESSIONS_IN_ORDERBY, { "SQL_EXPRESSIONS_IN_ORDERBY", _getInfoString } },
	{ SQL_FILE_USAGE, { "SQL_FILE_USAGE", _getNumeric<SQLUSMALLINT> } },
	{ SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, { "SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1", _getNumeric<BitMask> } },
	{ SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, { "SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2", _getNumeric<BitMask> } },
	{ SQL_GETDATA_EXTENSIONS, { "SQL_GETDATA_EXTENSIONS", _getNumeric<BitMask> } },
	{ SQL_GROUP_BY, { "SQL_GROUP_BY", _getNumeric<SQLUSMALLINT> } },
	{ SQL_IDENTIFIER_CASE, { "SQL_IDENTIFIER_CASE", _getNumeric<SQLUSMALLINT> } },
	{ SQL_IDENTIFIER_QUOTE_CHAR, { "SQL_IDENTIFIER_QUOTE_CHAR", _getInfoString } },
	{ SQL_INDEX_KEYWORDS, { "SQL_INDEX_KEYWORDS", _getNumeric<BitMask> } },
	{ SQL_INFO_SCHEMA_VIEWS, { "SQL_INFO_SCHEMA_VIEWS", _getNumeric<BitMask> } },
	{ SQL_INSERT_STATEMENT, { "SQL_INSERT_STATEMENT", _getNumeric<BitMask> } },
	{ SQL_INTEGRITY, { "SQL_INTEGRITY", _getInfoString } },
	{ SQL_KEYSET_CURSOR_ATTRIBUTES1, { "SQL_KEYSET_CURSOR_ATTRIBUTES1", _getNumeric<BitMask> } },
	{ SQL_KEYSET_CURSOR_ATTRIBUTES2, { "SQL_KEYSET_CURSOR_ATTRIBUTES2", _getNumeric<BitMask> } },
	{ SQL_KEYWORDS, { "SQL_KEYWORDS", _getInfoString } },
	{ SQL_LIKE_ESCAPE_CLAUSE, { "SQL_LIKE_ESCAPE_CLAUSE", _getInfoString } },
	{ SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, { "SQL_MAX_ASYNC_CONCURRENT_STATEMENTS", _getNumeric<SQLUINTEGER> } },
	{ SQL_MAX_BINARY_LITERAL_LEN, { "SQL_MAX_BINARY_LITERAL_LEN", _getNumeric<SQLUINTEGER> } },
	{ SQL_MAX_CATALOG_NAME_LEN, { "SQL_MAX_CATALOG_NAME_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_CHAR_LITERAL_LEN, { "SQL_MAX_CHAR_LITERAL_LEN", _getNumeric<SQLUINTEGER> } },
	{ SQL_MAX_COLUMN_NAME_LEN, { "SQL_MAX_COLUMN_NAME_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_COLUMNS_IN_GROUP_BY, { "SQL_MAX_COLUMNS_IN_GROUP_BY", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_COLUMNS_IN_INDEX, { "SQL_MAX_COLUMNS_IN_INDEX", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_COLUMNS_IN_ORDER_BY, { "SQL_MAX_COLUMNS_IN_ORDER_BY", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_COLUMNS_IN_SELECT, { "SQL_MAX_COLUMNS_IN_SELECT", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_COLUMNS_IN_TABLE, { "SQL_MAX_COLUMNS_IN_TABLE", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_CONCURRENT_ACTIVITIES, { "SQL_MAX_CONCURRENT_ACTIVITIES", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_CURSOR_NAME_LEN, { "SQL_MAX_CURSOR_NAME_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_DRIVER_CONNECTIONS, { "SQL_MAX_DRIVER_CONNECTIONS", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_IDENTIFIER_LEN, { "SQL_MAX_IDENTIFIER_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_INDEX_SIZE, { "SQL_MAX_INDEX_SIZE", _getNumeric<SQLUINTEGER> } },
	{ SQL_MAX_PROCEDURE_NAME_LEN, { "SQL_MAX_PROCEDURE_NAME_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_ROW_SIZE, { "SQL_MAX_ROW_SIZE", _getNumeric<SQLUINTEGER> } },
	{ SQL_MAX_ROW_SIZE_INCLUDES_LONG, { "SQL_MAX_ROW_SIZE_INCLUDES_LONG", _getInfoString } },
	{ SQL_MAX_SCHEMA_NAME_LEN, { "SQL_MAX_SCHEMA_NAME_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_STATEMENT_LEN, { "SQL_MAX_STATEMENT_LEN", _getNumeric<SQLUINTEGER> } },
	{ SQL_MAX_TABLE_NAME_LEN, { "SQL_MAX_TABLE_NAME_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_TABLES_IN_SELECT, { "SQL_MAX_TABLES_IN_SELECT", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MAX_USER_NAME_LEN, { "SQL_MAX_USER_NAME_LEN", _getNumeric<SQLUSMALLINT> } },
	{ SQL_MULT_RESULT_SETS, { "SQL_MULT_RESULT_SETS", _getInfoString } },
	{ SQL_MULTIPLE_ACTIVE_TXN, { "SQL_MULTIPLE_ACTIVE_TXN", _getInfoString } },
	{ SQL_NEED_LONG_DATA_LEN, { "SQL_NEED_LONG_DATA_LEN", _getInfoString } },
	{ SQL_NON_NULLABLE_COLUMNS, { "SQL_NON_NULLABLE_COLUMNS", _getNumeric<SQLUSMALLINT> } },
	{ SQL_NULL_COLLATION, { "SQL_NULL_COLLATION", _getNumeric<SQLUSMALLINT> } },
	{ SQL_NUMERIC_FUNCTIONS, { "SQL_NUMERIC_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_ODBC_INTERFACE_CONFORMANCE, { "SQL_ODBC_INTERFACE_CONFORMANCE", _getNumeric<SQLUINTEGER> } },
	{ SQL_ODBC_VER, { "SQL_ODBC_VER", _getInfoString } },
	{ SQL_OJ_CAPABILITIES, { "SQL_OJ_CAPABILITIES", _getNumeric<BitMask> } },
	{ SQL_ORDER_BY_COLUMNS_IN_SELECT, { "SQL_ORDER_BY_COLUMNS_IN_SELECT", _getInfoString } },
	{ SQL_PARAM_ARRAY_ROW_COUNTS, { "SQL_PARAM_ARRAY_ROW_COUNTS", _getNumeric<SQLUINTEGER> } },
	{ SQL_PARAM_ARRAY_SELECTS, { "SQL_PARAM_ARRAY_SELECTS", _getNumeric<SQLUINTEGER> } },
	{ SQL_POS_OPERATIONS, { "SQL_POS_OPERATIONS", _getNumeric<SQLUINTEGER> } },
	{ SQL_PROCEDURE_TERM, { "SQL_PROCEDURE_TERM", _getInfoString } },
	{ SQL_PROCEDURES, { "SQL_PROCEDURES", _getInfoString } },
	{ SQL_QUOTED_IDENTIFIER_CASE, { "SQL_QUOTED_IDENTIFIER_CASE", _getNumeric<SQLUSMALLINT> } },
	{ SQL_ROW_UPDATES, { "SQL_ROW_UPDATES", _getInfoString } },
	{ SQL_SCHEMA_TERM, { "SQL_SCHEMA_TERM", _getInfoString } },
	{ SQL_SCHEMA_USAGE, { "SQL_SCHEMA_USAGE", _getNumeric<BitMask> } },
	{ SQL_SCROLL_OPTIONS, { "SQL_SCROLL_OPTIONS", _getNumeric<BitMask> } },
	{ SQL_SEARCH_PATTERN_ESCAPE, { "SQL_SEARCH_PATTERN_ESCAPE", _getInfoString } },
	{ SQL_SERVER_NAME, { "SQL_SERVER_NAME", _getInfoString } },
	{ SQL_SPECIAL_CHARACTERS, { "SQL_SPECIAL_CHARACTERS", _getInfoString } },
	{ SQL_SQL_CONFORMANCE, { "SQL_SQL_CONFORMANCE", _getNumeric<SQLUINTEGER> } },
	{ SQL_SQL92_DATETIME_FUNCTIONS, { "SQL_SQL92_DATETIME_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_SQL92_FOREIGN_KEY_DELETE_RULE, { "SQL_SQL92_FOREIGN_KEY_DELETE_RULE", _getNumeric<BitMask> } },
	{ SQL_SQL92_FOREIGN_KEY_UPDATE_RULE, { "SQL_SQL92_FOREIGN_KEY_UPDATE_RULE", _getNumeric<BitMask> } },
	{ SQL_SQL92_GRANT, { "SQL_SQL92_GRANT", _getNumeric<BitMask> } },
	{ SQL_SQL92_NUMERIC_VALUE_FUNCTIONS, { "SQL_SQL92_NUMERIC_VALUE_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_SQL92_PREDICATES, { "SQL_SQL92_PREDICATES", _getNumeric<BitMask> } },
	{ SQL_SQL92_RELATIONAL_JOIN_OPERATORS, { "SQL_SQL92_RELATIONAL_JOIN_OPERATORS", _getNumeric<BitMask> } },
	{ SQL_SQL92_REVOKE, { "SQL_SQL92_REVOKE", _getNumeric<BitMask> } },
	{ SQL_SQL92_ROW_VALUE_CONSTRUCTOR, { "SQL_SQL92_ROW_VALUE_CONSTRUCTOR", _getNumeric<BitMask> } },
	{ SQL_SQL92_STRING_FUNCTIONS, { "SQL_SQL92_STRING_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_SQL92_VALUE_EXPRESSIONS, { "SQL_SQL92_VALUE_EXPRESSIONS", _getNumeric<BitMask> } },
	{ SQL_STANDARD_CLI_CONFORMANCE, { "SQL_STANDARD_CLI_CONFORMANCE", _getNumeric<BitMask> } },
	{ SQL_STATIC_CURSOR_ATTRIBUTES1, { "SQL_STATIC_CURSOR_ATTRIBUTES1", _getNumeric<BitMask> } },
	{ SQL_STATIC_CURSOR_ATTRIBUTES2, { "SQL_STATIC_CURSOR_ATTRIBUTES2", _getNumeric<BitMask> } },
	{ SQL_STRING_FUNCTIONS, { "SQL_STRING_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_SUBQUERIES, { "SQL_SUBQUERIES", _getNumeric<BitMask> } },
	{ SQL_SYSTEM_FUNCTIONS, { "SQL_SYSTEM_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_TABLE_TERM, { "SQL_TABLE_TERM", _getInfoString } },
	{ SQL_TIMEDATE_ADD_INTERVALS, { "SQL_TIMEDATE_ADD_INTERVALS", _getNumeric<BitMask> } },
	{ SQL_TIMEDATE_DIFF_INTERVALS, { "SQL_TIMEDATE_DIFF_INTERVALS", _getNumeric<BitMask> } },
	{ SQL_TIMEDATE_FUNCTIONS, { "SQL_TIMEDATE_FUNCTIONS", _getNumeric<BitMask> } },
	{ SQL_TXN_CAPABLE, { "SQL_TXN_CAPABLE", _getNumeric<SQLUSMALLINT> } },
	{ SQL_TXN_ISOLATION_OPTION, { "SQL_TXN_ISOLATION_OPTION", _getNumeric<BitMask> } },
	{ SQL_UNION, { "SQL_UNION", _getNumeric<BitMask> } },
	{ SQL_USER_NAME, { "SQL_USER_NAME", _getInfoString } },
	{ SQL_XOPEN_CLI_YEAR, { "SQL_XOPEN_CLI_YEAR", _getInfoString } },
	};




}