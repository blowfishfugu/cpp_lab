// sequential_read.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "StopWatch.h"
#include "Loaders.h"
#include <iostream>
#include <vector>
#include <map>
#include <set>


int main(void)
{
	std::ios_base::sync_with_stdio(false); //optimierung.

										   //doubles
	std::cout.setf(std::cout.fixed);
	std::cout.precision(9);

	StopWatch stopWatch(std::cout);
	stopWatch.checkpoint("init done ");
	data_type data;
	__int64 linecount=load<read_buffered>(data);
	std::cout << linecount << " items\n";
	stopWatch.checkpoint("load done ");
	
	size_t offset = data.size() / 30; //30 items gleichverteilt über alles ausgeben
	if (offset == 0) { offset = 1LL; } // 1/30=0 abfangen
	
	for (size_t i = 0; i < data.size(); i += offset)
	{
		data[i].print();
	}
	stopWatch.checkpoint("print done ");

	extern std::map<size_t, std::set<std::string> > StringPools;
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
		for (const std::string& str : pool)
		{
			poolSize += sizeof(std::string::value_type)*str.size();
		}
	}
	std::cout << "ObjectSize:\t" << objectsSize << "\n";
	std::cout << "PoolSize:\t" << poolSize << "\n";


	constexpr std::tuple<double, double> fernsehturm{ 52.520803, 13.40945 };

}
