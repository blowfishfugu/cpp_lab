#include "pch.h"
#include <functional>
#include <algorithm>

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

void test(
	std::function<bool(int, int)> sortFct
)
{
	std::vector<int> values = { 12,3,54,13,63,14,7,23,5,35,1,65,15,9,6,11,41 };
	auto it = std::partition(values.begin(), values.end(), TLesser(15) );
	
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

TEST(functional_ex, main)
{
	test(is_lesser);

}