#include "pch.h"
#include <tuple>
#include "TTracer.h"

using Int = TTracer;
std::vector<Int> heapTrace;

std::tuple<Int, Int, Int> build(int val = 1)
{
	return { 1 * val,2 * val,3 * val };
}

TEST(sometuples, t1)
{
	auto val = build();
	std::cout << "\t" << std::get<0>(val);
	std::cout << "\t" << std::get<1>(val);
	std::cout << "\t" << std::get<2>(val);
	std::cout << "\n";
}

TEST(sometuples, t2)
{
	Int t1, t2, t3;
	std::tie(t1, t2, t3) = build(2);
	std::cout << "\t" << t1;
	std::cout << "\t" << t2;
	std::cout << "\t" << t3;
	std::cout << "\n";
}

TEST(sometuples, t3)
{
	auto[a, b, c] = build(3);
	std::cout << "\t" << a;
	std::cout << "\t" << b;
	std::cout << "\t" << c;
	std::cout << "\n";
}