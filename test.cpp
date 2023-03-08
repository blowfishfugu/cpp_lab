#include "pch.h"
#include <iostream>
#include <string>

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}


TEST(niche, nice)
{
#define void auto
#define console std::cout
	console.setf(std::ios::fixed);
	console.precision(3);

	void x = 10.0;
	void f = [](void& v)->void&
	{
		console << v * v << " hello\n";
		return console; 
	};
	f(x) << " world\n";
}