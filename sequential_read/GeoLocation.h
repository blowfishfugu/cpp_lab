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
	using SP = const std::string*;
private:
	//tuple hat offene Pointer-> private machen.
	//Berlin;Aachener Straﬂe;1;10713;Charlottenburg-Wilmersdorf;Wilmersdorf;Wilmersdorf;52.482187140;13.318354210
	std::tuple<SP, SP, SP, I, SP, SP, SP, D, D> raw_data{};

public:
	S const& City() noexcept { return *std::get<0>(raw_data); }
	S const& Street() noexcept { return *std::get<1>(raw_data); }
	S const& House() noexcept { return *std::get<2>(raw_data); }
	I& Zip() noexcept { return std::get<3>(raw_data); }
	S const& UrbanName() noexcept { return *std::get<4>(raw_data); }
	S const& OldName() noexcept { return *std::get<5>(raw_data); }
	S const& District() noexcept { return *std::get<6>(raw_data); }
	D& Latitude() noexcept { return std::get<7>(raw_data); }
	D& Longitude() noexcept { return std::get<8>(raw_data); }

	GeoLoc(std::string_view csv) noexcept;
	void print() const noexcept;
};

constexpr const size_t szGeoLoc = sizeof(GeoLoc);