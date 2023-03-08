#pragma once
#ifndef READTOTOKEN_H
#define READTOTOKEN_H
#include <string_view>

//Erweiterbar zum Iterator..
//++,==
struct readInfos
{
	std::string_view all;
	int pos = 0;
};

struct range
{
	int begin{};
	int len{};
};

template<char token, bool RewindNewline=false>
range readTo(readInfos& text)
{
	int start = text.pos;
	int len = 0;
	while (text.pos < text.all.length())
	{
		//konsumieren immer
		char c = text.all[text.pos];
		//Achtung, bugpotenzial, readInfos, darf nicht weiterlaufen, wenn \n vor nächstem ; kommt!
		++text.pos;
		
		if (c == token)
		{
			break;
		}
		if constexpr (RewindNewline)
		{
			if (c == '\r') //ignore
			{
				continue;
			}
			if (c == '\n') //newline
			{
				break;
			}
		}
		++len; //token nicht mitzählen
	}
	return { start,len };
}


struct my_line_iterator
{
	using value_type = std::string_view;
	using difference_type = std::ptrdiff_t;
	using reference_type = value_type const&;
	using pointer_type = const value_type*;

	
};


#endif