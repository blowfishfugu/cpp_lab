#include "pch.h"

#include <iostream>

#include <algorithm>
#include <vector>
#include <array>

class TTracer
{
	friend std::ostream& operator<<(std::ostream& out, TTracer const& data)
	{
		out << "<< " << data.i;
		return out;
	}
private: 
	int i;
public:
	TTracer(void)
		: i(0)
	{ 
		std::cout << "default ctor " << i << std::endl; 
	}
	TTracer(TTracer const& rhs)
		: i(rhs.i)
	{
		std::cout << "copy ctor " << i << std::endl;
	}
	TTracer(int val)
		: i(val)
	{
		std::cout << "value ctor " << i << std::endl;
	}
	~TTracer()
	{
		std::cout << "default dtor " << i << std::endl;
	}

	TTracer& operator=(int rhs)
	{ 
		this->i = rhs; 
		std::cout << "assignment " << i << std::endl;
		return *this; 
	}

	operator int() 
	{ 
		std::cout << "cast int() " << i << std::endl;
		return i; 
	}
};

//bei Return erfolgt move-semantic :)
template<typename intType, int count=30>
std::vector<intType> build()
{
	std::vector<intType> vals;
	vals.reserve(count);
	for (int i = 0; i < count; ++i)
	{
		vals.emplace_back(i * 5);
	}
	return vals;
}

template<typename intType, size_t count = 30>
std::array<intType,count> buildFix()
{
	std::array<intType,count> vals;
	for (int i = 0; i < count; ++i)
	{
		vals[i]=i*5;
	}
	return vals;
}

TEST(move_semantic, cout_Timing)
{
	for (int i = 0; i < 30; ++i)
	{
		std::cout << (i * 5) << std::endl;
	}
}

TEST(move_semantic, move1)
{
	std::vector<TTracer> tt = build<TTracer>();
	for (const auto& t : tt)
	{
		std::cout << t << std::endl;
	}
}

TEST(move_semantic, move2)
{
	auto tt = buildFix<TTracer,30>();
	for (const auto& t : tt)
	{
		std::cout << t << std::endl;
	}
}