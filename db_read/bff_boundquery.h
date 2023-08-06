
#include <sql.h>
#include <array>
#include <tuple>
#include <string>
#include <string_view>
#include <vector>



inline long conv( const long& src, size_t currentIndex )
{
	return src;
}

template<size_t sz>
inline std::string conv( CharBuffer<sz>& src, size_t currentIndex )
{
	return static_cast<std::string>(src); //<-explicit overload
}



template<class TSrc, class TDest, size_t...Is>
void tuple_transform( TSrc& input, TDest& output,
					  std::index_sequence<Is...> )
{
	//Is entpacken
	using expander = int[];
	(void)expander
	{
		0, ( std::get<Is>(output)=conv( std::get<Is>( input ), Is ), 0 )...
	};
}

template<typename InTuple,typename OutTuple>
constexpr void extractBuffer( InTuple& input, OutTuple& output )
{
	constexpr size_t inSize = std::tuple_size<std::decay_t<InTuple>>::value;
	constexpr size_t outSize = std::tuple_size<std::decay_t<OutTuple>>::value;
	static_assert( inSize == outSize );
	tuple_transform < InTuple, OutTuple >( input, output, std::make_index_sequence<inSize>() );
}



template<typename... ArgTypes>
const char* printem(const char* fmt, ArgTypes... args)
{
	static char buf[256];
	snprintf(buf, sizeof(buf), fmt, args...);
	return buf;
}

//pro ItemTyp: bindColumn,cleanColumn, conv()-überladung -> muss existieren
template<typename... ColumnTypes>
struct BoundQuery
{
	std::tuple<ColumnTypes...> Bindings;
	using OutputType = std::tuple<ColumnTypes...>;
	HDBC _hdbc;
	HSTMT _stmt;
	std::string queryString;
	BoundQuery() = delete;
	BoundQuery( HDBC dbc, const std::string_view sql )
		: _hdbc( dbc ), _stmt( SQL_NULL_HANDLE ), queryString( sql )
	{}

	//StringField
	template<size_t BufCap=MAXFIELDLEN>
	void bindColumn( CharBuffer<BufCap>& buffer, int n )
	{
		SQLLEN len = buffer.capacity;
		SQLBindCol( _stmt, n, SQL_C_TCHAR, buffer.data.data(), buffer.capacity, &len );
	}


	template<size_t BufCap = MAXFIELDLEN>
	void cleanColumn( CharBuffer<BufCap>& buffer ) noexcept
	{
		buffer.clear();
	}

	//long
	void bindColumn( SQLINTEGER& buffer, int n )
	{
		SQLLEN len = 0;
		SQLBindCol( _stmt, n, SQL_C_LONG, &buffer , sizeof(SQLINTEGER), &len);
	}

	void cleanColumn( SQLINTEGER& buffer ) noexcept
	{
		buffer = 0;
	}

	void Prepare()
	{
		SQLAllocStmt( _hdbc, &_stmt );
		SQLPrepare( _stmt, (SQLTCHAR*)queryString.data(), (SQLINTEGER)queryString.length() );
		SQLExecute( _stmt );
		
		
		int n = 1;
		std::apply( [ this, &n ]( auto& ...item ) { 
			( ..., bindColumn( item, n++ ) ); 
					}, Bindings );

	};

	template<typename OutTuple>
	bool Fetch(OutTuple& output)
	{
		//clean oder rebind?
		std::apply( [ this ]( auto& ...item ) { 
			( ..., cleanColumn( item ) ); 
					}, Bindings );

		SQLRETURN res = SQLFetch( _stmt );
		if( !SQL_SUCCEEDED( res ) ) { return false; }
		if( res == SQL_NO_DATA_FOUND ) { return false; }
		extractBuffer( Bindings, output );
		return true;
	}

	template<typename OutTuple>
	std::vector<OutTuple> FetchAll()
	{
		std::vector<OutTuple> rows;
		OutTuple row{};
		while( Fetch<OutTuple>( row ) )
		{
			rows.emplace_back( row );
		}
		this->Close();
		return rows;
	}

	void Close()
	{
		//std::apply( [ this ]( auto& ...item ) { ( ..., unbindColumn( item ) ); }, Bindings );
		if( _stmt )
		{
			SQLCloseCursor( _stmt );
			SQLFreeHandle( SQL_HANDLE_STMT, _stmt );
		}
	}
};
