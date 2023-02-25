#pragma once
#include <tuple>
#include <string>

struct ignored_field{};



struct GeoLoc
{
	using S = std::string;
	using I = unsigned int;
	using D = double;
	using X = ignored_field;
	//Berlin;Aachener Straße;1;10713;Charlottenburg-Wilmersdorf;Wilmersdorf;Wilmersdorf;52.482187140;13.318354210
	std::tuple<X, S, S, I, S, S, S, D, D> raw_data{};

	X& City() { return std::get<0>(raw_data); }
	S& Street() { return std::get<1>(raw_data); }
	S& House() { return std::get<2>(raw_data); }
	I& Zip() { return std::get<3>(raw_data); }
	S& UrbanName() { return std::get<4>(raw_data); }
	S& OldName() { return std::get<5>(raw_data); }
	S& District() { return std::get<6>(raw_data); }
	D& Latitude() { return std::get<7>(raw_data); }
	D& Longitude() { return std::get<8>(raw_data); }

	GeoLoc(std::string_view csv);
	void print() const;
};