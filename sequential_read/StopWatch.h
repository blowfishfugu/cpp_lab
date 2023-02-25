#pragma once
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <string_view>
#include <iostream>

#ifdef NDEBUG
constexpr const char* build_mark = "ReleaseBuild\t";
#else
constexpr const char* build_mark = "DebugBuild\t";
#endif

template<typename StreamType=std::ostream>
struct StopWatch final {
	using timepoint = std::chrono::time_point<std::chrono::steady_clock>;
	using clock = std::chrono::high_resolution_clock;
	timepoint now;
	timepoint from_start;
	StreamType& output;
	StopWatch() = delete;
	StopWatch(StreamType& outputStream=std::cout) noexcept
		: 
		output(outputStream), from_start(clock::now())
	{
		now = from_start;
		return;
	}
	StopWatch(StopWatch const& ref) noexcept
		:
		output(ref.output), from_start(ref.from_start), now(ref.now)
	{
		return;
	}

	StopWatch(StopWatch&& rValue) noexcept
	{
		std::swap(output, rValue.output);
		std::swap(from_start, rValue.from_start);
		std::swap(now, rValue.now);
	}

	~StopWatch()
	{
		checkpoint("stopwatch finished ");
		return;
	}

	void reset() noexcept
	{
		from_start = now = (clock::now());
		return;
	}

	float checkpoint(std::string_view txt) noexcept{
		const auto local_now = clock::now();
		std::chrono::duration<float, std::milli> fp_total = local_now - from_start;
		std::chrono::duration<float, std::milli> fp_ms = local_now - now;
		output << build_mark;
		output << txt.data() << "\t" << fp_ms.count() << "ms\ttotal:\t" << fp_total.count() << "ms\n";
		now = clock::now(); //..ignore cout-time
		return fp_ms.count();
	};
};

#endif