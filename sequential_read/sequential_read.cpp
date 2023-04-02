// sequential_read.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "StopWatch.h"
#include "Loaders.h"
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <fcntl.h>
#include <io.h>
#include <algorithm>
#include <execution>
#include <charconv>
#include <functional> //std::function

extern PoolType StringPools;

template<typename LoaderFunc>
void runLoader( LoaderFunc func, std::string_view info )
{
	std::cout << "\n start " << info << "\n";
	StopWatch watch( std::cout );
	
	data_type data;
	__int64 lineCount=func( data );
	std::cout << lineCount << " items\n";
	watch.checkpoint( info );
	StringPools.clear();
}


static inline void normalize(std::string& str)
{
	//for (const unsigned char c : str)
	//{
	//	if (c >= 'a' && c <= 'z') { continue; }
	//	if (c >= 'A'&& c <= 'Z') { continue; }
	//	if (c >= '0' && c <= '9') { continue; }
	//	if (c < 0xC4) { continue; } //'Ä'
	//	foundChars[c]++;
	//}

	/*
	ß : 0xDF : 4443
	ü : 0xFC : 477
	ö : 0xF6 : 403
	ä : 0xE4 : 259
	é : 0xE9 : 25
	è : 0xE8 : 6
	Ä : 0xC4 : 3
	Ö : 0xD6 : 3
	Ü : 0xDC : 3
	á : 0xE1 : 3
	ë : 0xEB : 1
	*/

	//gnadenlos entnommen aus adecc-repository, 
	//reihenfolge anhand vorab-histogramm, straße+äöü sind im deutschen adressraum (nicht nur berlin) in überhand
	using namespace std::literals;
	static const std::string special = "ßüöäéèÄÖÜáëà";
	static const std::unordered_map<char, std::string> new_values = {
		{'ß',"ss"s},
		{'ü',"ue"s},
		{'ö',"oe"s},
		{'ä',"ae"s},
		{'é',"e"s},
		{'è',"e"s},
		{'Ä',"Ae"s},
		{'Ö',"Oe"s},
		{'Ü',"Ue"s},
		{'á',"a"s},
		{'ë',"e"s},
		{'à',"a"s},
	};
	static constexpr auto norm = [](std::string& str, size_t pos) noexcept {
		do {
			str.replace(pos, 1, new_values.find(str[pos])->second);
		} while ((pos = str.find_first_of(special, pos)) != std::string::npos);
	};

	size_t foundpos = str.find_first_of(special);
	if (foundpos == std::string::npos) { return; }
	norm(str, foundpos);
}

static inline bool houseCompare(std::string& lhs, std::string& rhs)
{
	//10A -> {10,'a'}
	static constexpr auto split = [](std::string& str)->std::tuple<__int64,std::string>
	{
		using namespace std::literals;
		__int64 num = 0;
		std::from_chars_result res=std::from_chars(str.data(), str.data() + str.size(), num);
		if (*res.ptr != 0) {
			const char* tst = res.ptr;
			while (*tst != 0) { 
				if (*tst >= 'A' && *tst <= 'Z')
				{
					(*const_cast<char*>(tst)) += ('a' - 'A');
				}
				++tst; 
			}
			return { num,res.ptr };
		}
		return { num, " "s };
	};
	auto left = split(lhs);
	auto right = split(rhs);
	return left < right;
}

static inline void inspectIndex()
{
	std::cout << "Indexcount " << StringPools.size() << "\n";
	size_t uniqueStrings = 0;
	for (const auto&[index, pool] : StringPools)
	{
		std::cout << index << "\t Uniques: " << pool.size() << "\n";
		uniqueStrings += pool.size();
	}
	std::cout << "Unique string count:\t" << uniqueStrings << "\n";
}

static inline void organizeIndex()
{
	for (auto&[index, pool] : StringPools)
	{
			std::vector<std::pair<GeoLoc::S, GeoLoc::S> > strings;
			strings.reserve(pool.size());
			for (const auto& poolItem : pool)
			{
				strings.emplace_back(poolItem.first.str, poolItem.first.str);
			}

			//normalized sort
			if (index != GeoLoc::HOUSE)
			{
				std::for_each(
					std::execution::par,
					strings.begin(), strings.end(),
					[](std::pair<std::string, std::string>& normStr) mutable {
						//TODO: ->inkl umlautregeln ä->ae , ß->ss, è zu e...
						normalize(normStr.second);
					}
				);

				std::sort(
					std::execution::par,
					strings.begin(), strings.end(), 
					[](auto const& lhs, auto const& rhs) {
						return lhs.second < rhs.second;
					});

			}
			//splitted sort on <number, optional character>
			else
			{
				std::sort(
					std::execution::par,
					strings.begin(), strings.end(),
					[](auto& lhs, auto& rhs) {
						return houseCompare(lhs.second, rhs.second);
					});

			}

			for (size_t order = 0; order < strings.size(); ++order)
			{
				if (auto it = pool.find(strings[order].first); it != pool.end())
				{
					it->second = (order + 1);
					it->first.order = (order + 1);
				}
			}
	}
}

