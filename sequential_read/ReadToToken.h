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

#endif