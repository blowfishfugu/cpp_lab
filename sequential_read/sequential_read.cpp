// sequential_read.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "StopWatch.h"
#include "Loaders.h"
#include <iostream>
#include <vector>



int main(void)
{
	StopWatch stopWatch(std::cout);
	stopWatch.checkpoint("init done ");
	data_type data;
	__int64 linecount=load(data);
	std::cout << linecount << " items\n";
	stopWatch.checkpoint("loaded none ");

}
