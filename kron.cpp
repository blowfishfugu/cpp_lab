#include "pch.h"
#include <chrono>
#include <array>
#include <functional>
#include <algorithm>

#if _MSC_VER <= 1916
//vs2017 c++17
//Kalender erst ab c++20, für ältere c++ Versionen brauchen wir den hier:
//https://github.com/HowardHinnant/date
//Additional include-directory: $(MSBuildThisFileDirectory)Hinnant_date\include
//tz.h+tz.cpp entfernt, enthält ungewollte shgetknownfolder/downloads/process-start usw.
#include <date/date.h>
#endif

TEST(kron, ymd)
{
	namespace chrono = std::chrono;
	namespace date = std::chrono; // std::chrono;
	using namespace std::chrono_literals;
	//using namespace date::literals;
	date::year_month_day ymd = { date::year{2022},date::month{1},date::day{1} };
	std::cout << ymd << "\n";
	date::year_month_day ymd2 = { date::year{2022}, date::January, date::day{1} };
	ASSERT_EQ(ymd, ymd2);

	auto ymd3 = date::year{ 2022 } / date::month{ 1 } / date::day{ 1 };
	ASSERT_EQ(ymd, ymd3);

	int days_since_epoch = date::sys_days(ymd).time_since_epoch().count();
	std::cout << "since epoch days= " << days_since_epoch << "\n";
}

TEST(kron, date_diffs)
{
	namespace chrono = std::chrono;
	namespace date = std::chrono; // std::chrono;
	
	using namespace std::chrono_literals;
	
	auto ymd = 2022y / 1/ 1;
	auto ymd_later = 2022y / 12 / 31;
	
	ASSERT_TRUE(ymd.ok());
	ASSERT_TRUE(ymd_later.ok());

	auto daydiff = (date::sys_days(ymd_later) - date::sys_days(ymd)).count();
	ASSERT_EQ(daydiff, 364);
}

constexpr
bool
is_weekend(std::chrono::year_month_day ymd)
{
	using namespace std::chrono;
	const std::chrono::sys_days days = ymd;
	const std::chrono::weekday wd{ days };
	return wd == std::chrono::Saturday || wd == std::chrono::Sunday;
}

TEST(kron, weekdays)
{
	namespace chrono = std::chrono;
	namespace date = std::chrono; // std::chrono;
	using namespace std::chrono_literals;
	constexpr int year = 2022;
	date::month m = date::January;
	date::year_month_day today=date::floor<date::days>(std::chrono::system_clock::now());
	std::cout << is_weekend(today) << "\n";

	do {
		date::year_month_day_last ym_last{ date::year{year} / m / date::last };
		//std::cout <<  ym_last.month() << " " << static_cast<unsigned>(ym_last.day()) << "\n";
		
		const date::year_month_day from{ date::year{year} / m / 1 };
		const date::year_month_day to{ ym_last };
		std::cout << from << " " << to <<"\n";
		int daycount = 0;
		for (auto d = from.day(); d <= to.day(); d++)
		{
			date::sys_days days = date::year_month_day{ from.year(),from.month(),d };
			date::weekday wd{ days };
			std::cout << d << " " << wd << " ";
			++daycount;
			if (daycount % 7 == 0 && daycount!=static_cast<unsigned>(ym_last.day()) ) { std::cout << "\n"; }
		}
		++m;
		std::cout << "\n\n";
	} while (m != date::January);
}

//impl aus ym_last.day()
constexpr auto last_day_of_the_month=[](const std::chrono::year_month_day& ymd)->unsigned
{
	unsigned m = static_cast<unsigned>(ymd.month());
	constexpr static std::array<unsigned, 12> normals{31,28,31,30,31,30,31,31,30,31,30,31};
	return (m != 2 || !ymd.year().is_leap() ? normals[m - 1] : 29);
};