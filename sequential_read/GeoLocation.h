#pragma once
#include <tuple>
#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <utility>

struct ignored_field{};

struct IndexedString {
	std::string str{};
	mutable __int64 order = 0;
	IndexedString(std::string const& val) : str(val) {};
	IndexedString(std::string_view const& val) : str(val) {};
};

inline bool operator==(IndexedString const& l, IndexedString const& r){
	return l.str == r.str;
}

inline bool operator!=(IndexedString const& l, IndexedString const& r){
	return !(l == r);
}

template<>
struct std::hash<IndexedString>{	
	[[nodiscard]] size_t operator()(const IndexedString& _Keyval) const noexcept{
		return std::hash<std::string>()(_Keyval.str);	
	}
};
using PoolType = std::map<size_t, std::unordered_map<IndexedString, __int64> >;

struct GeoLoc
{
	using S = std::string;
	using I = unsigned int;
	using D = double;
	using X = ignored_field;
	using SP = const std::string*;
	using LL = __int64;
	using ISP = const IndexedString*;
private:
	//tuple hat offene Pointer-> private machen.
	//Berlin;Aachener Straﬂe;1;10713;Charlottenburg-Wilmersdorf;Wilmersdorf;Wilmersdorf;52.482187140;13.318354210
	std::tuple<ISP, ISP, ISP, ISP, ISP, ISP, ISP, D, D> raw_data{};
	
	//map-less: w‰re 200ms schneller zu lesen
	//std::tuple<S, S, S, S, S, S, S, D, D> raw_data{};

public:
	enum FieldNames : unsigned int{
		CITY =0,
		STREET,
		HOUSE,
		ZIP,
		URBANUNIT,
		OLDNAME,
		DISTRICT,
		LATITUDE,
		LONGITUDE
	};
	ISP const City() const noexcept { return std::get<0>(raw_data); }
	ISP const Street() const noexcept { return std::get<1>(raw_data); }
	ISP const House() const noexcept { return std::get<2>(raw_data); }
	ISP const Zip() const noexcept { return std::get<3>(raw_data); }
	ISP const UrbanUnit() const noexcept { return std::get<4>(raw_data); }
	ISP const OldUnit() const noexcept { return std::get<5>(raw_data); }
	ISP const District() const noexcept { return std::get<6>(raw_data); }
	D const& Latitude() const noexcept { return std::get<7>(raw_data); }
	D const& Longitude() const noexcept { return std::get<8>(raw_data); }

	GeoLoc() noexcept = default;
	GeoLoc& operator = (GeoLoc const&) = default;
	GeoLoc(GeoLoc const&) = default;
	
	GeoLoc(GeoLoc&& ref) noexcept {
		swap(*this, ref);
	}

	GeoLoc(std::string_view const& csv) noexcept;
	void setLine(std::string_view const& csv) noexcept;
	void print(std::ostream& output) const noexcept;

	friend bool operator<(GeoLoc const& l, GeoLoc const& r);
	

	//TODO: ruleof 5..3. whatever
	//init,copy,swap,assign.. -Operationen
	friend void swap(GeoLoc& lhs, GeoLoc& rhs)
	{
		std::swap(lhs.raw_data, rhs.raw_data);
	}
};

constexpr const size_t szGeoLoc = sizeof(GeoLoc);