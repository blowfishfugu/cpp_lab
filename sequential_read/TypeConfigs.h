#pragma once
#ifndef TYPECONFIGS_H
#define TYPECONFIGS_H
#include <vector>

using data_type = std::vector<std::string>;

enum LoadApproaches : int {
	read_none,
	read_standard,
};

constexpr LoadApproaches DefaultLoadType = read_none;

#endif