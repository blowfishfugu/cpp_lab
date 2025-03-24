#include "stdafx.h"
#ifdef _UNIT_TEST
#include "Unit_Main.h"

namespace {
	constexpr const int knownSquares[] =
	{ 
		0,
		1,
		2 * 2,
		3 * 3,
		4 * 4,
		5 * 5,
		6 * 6,
		7 * 7,
		8 * 8,
		9 * 9
	};

	bool isKnownSquare(int num, int& root)
	{
		if (num < 100)
		{
			int i = 0;
			for (int t : knownSquares)
			{
				if (t == num)
				{
					root = i;
					return true;
				}
				i++;
			}
		}
		return false;
	}

	int closestRoot(int num, int& base)
	{
		for (int i = 9; i >= 0; i--)
		{
			if (knownSquares[i] <= num)
			{
				base = i;
				return knownSquares[i];
			}
		}
		base = 0;
		return num;
	}

	int nextfit(int maxnum, int carry,int& base)
	{
		for (int i = 9; i >= 0; i--)
		{
			//(2x+i) *i
			int tst = (carry + i) * i;
			if (tst <= maxnum)
			{
				base = (base*10)+i;
				return tst;
			}
		}
		return 0;
	}

	// https://youtu.be/EnxV3_1oaOU?t=470
	//njwildenberger, sqrt by hand.
	int getRemainderOfVedicApproach(int num, int& result)
	{
		std::vector<int> parts;
		while (num != 0) //split to xx-packages
		{
			int r = num % 100;
			num /= 100;
			parts.push_back(r);
		}
		//51 63 85 96 but reverse
		
		// xx + (2x+y)y
		int idx = parts.size() - 1;
		int base = 0;
		int lastpart = parts[idx];
		int xx = closestRoot(parts[idx], base); //entry, reverse lookup in known 0..99
		idx--;

		for (int i=idx;i>=0;i--)
		{
			//200 + 63
			int tmp = ((lastpart - xx) * 100) + parts[i]; //subtract shift left + append next part.
			
			int carry = 2*base*10;
			xx = nextfit(tmp, carry,base); // (2x+y)*y //2x is known and preshifted, y walks 9 to 0, first lower wins
			lastpart = tmp;
		}
		result = base;
		return lastpart - xx;
	}

		

	bool isPerfectSquare(int num)
	{
		int foundRoot = 0;
		if (isKnownSquare(num, foundRoot))
		{
			return true;
		}
		else if (num < 100)
		{
			return false;
		}
		//unknown and bigger/equal 100
		int result = 0;
		int m = getRemainderOfVedicApproach(num,result);
		if (m == 0)
		{
			return true;
		}
		return false;
	}

	
}

TEST(Leet, IsValidSquare)
{
	ASSERT_TRUE(isPerfectSquare(9));
	ASSERT_TRUE(isPerfectSquare(1));
	ASSERT_FALSE(isPerfectSquare(3));
	ASSERT_TRUE(isPerfectSquare(51638596));
	ASSERT_TRUE(isPerfectSquare(189 * 189));
	ASSERT_FALSE(isPerfectSquare(188 * 189));
}

#endif