// sequential_read.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "StopWatch.h"
#include "Loaders.h"
#include <iostream>
#include <vector>
#include <map>
#include <set>

extern std::map<size_t, std::set<std::string_view> > StringPools;

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
	std::ios_base::sync_with_stdio(false); //optimierung.

										   //doubles
	std::cout.setf(std::cout.fixed);
	std::cout.precision(8);

	runLoader( load<read_none>, " read none " );
	runLoader( load<read_standard>, " read standard warmup.." );
	runLoader( load<read_standard>, " read standard " );
	//runLoader( load<read_mfc>, " read mfc " ); // include afx.h, CStdioStream.ReadLine(CString)
	runLoader( load<read_buffered>, " read buffered " );
	runLoader( load<read_stringbuffered>, " read with countlines into string as buffer, no getline" );
	runLoader( load<read_stringbuffered2>, "string_view-iterator, fs, no getline" );
	return 0;	

	StopWatch stopWatch(std::cout);
	stopWatch.checkpoint("init done ");
	data_type data;
	__int64 linecount=load<read_stringbuffered2>(data);
	std::cout << linecount << " items\n";
	stopWatch.checkpoint("load done ");
	
	size_t offset = data.size() / 30; //30 items gleichverteilt über alles ausgeben
	if (offset == 0) { offset = 1LL; } // 1/30=0 abfangen
	
	for (size_t i = 0; i < data.size(); i += offset)
	{
		data[i].print();
	}
	stopWatch.checkpoint("print done ");

	std::cout << "Indexcount " << StringPools.size() << "\n";
	for (const auto& [index,pool] : StringPools)
	{
		std::cout << index << "\t Uniques: " << pool.size() << "\n";
	}
	stopWatch.checkpoint("loop indizes done");

	size_t objectsSize = sizeof(GeoLoc)*data.size();
	size_t poolSize = 0;
	for (const auto&[index, pool] : StringPools)
	{
		poolSize += sizeof(index);
		for (const auto& str : pool)
		{
			poolSize += sizeof(std::string::value_type)*str.size();
		}
	}
	std::cout << "ObjectSize:\t" << objectsSize << "\n";
	std::cout << "PoolSize:\t" << poolSize << "\n";


	constexpr std::tuple<double, double> fernsehturm{ 52.520803, 13.40945 };

}