std::ostream& operator<<(std::ostream& output, GeoLoc const& loc)
{
	loc.print(output);
	return output;
}
std::ostream& operator<<(std::ostream& output, GeoLoc* loc)
{
	loc->print(output);
	return output;
}

static auto printSamples = [](auto const& data, const size_t sampleCount = 20, const size_t itemsPerSample = 8)
{
	if (data.size() == 0) { return; }

	size_t offset = data.size() / sampleCount; //20 items gleichverteilt über alles ausgeben
	if (offset == 0) { offset = 1LL; } // 1/20=0 abfangen
	
	std::cout << "first:\n"
		<< data[0] <<"\n";

	for (size_t i = 0; i < data.size()-itemsPerSample; i += offset)
	{
		const size_t uboundSample = i + itemsPerSample;
		for (size_t region = i; region < uboundSample; ++region)
		{
			std::cout << data[region];
		}
		std::cout << "\n";
	}

	std::cout << "last:\n" 
		<< data[data.size()-1] <<"\n";
	
	std::cout << "---\n";
};

static std::map<unsigned char, __int64> foundChars;
static void histOfChars()
{
	FILE* fp = nullptr;
	fopen_s(&fp, "C:\\Temp\\histogram.txt", "w");
	if (fp)
	{
		for ( const auto& it : foundChars)
		{
			fprintf_s( fp, "%c : 0x%.2X : %I64d\n", it.first, it.first, it.second );
		}
	}
	fp && fclose(fp);
}

int main(void)
{
	//std::ios_base::sync_with_stdio(false); //optimierung.
										   //doubles
	std::cout.setf(std::cout.fixed);
	std::cout.precision(8);

	/*
	runLoader( load<read_none>, " read none " );
	runLoader( load<read_standard>, " read standard warmup.." );
	runLoader( load<read_standard>, " read standard " );
	//runLoader( load<read_mfc>, " read mfc " ); // include afx.h, CStdioStream.ReadLine(CString)
	runLoader( load<read_buffered>, " read buffered " );
	runLoader( load<read_stringbuffered>, " read with countlines into string as buffer, no getline" );
	runLoader( load<read_stringbuffered2>, "string_view-iterator, fs, no getline" );
	return 0;	
	*/

	StopWatch stopWatch(std::cout);
	stopWatch.checkpoint("init done ");

	static data_type data;
	__int64 linecount=load<read_stringbuffered2>(data);
	std::cout << linecount << " items\n";
	stopWatch.checkpoint("load done ");

	inspectIndex();
	stopWatch.checkpoint("print uniques done");

	organizeIndex();
	stopWatch.checkpoint("calculate poolorders done");
	
	//histOfChars();
	constexpr const size_t viewCount = 5LL;
	static std::array<view_type,viewCount> views;
	std::for_each( std::execution::par,
		views.begin(), views.end(), 
		[](auto& view){
			view.reserve(data.size());
			for ( GeoLoc& item : data)
			{ view.push_back(&item); }
		}
	);
	stopWatch.checkpoint("copy pointers to views");

	constexpr std::tuple<double, double> fernsehturm{ 52.5208182, 13.4072251 };


	//latitude/longitude umrechnen zu Abstand+Winkel zu "fernsehturm"

	//Sortieren
	static std::array<std::function<bool(const GeoLoc*, const GeoLoc*)>, 5> sortFuncs =
	{
		//Stadt-Bezirk-Stadtteil-Plz-Straße-Hausnummer
		[](const GeoLoc* l, const GeoLoc* r) {return *l < *r; },
		[](const GeoLoc* l, const GeoLoc* r) {return *r < *l; },

		[](const GeoLoc* l, const GeoLoc* r) {return (*l).Zip()->order < (*r).Zip()->order; },
		[](const GeoLoc* l, const GeoLoc* r) {return (*l).Latitude() < (*r).Latitude(); },
		[](const GeoLoc* l, const GeoLoc* r) {return (*l).Longitude() < (*r).Longitude(); },
	};

	std::vector<size_t> range(views.size());
	std::iota(range.begin(), range.end(), 0);
	std::for_each_n(std::execution::par, range.cbegin(), range.size(), 
		[](size_t N) {
		std::sort(std::execution::par,
			views[N].begin(), views[N].end(), 
			sortFuncs[ (N%sortFuncs.size()) ]
			);
		}
	);
	stopWatch.checkpoint("sorting datagrid done");

	for (size_t v = 0; v < views.size(); ++v)
	{
		printSamples(views[v], 3, 5);
	}
	stopWatch.checkpoint("print done ");
}
