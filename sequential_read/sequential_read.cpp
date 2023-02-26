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
	//doubles
	std::cout.setf(std::cout.fixed);
	std::cout.precision(9);

	StopWatch stopWatch(std::cout);
	stopWatch.checkpoint("init done ");
	data_type data;
	__int64 linecount=load<read_standard>(data);
	std::cout << linecount << " items\n";
	stopWatch.checkpoint("load done ");
	
	if (data.size() > 0)
	{
		for (size_t i = 0; i < data.size(); i += 10000)
		{
			data[i].print();
		}
	}
	stopWatch.checkpoint("print done ");

	extern std::map<size_t, std::set<std::string> > StringPools;
	std::cout << "Indexes " << StringPools.size() << "\n";
	for (const auto& [index,pool] : StringPools)
	{
		std::cout << index << "\t Size: " << pool.size() << "\n";
	}
	stopWatch.checkpoint("loop indexes done");

	constexpr std::tuple<double, double> fernsehturm{ 52.520803, 13.40945 };

}
