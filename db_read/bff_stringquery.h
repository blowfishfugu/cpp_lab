#pragma once
#include "bff_diagnostics.h"
#include "bff_charbuffer.h"

struct Query
{
	HSTMT const& stmt;
	std::string sql;
	Query() = delete;
	Query(HSTMT const& statement, std::string_view _sql = "") : stmt(statement), sql(_sql) {};
	void SetSql(std::string_view sqlStatement)
	{
		CloseIfNecessary();
		this->sql = sqlStatement;
	}
	bool didExec = false;

	using ResultBuffer = std::vector<DefaultRecord>;
	using DescriptorBuffer = std::vector<DefaultDescriptor>;
	ResultBuffer buffer;
	DescriptorBuffer descriptions;

	//returns ResultCols,RowCount
	std::tuple<SQLSMALLINT, SQLLEN> Execute()
	{
		buffer.clear();
		descriptions.clear();
		SQLRETURN execResult = SQLExecDirect(stmt, (SQLCHAR*)sql.data(), (SQLINTEGER)sql.length());
		didExec = true;
		if (!SQL_SUCCEEDED(execResult))
		{
			printDiagnostics(execResult, SQL_HANDLE_STMT, stmt, std::source_location::current());
			CloseIfNecessary();
			return { 0,0 };
		}


		SQLSMALLINT sColCount = 0;
		SQLRETURN rc = SQLNumResultCols(stmt, &sColCount);
		printDiagnostics(rc, SQL_HANDLE_STMT, stmt, std::source_location::current());

		SQLLEN cRowCount = 0 ;
		rc = SQLRowCount(stmt, &cRowCount);
		printDiagnostics(rc, SQL_HANDLE_STMT, stmt, std::source_location::current());
		bindBuffer(sColCount);

		return { sColCount,cRowCount };
	}

	void bindBuffer(SQLSMALLINT colCount)
	{
		if (!colCount) { return; }
		buffer.resize(colCount);
		descriptions.resize(colCount);

		std::cout << std::format("{:>5} {:>5} {:>5} {:<64}\n", "col", "size", "type", "name");
		SQLUSMALLINT col = 0;
		for (auto& item : buffer)
		{
			DefaultDescriptor& desc = descriptions[col];
			SQLSMALLINT bufLength = (SQLSMALLINT)(sizeof(DefaultDescriptor::value_type) * desc.capacity);
			
			++col;
			SQLLEN readLen = item.data.size();
			
			SQLColAttribute(stmt, col, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &desc.colSize);
			SQLColAttribute(stmt, col, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &desc.colType);
			SQLColAttribute(stmt, col, SQL_DESC_NAME, (SQLPOINTER)desc.data.data(), bufLength, NULL, NULL);
			std::cout << std::format("{:>5} {:>5} {:>5} {:<64}\n", col, desc.colSize, desc.colType, static_cast<std::string>(desc));
			SQLBindCol(stmt, col, SQL_C_TCHAR, (SQLPOINTER)item.data.data(), item.capacity, &readLen);
		}
	}

	bool Fetch()
	{
		for (auto& item : buffer) { item.clear(); }
		SQLRETURN res = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(res)) { return false; }
		if (res == SQL_NO_DATA_FOUND) { return false; }
		return true;
	}

	void CloseIfNecessary()
	{
		buffer.clear();
		descriptions.clear();

		if (didExec && stmt)
		{
			SQLRETURN rc = SQLCloseCursor(stmt);
			printDiagnostics(rc, SQL_HANDLE_STMT, stmt, std::source_location::current());
		}
		didExec = false;
	}
	~Query() {
		CloseIfNecessary();
	}
};