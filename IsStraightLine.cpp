#include "stdafx.h"
#ifdef _UNIT_TEST
#include "Unit_Main.h"

typedef std::vector<int> XY;
typedef std::vector<XY> PointList;

int getDelta(const int& v1, const int& v2)
{
	if (v1 > v2)
	{
		return v1- v2;
	}
	else
	{
		return v2 - v1;
	}
}

void getDelta(const XY& p1, const XY& p2, XY& delta)
{
	delta.emplace_back(getDelta(p1[0], p2[0]));
	delta.emplace_back(getDelta(p1[1], p2[1]));
}

bool checkStraightLine(PointList& coordinates) 
{
	PointList deltas;
	PointList::const_iterator s = coordinates.begin();
	PointList::const_reverse_iterator e = coordinates.rbegin();
	bool isEven = (coordinates.size() % 2) ==0;
	int halflen = coordinates.size() / 2;
	if (!isEven) { halflen++; }

	XY lastDelta = { 0,0 };
	for( int i=0;i<halflen;i++)
	{ 
		const XY& p1 = *s; s++;
		const XY& p2 = *e;
		if (i < (halflen - 2)) { e++; }
		
		XY delta;
		getDelta(p1, p2, delta);
		if (delta[0] != 0 && delta[1] != 0)
		{
			int d = std::gcd(delta[0], delta[1]);
			delta[0] /= d;
			delta[1] /= d;
		}

		if (i == 0)
		{
			lastDelta = delta;
		}
		else
		{
			if (lastDelta[0] != delta[0] && lastDelta[1] != delta[1])
			{
				return false;
			}
			lastDelta = delta;
		}

		deltas.emplace_back(std::move(delta));
	}
	return true;
}

//fetched from 8ms solution, after positive submission, testcase-checking
inline bool naiveStraightLine(PointList& coordinates) {
	//y=mx+c
	//Find m
	double m = 0;
	double num = coordinates[1][1] - coordinates[0][1];
	double den = coordinates[1][0] - coordinates[0][0];
	m = (num / den);
	double c = (coordinates[0][1] - (m*coordinates[0][0]));

	for (int i = 2; i < coordinates.size(); i++)
	{
		if ((coordinates[i][1] - (m*coordinates[i][0])) != c)
			return false;
	}

	return true;

}


TEST(Leet, IsStraightLine)
{
	PointList trueList = { {-1,0}, {0,1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} };
	PointList falseList = { {-1,-1}, {1, 1}, {2, 2}, {3, 4}, {4, 5}, {5, 6},{4,1}, {7, 7} };
	naiveStraightLine(trueList);
	ASSERT_TRUE(checkStraightLine(trueList));
	ASSERT_FALSE(checkStraightLine(falseList));
}

#endif