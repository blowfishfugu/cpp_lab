#include "GeoLocation.h"
#include "ReadToToken.h"

#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <charconv>
struct incomplete_line_error {};

//ignored_field, wird in outstream übersprungen
std::ostream& operator<<(std::ostream& output, ignored_field&){	return output; }
//ignored_field, wird in outstream übersprungen
std::ostream& operator<<(std::ostream& output, ignored_field const&){ return output; }



//String, stream wird konsumiert, direktes befüllen von target
template<char delim>
constexpr void set(readInfos& input, GeoLoc::S& target, size_t index)
{
	auto range = readTo<delim>( input );
	target = input.all.substr( range.begin, range.len );
}

//Int, stream wird konsumiert, Umwandlung string->int
template<char delim>
constexpr void set(readInfos& input, GeoLoc::I& target, size_t index)
{
	auto range = readTo<delim>( input );
	std::string_view tmp{ input.all.substr( range.begin, range.len ) };
	auto result = std::from_chars(tmp.data(), tmp.data()+tmp.size(), target);
	//target = std::stoi(std::forward<std::string>(tmp));
	return;
}

//Double, stream wird konsumiert, Umwandlung string->double
template<char delim>
constexpr void set(readInfos& input, GeoLoc::D& target, size_t index)
{
	auto range = readTo<delim>( input );
	std::string_view tmp{ input.all.substr( range.begin, range.len ) };
	auto result=std::from_chars(tmp.data(), tmp.data()+tmp.size(), target);
	//target = std::stod(std::forward<std::string_view>(tmp));
	return;
}

//ignored_field, stream wird konsumiert, desweiteren soll nichts passieren
template<char delim>
constexpr void set(readInfos& input, ignored_field& target, size_t index)
{
	readTo<delim>( input );
}

static std::string emptyString{ "(null)" };
static std::string_view vemptyString{ emptyString };
static IndexedString emptyIndexString{ emptyString };
//strings nicht editierbar, weil sie schlüssel sind.
PoolType StringPools;

//falls hinterher änderbar sein soll (z.B. umlautersetzung), doppelter Platzbedarf
//std::map<size_t, std::map<GeoLoc::S,GeoLoc::S> > StringPools;

template<char delim>
constexpr void set( readInfos& input, GeoLoc::SP& target, size_t index )
{
	auto range = readTo<delim>( input );
	if( range.len == 0 ) { 
		target = &emptyString;
		return; 
	}
	std::string tmp{ input.all.substr( range.begin, range.len ) };

	auto& indexPool = StringPools[ index ];
	auto f = indexPool.find( tmp );
	if( f == indexPool.end() )
	{
		auto inserted = indexPool.emplace( tmp,0LL );
		f = inserted.first;
	}

	GeoLoc::SP ptr = ( &(f->first) );
	target = ptr;
	return;
}

template<char delim>
constexpr void set(readInfos& input, GeoLoc::ISP& target, size_t index)
{
	auto range = readTo<delim>(input);
	if (range.len == 0) {
		target = &emptyIndexString;
		return;
	}
	IndexedString tmp{ input.all.substr(range.begin, range.len) };

	auto& indexPool = StringPools[index];
	auto f = indexPool.find(tmp);
	if (f == indexPool.end())
	{
		auto inserted = indexPool.emplace(tmp, 0LL);
		f = inserted.first;
	}

	GeoLoc::ISP ptr = (&(f->first));
	target = ptr;
	return;
}


std::ostream& operator<<(std::ostream& output, GeoLoc::SP& strPtr) 
{ 
	if (strPtr)
	{
		output << strPtr->data();
	}
	else { output << ""; }
	return output; 
}

std::ostream& operator<<(std::ostream& output, GeoLoc::SP const& strPtr) 
{
	if (strPtr)
	{
		output << strPtr->data();
	}
	else { output << ""; }
	return output; 
}

