#include <Windows.h>
#include <sal.h>
#include <tchar.h>
#include "bff_odbc.h"
#include "sample.h"
#include <iomanip> //quoted
#include <ranges>
#include <algorithm>

bool tstTupleLike() {
	std::tuple<int, long, int> t{ 1,2,3 };
	std::array<int, 3> a{ 1,2,3 };
	return t == a;
	//return std::get<0>(t) == std::get<0>(a);
}

static bool result = tstTupleLike();


template<typename Input>
concept ContiguousSizedCollection = true;

template <typename F, typename... Args>
concept TernaryFunction = std::invocable<F, Args...>;

template <typename Executor, std::invocable F,
	typename InputIt1,
	typename InputIt2,
	typename InType1 = InputIt1::value_type,
	typename InType2 = InputIt2::value_type,
	typename ResultPair = typename std::invoke_result_t<F, size_t, InType1, InType2>::value_type,
	typename FirstResultType = ResultPair::first_type,
	typename SecondResultType = ResultPair::second_type,
	typename OutputIt1 = std::vector<FirstResultType>,
	typename OutputIt2 = std::vector<SecondResultType>,
	bool BoundsChecking = false>
	requires
ContiguousSizedCollection<InputIt1> && ContiguousSizedCollection<InputIt2> &&
ContiguousSizedCollection<OutputIt1> && ContiguousSizedCollection<OutputIt2>&&
TernaryFunction< F,
	size_t, InType1, InType2, std::optional< std::pair<FirstResultType, SecondResultType> >
>
auto parallel_enumerate_map2_filter2(
	Executor& pool,
	F&& f,
	const InputIt1& it1,
	const InputIt2& it2,
	OutputIt1&& out1 = std::vector<FirstResultType>(),
	OutputIt2&& out2 = std::vector<SecondResultType>()
) -> std::pair<decltype(std::forward<OutputIt1&&>(out1)), decltype(std::forward<OutputIt2&&>(out2))>
{
	for (size_t i = 0; i < std::min(it1.size(), it2.size()); ++i) {
		if (auto opt = f(i, it1[i], it2[i])) {
			out1.push_back(opt->first);
			out2.push_back(opt->second);
		}
	}
	return { out1,out2 };
}

#include <concepts>
#include <random>
#include <print>
#include <iostream>
template <typename ty> 
concept Integer = std::integral<ty> && (!std::is_same_v < std::remove_cv_t<ty>, bool>);

template<Integer ty>
ty randomInteger(ty const& min, ty const& max) {
	static std::random_device rd;
	static std::mt19937 gen{ rd() };
	std::uniform_int_distribution<ty> dist(min, max);
	return dist(gen);
}

int runRandoms() {
	constexpr Integer auto min = -20;
	constexpr Integer auto max = 20;
	for (auto _ : std::views::iota(0, 30) ) {
		auto val = randomInteger( min, max );
		std::println("{:.>{}}{:>{}d}{:.<{}}", "", val - min , val, 3, "", max - val);
	}
	return 0;
}

//microsoft-signatur, mit sal.h.. braucht man das?
//int __cdecl _tmain(int argc, _In_reads_(argc) TCHAR** argv)
int main(int argc, char** argv)
{
	//tstTupleLike();
	//return 0;
	
	//extern int runHellos();
	//return runHellos();

	return runRandoms();

	const char* pwszConnStr =
		"DRIVER={ODBC Driver 18 for SQL Server}"
		";SERVER=MENACE\\SQL2012"
		";DATABASE=destatis"
		";Trusted_Connection=YES"
		";Encrypt=YES"
		";TrustServerCertificate=YES";
	if (argc > 1)
	{
		pwszConnStr = *++argv;
	}

	HDbc db;
	std::vector<HEnv::DriverInfo> drivers = db._env.GetDrivers();
	for (const auto& driver : drivers)
	{
		const auto& [desc, descLen, attr, attrLen] = driver;
		std::cout << desc << "\n";
		std::cout << attr << "\n";
	}

	std::cerr << "try connect\n";
	db.TryConnect(pwszConnStr);
	std::cerr << (db ? (SQLHANDLE)db : "no db") << " connected=" << db.connected << "\n";
	std::cout << db.GetInfo(SQL_SPECIAL_CHARACTERS) << "\n";
	std::vector<InfoReturn> infos = db.GetRegisteredInfos();
	for (const auto& info : infos)
	{
		std::cout << info << "\n";
	}

	Query q = db.CreateQuery("SELECT * FROM de");
	const auto [colCount, rowCount] = q.Execute();
	if (rowCount >= 0)
	{
		std::cout << rowCount << (rowCount == 1 ? _T(" row") : _T(" rows")) << " affected\n";
	}
	if (colCount > 0)
	{
		__int64 fetchCount = 0;
		while (q.Fetch())
		{
			++fetchCount;
			std::vector<std::string> trimmedItems;
			size_t index = 0;
			for (auto& bufItem : q.buffer)
			{
				trimmedItems.emplace_back(conv(bufItem, index++));
			}

			if ((fetchCount % 1000) == 0)
			{
				for (const auto& trimmed : trimmedItems)
				{
					std::cout << std::quoted(trimmed, '\'') << " ";
				}
				std::cout << "\n";
			}
		}
		std::cout << fetchCount << "\n";
	}
	return 0;

	return sample(argc, argv);
}
