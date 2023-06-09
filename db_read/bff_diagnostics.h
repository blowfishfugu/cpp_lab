#pragma once
#include <exception>
#include <string>
#include <sqlext.h>
#include <format>
#include <source_location>
#include <iostream>
#include <vector>
#include <array>
struct odbc_exception : public std::exception
{
	odbc_exception(std::string msg) : std::exception(msg.c_str())
	{}
};

template<bool shouldThrow = false>
void printDiagnostics(SQLRETURN RetCode, SQLSMALLINT handleType, SQLHANDLE context, const std::source_location& loc)
{
	if (RetCode != SQL_SUCCESS)
	{
		if (RetCode == SQL_INVALID_HANDLE)
		{
			std::cerr << std::format("Invalid handle! {} : {}\n", loc.function_name(), loc.line());
			return;
		}
		if (context == SQL_NULL_HANDLE)
		{
			std::cerr << std::format("NULL handle! {} : {}\n", loc.function_name(), loc.line());
			return;
		}


		//[ vendor-identifier ][ ODBC-component-identifier ] component-supplied-text
		//[ vendor-identifier ][ ODBC-component-identifier ][ data-source-identifier ] data-source-supplied-text
		SQLSMALLINT returnedMessageSize = 0;
		std::array<char, SQL_MAX_MESSAGE_LENGTH> wszMessage{};
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
			if (std::string_view stateID{ wszState.data(),5 }; stateID.compare(_T("01004")) != 0)
			{
				const size_t messageLen = std::min(wszMessage.size() - 1, (size_t)returnedMessageSize);
				std::string_view fullMessage{ wszMessage.data(), messageLen };
				std::cerr << std::format("[{}] {} (code: {}) at {} :Z{}\n", wszState.data(), fullMessage.data(), iError, loc.function_name(), loc.line());
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

std::vector<std::string> fetchDiagnostics(SQLRETURN RetCode, SQLSMALLINT handleType, SQLHANDLE context, const std::source_location& loc)
{
	std::vector<std::string> diagnostics;
	if (RetCode != SQL_SUCCESS)
	{
		if (RetCode == SQL_INVALID_HANDLE)
		{
			diagnostics.emplace_back(std::format("Invalid handle! {} : {}\n", loc.function_name(), loc.line()));
			return diagnostics;
		}
		if (context == SQL_NULL_HANDLE)
		{
			diagnostics.emplace_back(std::format("NULL handle! {} : {}\n", loc.function_name(), loc.line()));
			return diagnostics;
		}


		//[ vendor-identifier ][ ODBC-component-identifier ] component-supplied-text
		//[ vendor-identifier ][ ODBC-component-identifier ][ data-source-identifier ] data-source-supplied-text
		SQLSMALLINT returnedMessageSize = 0;
		std::array<char, SQL_MAX_MESSAGE_LENGTH> wszMessage{};
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
			if (std::string_view stateID{ wszState.data(),5 }; stateID.compare(_T("01004")) != 0)
			{
				const size_t messageLen = std::min(wszMessage.size() - 1, (size_t)returnedMessageSize);
				std::string_view fullMessage{ wszMessage.data(), messageLen };
				diagnostics.emplace_back(std::format("[{}] {} (code: {}) at {} :Z{}\n", wszState.data(), fullMessage.data(), iError, loc.function_name(), loc.line()));
			}
			//reset
			wszMessage.fill(0);//<-memset(&message,0,count)
			wszState.fill(0);
			returnedMessageSize = 0;
			iError = 0;
		}
	}
	return diagnostics;
}