std::ostream& operator<<(std::ostream& output, GeoLoc::ISP& strPtr)
{
	if (strPtr)
	{
		output << strPtr->str.data();
	}
	else { output << ""; }
	return output;
}

std::ostream& operator<<(std::ostream& output, GeoLoc::ISP const& strPtr)
{
	if (strPtr)
	{
		output << strPtr->str.data();
	}
	else { output << ""; }
	return output;
}



template<typename FieldType>
void assignEmpty(FieldType& item)
{
	return;
}

template<>
void assignEmpty(GeoLoc::SP& strPtr)
{
	strPtr = &emptyString;
}

GeoLoc::GeoLoc(std::string_view const& csv) noexcept
{
	this->setLine(csv);
}

void GeoLoc::setLine(std::string_view const& csv) noexcept
{
	try
	{
		if (csv.length() == 0)
		{
			throw incomplete_line_error{};
		}
		readInfos input{ csv, 0 };
		size_t index = 0;
		auto setItem = [&index](readInfos& input, auto& item) {
			set<';'>(input, item, index);
			++index;
		};

		std::apply([&setItem, &input](auto&&... items) {
			(setItem(input, items), ...);
			//(set<';'>(input, items), ...); //items entpacken
			},
			raw_data);
	}
	catch (incomplete_line_error&)
	{
		std::cerr << "incomplete line!: " << csv.data() << "\n";

		//alternative: line_error weiterwerfen, so dass leere elemente nicht in data landen?

		//Vorzeitiger Abbruch, typ SP vorbelegen!
		std::apply([](auto&&... items) {
			(assignEmpty(items), ...);
			}, raw_data);
	}
}



void GeoLoc::print(std::ostream& output) const noexcept
{
	size_t index = 0;
	//operator << S
	//operator << SP
	//operator << ISP
	auto perItem = [&index,&output](const auto& item) {
		if (++index > 1) { output << "\t"; }
		output << item;
	};

	std::apply([&perItem](auto&&... items) {
		(perItem(items), ...);
		}, raw_data);
	output << "\n";
}

bool oldLesserOperator(GeoLoc const& lhs, GeoLoc const& rhs)
{
	//Stadt-Bezirk-Stadtteil-Plz-Straße-Hausnummer
	using SortType = std::tuple<__int64, __int64, __int64, __int64, __int64, __int64>;

	static auto ToSortType = [](GeoLoc const& geo, GeoLoc const& ref)->SortType
	{
		return { 
			geo.City()==ref.City()?0:StringPools[GeoLoc::CITY].find(*geo.City())->second,
			geo.District()==ref.District()?0:StringPools[GeoLoc::DISTRICT].find(*geo.District())->second,
			geo.UrbanUnit()==ref.UrbanUnit()?0:StringPools[GeoLoc::URBANUNIT].find(*geo.UrbanUnit())->second,
			geo.Zip()==ref.Zip()?0:StringPools[GeoLoc::ZIP].find(*geo.Zip())->second,
			geo.Street()==ref.Street()?0:StringPools[GeoLoc::STREET].find(*geo.Street())->second,
			geo.House()==ref.House()?0:StringPools[GeoLoc::HOUSE].find(*geo.House())->second,
		};
	};

	SortType left = ToSortType(lhs,rhs);
	SortType right = ToSortType(rhs,lhs);
	return left < right;
}

bool operator<(GeoLoc const& lhs, GeoLoc const& rhs)
{
	//Stadt-Bezirk-Stadtteil-Plz-Straße-Hausnummer
	using SortType = std::tuple<__int64, __int64, __int64, __int64, __int64, __int64>;

	static auto ToSortType = [](GeoLoc const& geo)->SortType
	{
		return {
			geo.City()->order,
			geo.District()->order,
			geo.UrbanUnit()->order,
			geo.Zip()->order,
			geo.Street()->order,
			geo.House()->order,
		};
	};

	SortType left = ToSortType(lhs);
	SortType right = ToSortType(rhs);
	return left < right;
}
