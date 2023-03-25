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
	data_type data;
	__int64 linecount=load<read_stringbuffered2>(data);
	std::cout << linecount << " items\n";
	stopWatch.checkpoint("load done ");

	std::cout << "Indexcount " << StringPools.size() << "\n";
	size_t uniqueStrings = 0;
	for (const auto& [index,pool] : StringPools)
	{
		std::cout << index << "\t Uniques: " << pool.size() << "\n";
		uniqueStrings += pool.size();
	}
	std::cout << "Unique string count:\t" << uniqueStrings << "\n";
	stopWatch.checkpoint("print uniques done");

	size_t objectsSize = sizeof(GeoLoc)*data.size();
	size_t poolSize = 0;
	for ( auto&[index, pool] : StringPools)
	{
		poolSize += sizeof(index);
		std::vector<std::pair<GeoLoc::S,GeoLoc::S> > strings;
		strings.reserve(pool.size());
		for (const auto& str : pool)
		{
			poolSize += sizeof(std::string::value_type)*str.first.size();
			strings.emplace_back(str.first,str.first);
		}

		if (index != GeoLoc::HOUSE)
		{
			std::for_each(std::execution::par, strings.begin(), strings.end(),
				[](std::pair<std::string,std::string>& normStr) mutable {
					//TODO: ->inkl umlautregeln ä->ae , ß->ss, è zu e...
					normStr.second.reserve(normStr.first.length() * 2);
				}
			);
		}
		else
		{
			std::for_each(std::execution::par, strings.begin(), strings.end(),
				[](std::pair<std::string,std::string>& normStr) mutable {
					//TODO: ->inkl umlautregeln ä->ae , ß->ss, è zu e...
					normStr.second.reserve(normStr.first.length() * 2);
				}
			);
		}

		std::sort(
			strings.begin(), strings.end(), [](auto const& lhs, auto const& rhs) {
			return lhs.second < rhs.second;
			});

		for (size_t order = 0; order < strings.size(); ++order)
		{
			pool[strings[order].first] = (order+1);
		}
	}
	stopWatch.checkpoint("calculate poolorders done");

	std::cout << "ObjectSize:\t" << objectsSize << "\n";
	std::cout << "PoolSize:\t" << poolSize << "\n";


	constexpr std::tuple<double, double> fernsehturm{ 52.5208182, 13.4072251 };


	//latitude/longitude umrechnen zu Abstand+Winkel zu "fernsehturm"

	//Sortieren
	//Stadt-Bezirk-Stadtteil-Plz-Straße-Hausnummer
	std::sort(std::execution::par, data.begin(), data.end());
	stopWatch.checkpoint("sorting datagrid done");

	size_t offset = data.size() / 30; //30 items gleichverteilt über alles ausgeben
	if (offset == 0) { offset = 1LL; } // 1/30=0 abfangen

	for (size_t i = 0; i < data.size(); i += offset)
	{
		for (size_t region = i; region < (i + 5); ++region)
		{
			data[region].print();
		}
		std::cout << "\n";
	}
	stopWatch.checkpoint("print done ");
}
