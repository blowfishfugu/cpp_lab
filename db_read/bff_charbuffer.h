#pragma once
#include <array>
#include <algorithm>

constexpr const size_t MAXFIELDLEN = 256;
template<size_t szCapacity = MAXFIELDLEN>
struct CharBuffer
{
	using value_type = char;
	std::array<value_type, szCapacity> data{};
	const size_t capacity = szCapacity;
	explicit operator std::string()
	{
		//trimLeft()
		auto itStart = data.begin();
		while (itStart != data.end()) {
			const char c = *itStart;
			if (c < 33) { ++itStart; }
			else { break; }
		}
		//trimRight()	
		auto itEnd = std::find_if(itStart, data.end(), [](const char c) { return c < 33; });
		if (itEnd != data.end()) { *itEnd = '\0'; }

		return std::string(itStart, itEnd);
	}
	void clear() noexcept
	{
		data.fill(0);
	}

};


using DefaultRecord = CharBuffer<MAXFIELDLEN>;