#include "stdafx.h"

#ifdef _UNIT_TEST
#include "Unit_Main.h"

#include "TimerLog.h"

using namespace std;

int myAtoi(const string& str)
{
	if (str.size() == 0) { return 0; }
	const char* s = str.data();
	const char* e = s + str.size() - 1;

	constexpr const int64_t bases[] = {
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000
	};
	//what's faster, lookup or calc -=48??
	constexpr int64_t nums[] = {
		0,0,0,0,0, 0,0,0,0,0,
		0,0,0,0,0, 0,0,0,0,0,
		0,0,0,0,0, 0,0,0,0,0,
		0,0,0,0,0, 0,0,0,0,0,
		0,0,0,0,0, 0,0,0,0,1,
		2,3,4,5,6, 7,8,9,0,0
	};
	//trim
	while (s <= e && *s == ' ')
	{
		s++;
	}
	if (*s != '-' && *s != '+' && (*s<'0' || *s>'9'))
	{
		return 0;
	}

	while (e >= s && (*e > '9' || *e < '0'))
	{
		e--;
	}


	std::function<void(int64_t&, int64_t)> op;
	bool isNegative = false;
	int res = 0;
	if (*s == '-')
	{
		isNegative = true;
		res = INT_MIN;
		op = [](int64_t& r, int64_t i) { r -= i; };
		s++;
	}
	else
	{
		res = INT_MAX;
		op = [](int64_t& r, int64_t i) {r += i; };
		if (*s == '+')
		{
			s++;
		}
	}

	while (*s == '0')
	{
		s++;
	}

	const char* mide = e;
	while (mide > s)
	{
		mide--;
		if (*mide<'0' || *mide>'9')
		{
			e = mide;
			e--;
		}
	}

	if (e < s)
	{
		return 0;
	}

	int n = e - s;

	if (n < 10)
	{
		res = 0;
		int64_t tmpres = 0;
		while (s <= e && n >= 0 && *s >= '0' && *s <= '9')
		{
			op(tmpres, (nums[*s] * bases[n]));
			if (tmpres > INT_MAX || tmpres < INT_MIN)
			{
				if (isNegative) { return INT_MIN; }
				else { return INT_MAX; }
			}
			s++;
			n--;
		}


		res = (int)tmpres;
	}


	return res;
}

TEST(Leet, tstAtoi)
{
	vector<string> strings = {
		"-91283472332",
		"-",
		"1",
	"42",
	"   -42",
	"4193 with words",
	"words and 987",
	"-91283472332",
	"-6147483648",
	"3.1415"

	};

	for (const string& str : strings)
	{
		int x = atoi(str.c_str());
		int y = myAtoi(str);
		printf("%d == %d\n", x, y);
		ASSERT_EQ(x, y);
	}

	{
		TimerLog p(_T("perfAtoi"));
		for (int i = 0; i < 100000; i++)
			for (const string& str : strings)
			{
				int x = atoi(str.c_str());
			}
	}

	{
		TimerLog p(_T("perfMytoi"));
		for (int i = 0; i < 100000; i++)
			for (const string& str : strings)
			{
				int y = myAtoi(str);
			}
	}
}

#endif