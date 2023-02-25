// sequential_read.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "StopWatch.h"
#include "Loaders.h"
#include <iostream>
#include <vector>



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
	for (const auto& item : data)
	{
		item.print();
	}
}
