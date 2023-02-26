#include "GeoLocation.h"
#include <sstream>
#include <iostream>
#include <map>
#include <set>
struct incomplete_line_error {};

//ignored_field, wird in outstream übersprungen
std::ostream& operator<<(std::ostream& output, ignored_field&){	return output; }
//ignored_field, wird in outstream übersprungen
std::ostream& operator<<(std::ostream& output, ignored_field const&){ return output; }

//String, stream wird konsumiert, direktes befüllen von target
template<char delim>
constexpr void set(std::istringstream& input, GeoLoc::S& target, size_t index)
{
	if (std::getline(input, target, delim))
	{
		return;
	}
	throw incomplete_line_error{};
}

//Int, stream wird konsumiert, Umwandlung string->int
template<char delim>
constexpr void set(std::istringstream& input, GeoLoc::I& target, size_t index)
{
	if (std::string tmp; std::getline(input, tmp, delim))
	{
		target = std::stoi(tmp);
		return;
	}
	throw incomplete_line_error{};
}

//Double, stream wird konsumiert, Umwandlung string->double
template<char delim>
constexpr void set(std::istringstream& input, GeoLoc::D& target, size_t index)
{
	if (std::string tmp; std::getline(input, tmp, delim))
	{
		target = std::stod(tmp);
		return;
	}
	throw incomplete_line_error{};
}

//ignored_field, stream wird konsumiert, desweiteren soll nichts passieren
template<char delim>
constexpr void set(std::istringstream& input, ignored_field& target, size_t index)
{
	if (std::string tmp; std::getline(input, tmp, delim))
	{
		return;
	}
	throw incomplete_line_error{};
}

//strings nicht editierbar, weil sie schlüssel sind.
std::map<size_t, std::set<GeoLoc::S> > StringPools;

//falls hinterher änderbar sein soll (z.B. umlautersetzung), doppelter Platzbedarf
//std::map<size_t, std::map<GeoLoc::S,GeoLoc::S> > StringPools;

template<char delim>
constexpr void set(std::istringstream& input, GeoLoc::SP& target, size_t index)
{
	if (std::string tmp; std::getline(input, tmp, delim))
	{
		std::set<GeoLoc::S>& indexPool = StringPools[index];
		auto f = indexPool.find(tmp);
		if (f == indexPool.end())
		{
			auto inserted=indexPool.insert(tmp);
			f=inserted.first;
		}
		
		//nicht schön -> tuple private gemacht, Accessoren müssen const& werden
		GeoLoc::SP ptr = ( &(*f) );
		target = ptr;
		return;
	}
	throw incomplete_line_error{};
}


std::ostream& operator<<(std::ostream& output, GeoLoc::SP& strPtr) 
{ 
	if (strPtr)
	{
		output << strPtr->c_str();
	}
	else { output << ""; }
	return output; 
}

std::ostream& operator<<(std::ostream& output, GeoLoc::SP const& strPtr) 
{
	if (strPtr)
	{
		output << strPtr->c_str();
	}
	else { output << ""; }
	return output; 
}

static std::string emptyString{ "(null)" };

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

GeoLoc::GeoLoc(std::string_view csv)
{
	try
	{
		if (csv.length() == 0)
		{
			throw incomplete_line_error{};
		}
		std::istringstream input(csv.data());
		size_t index = 0;
		auto setItem = [&index](std::istringstream& input, auto& item) {
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
		std::apply([](auto&&... items){
			(assignEmpty(items), ...);
			}, raw_data );
	}
}

void GeoLoc::print() const
{
	size_t index = 0;
	auto perItem = [&index](const auto& item) {
		if (++index > 1) { std::cout << "\t"; }
		std::cout << item;
	};

	std::apply([&perItem](auto&&... items) {
		(perItem(items), ...);
		}, raw_data);
	std::cout << "\n";
}
