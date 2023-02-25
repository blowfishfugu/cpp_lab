#include "GeoLocation.h"
#include <sstream>
#include <iostream>

struct incomplete_line_error {};

template<char delim>
constexpr void set(std::istringstream& input, GeoLoc::S& target)
{

	if (std::getline(input, target, delim))
	{
		return;
	}
	throw incomplete_line_error{};
}

template<char delim>
constexpr void set(std::istringstream& input, GeoLoc::I& target)
{
	std::string tmp;

	if (std::getline(input, tmp, delim))
	{
		target = std::stoi(tmp);
		return;
	}
	throw incomplete_line_error{};
}

template<char delim>
constexpr void set(std::istringstream& input, GeoLoc::D& target)
{
	std::string tmp;

	if (std::getline(input, tmp, delim))
	{
		target = std::stod(tmp);
		return;
	}
	throw incomplete_line_error{};
}

GeoLoc::GeoLoc(std::string_view csv)
{
	std::istringstream input(csv.data());
	try 
	{
		std::apply([&input](auto&&... items) {
			(set<';'>(input, items), ...); //items entpacken
			},
			raw_data);
	}
	catch (incomplete_line_error&)
	{
		std::cerr << "incomplete line!: " << csv.data() << "\n";
	}
}
