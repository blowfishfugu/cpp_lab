#include "pch.h"
#include <string>
#include <algorithm>

void markedOut(std::string const& val)
{
	std::cout <<"|"<< val<<"|\n";
}

std::string trim(std::string const& strVal)
{
	std::string tmp = strVal;
	tmp.erase(0, tmp.find_first_not_of("\t "));
	tmp.erase(tmp.find_last_not_of(" \t\f") + 1, tmp.length());
	return tmp;
}

std::string& trim(std::string& strVal)
{
	//trim tabs space and formfeed
	strVal.erase(0, strVal.find_first_not_of("\t "));
	strVal.erase(strVal.find_last_not_of(" \t\f") + 1, strVal.length());
	return strVal;
}

std::string&& trim(std::string&& strVal)
{
	strVal.erase(0, strVal.find_first_not_of("\t "));
	strVal.erase(strVal.find_last_not_of(" \t\f") + 1, strVal.length());
	return std::move(strVal); //rval
}

std::string&& ltrim(std::string&& strVal)
{
	strVal.erase(0, strVal.find_first_not_of("\t "));
	return std::move(strVal); //rval
}

std::string&& rtrim(std::string&& strVal)
{
	strVal.erase(strVal.find_last_not_of(" \t\f") + 1, strVal.length());
	return std::move(strVal); //rval
}

std::string&& fulltrim(std::string&& strVal)
{
	return rtrim(ltrim(std::forward<std::string>(strVal)));
}

std::string fulltrim(std::string const& strVal)
{
	return rtrim(ltrim(std::forward<std::string>(std::string(strVal)))); //<-temp kopie, landet im forwarding
}

TEST(rvalues, s1)
{
	markedOut("hello"); //<-implizit copy in temporary

	std::string strTest1 = "   hello world  \t  ";
	markedOut(strTest1);
	std::string strTest2 = trim(strTest1);
	markedOut(strTest1);
	markedOut(strTest2);
	ASSERT_TRUE(strTest1.compare(strTest2) == 0); //<- die sind gleich, weil byRef
}

TEST(rvalues, s2_constCopy)
{
	const std::string strTest1 = "   hello world  \t  ";
	markedOut(strTest1);
	std::string strTest2 = trim(strTest1);
	markedOut(strTest1);
	markedOut(strTest2);
	ASSERT_FALSE(strTest1.compare(strTest2) == 0); //<- die sind nicht gleich, da die copy getrimmt
}

TEST(rvalues, s3_rval)
{
	std::string strTest2 = trim("   hello world  \t  ");
	markedOut(strTest2);
	ASSERT_FALSE(strTest2.compare("   hello world  \t  ") == 0); //<- die sind nicht gleich, da die copy getrimmt
}

TEST(rvalues, s4_forwarding)
{
	std::string strTest1 = "   hello world  \t  ";
	std::string strTest2 = trim(std::forward<std::string>(strTest1));
	markedOut(strTest1); //<-robbed
	markedOut(strTest2); //<-swapped and trimmed
	ASSERT_TRUE(strTest2.compare("hello world") == 0); 
	ASSERT_TRUE(strTest1.length() == 0); //forwarded, gave it up
}

TEST(rvalues, s5_combinedTrim)
{
	std::string strTest1 = "   hello world  \t  "; //<-
	std::string strTest2 = fulltrim(std::forward<std::string>(strTest1));
	markedOut(strTest2);
	ASSERT_TRUE(strTest2.compare("hello world") == 0); //<- die sind nicht gleich, da die copy getrimmt
}

TEST(rvalues, s6_forwardedCopy)
{
	std::string strTest1 = "   hello world  \t  ";
	std::string strTest2= fulltrim(strTest1);
	markedOut(strTest2);
	ASSERT_TRUE(strTest2.compare("hello world") == 0);
	ASSERT_TRUE(strTest1.compare("   hello world  \t  ") == 0);
}

TEST(shorthands, plusplusplus)
{
	int i = 0;
	i=i+=++i+=++i;
	std::cout << i << "\n";
}