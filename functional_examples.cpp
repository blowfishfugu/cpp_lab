#include "pch.h"
#include <functional>
#include <algorithm>
#include <array>

//standard
void output(int value)
{
	std::cout << std::setw(3) << value << ",";
}

//standard
bool lesser_15(int element)
{
	return element < 15;
}

//operator()
class TLesser {
private:
	int iMax;
public:
	TLesser() = delete;
	TLesser(const TLesser&) = default;
	TLesser(int v):iMax(v){}

	bool operator()(int const& element) const {
		return element < iMax;
	}
};

class TOut {
	int iCols;
	int iWidth;
	mutable int iCount;
public:
	TOut(int cols, int width) 
		:iCols(cols), iWidth(width), iCount(0)
	{}
	void operator()(int const& element) const
	{
		if (iCount == 0) { std::cout << "\n"; }
		
		std::cout << std::setw(iWidth) << element;
		
		++iCount;
		if (iCount%iCols == 0 ) { std::cout << "\n"; }
		else{	std::cout << ","; }
	}
};

template<int maxval>
constexpr bool t_lesser(int val) {
	return val < maxval;
}

void test(
	std::function<bool(int, int)> sortFct
)
{
	auto tstBind = std::bind([](const int maxval, const int x)->bool {return x < maxval; }, 15, std::placeholders::_1);
	
	std::vector<int> values = { 12,3,54,13,63,14,7,23,5,35,1,65,15,9,6,11,41 };
	auto it = std::partition(values.begin(), values.end(), TLesser(15) );
	auto it2 = std::partition(values.begin(), values.end(), tstBind );
	auto it3 = std::partition(values.begin(), values.end(), t_lesser<15> );
	
	std::for_each(values.begin(), values.end(), TOut(10,2));
	std::cout << "\n";
	
	std::sort(values.begin(), it, sortFct);
	std::for_each(values.begin(), it, TOut(5,4) );
	std::cout << "\n";

	std::sort(it, values.end(), sortFct);
	std::for_each(it, values.end(), TOut(5,4) );
	std::cout << "\n";

}

static bool is_lesser(int l, int r) { return l < r; }

struct my_iterator {
	using iterator_category = std::input_iterator_tag;
	using value_type = int;
	using difference_type = std::ptrdiff_t;
	using reference_type = value_type & ;
	using pointer_type = value_type * ;

	my_iterator()
	{}
	my_iterator(value_type p[]) : data(p)
	{
		//++*this;
	}

	//cpy
	//assign
	//++
	//++(int)
	//friend ==
	//friend !=

private:
	value_type* data=nullptr;
	int start_pos = -1;
	int end_pos = -1;
};

//ideen: fileiterator foreach(char in stream)
//ideen: reflection iterator, der über felder zählt
//ideen: sql-select mit ++

class MyArray {
private:
	int data[5];
public:
	my_iterator begin() { return my_iterator{data}; }
	my_iterator end() { return my_iterator{}; };
};

TEST(functional_ex, main)
{
	test(is_lesser);

}

template <typename iterator, typename Pred, typename Operation>
void for_each_if(iterator begin, iterator end, Pred pred, Operation op)
{
	if (begin < end)
	{
		for (; begin != end; ++begin)
		{
			if (pred(*begin))
			{
				op(*begin);
			}
		}
	}
	else throw std::range_error("iterator begin isn't before iterator end");
	return;
}

bool is_odd(int v) { return (v % 2) != 0; }
bool always(int) { return true; }
void mult_2(int& v) { v *= 2; }

TEST(functional_ex, combinations)
{
	int values[] = { 12,3,54,13,63,14,7,23,5,35,1,65,15,9,6,11,41 };
	
	//constexpr size_t maxindex = (sizeof(values) / sizeof(int))-1LL;
	int* begin = std::begin(values); // &values[0];
	int* end = std::end(values);//!! gefährlich? std::end zeigt eins hinter array, &values[maxindex];
	for_each_if(begin, end, always, output); std::cout << "\n";
	for_each_if(begin, end, is_odd, mult_2);
	for_each_if(begin, end, always, output); std::cout << "\n";
	
}

struct my_map_not_found{};

template<typename ty>
typename ty::mapped_type& my_map_find(
	ty& container, 
	typename ty::key_type const& key)
{
	auto it = container.find(key);
	if (it != container.end())
	{
		return it->second;
	}
	else
	{
		throw my_map_not_found();
	}
}

TEST(functional_ex, maps)
{
	std::map<int, int> test = {
		{1,20},{12,12}
	};

	try 
	{
		auto& found = my_map_find(test, 12);
		std::cout << found << " found\n";
	}
	catch (my_map_not_found& )
	{
		std::cout << "key not found\n";
	}
}

#include <thread>
#include <future>
#include <mutex>
#include <atomic>

TEST(functional_ex, threading)
{
	std::vector<int> values = { 12,3,54,13,63,14,7,23,5,35,1,65,15,9,6,11,41 };
	auto it = std::partition(values.begin(), values.end(), is_odd);
	for_each_if(values.begin(), values.end(), always, output);
	std::cout << "\n";

	auto thread_func = [](auto begin, auto end, auto op) { 
		std::for_each(begin, end, op); 
	};

	std::thread t1( thread_func,values.begin(), it, [](auto& val) mutable {val *= 2; } );
	std::thread t2( thread_func, it, values.end(), [](auto& val) mutable {val *= 3; } );
	t1.join(); // t1.detach(); //"laufen lassen", bis zum ProgrammEnde
	t2.join();

	for_each_if(values.begin(), values.end(), always, output);
	std::cout << "\n";
}