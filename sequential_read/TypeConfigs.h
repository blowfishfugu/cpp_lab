#pragma once
#ifndef TYPECONFIGS_H
#define TYPECONFIGS_H
#include <vector>
#include "GeoLocation.h"

using data_type = std::vector<GeoLoc>;

enum LoadApproaches : int {
	read_none,
	read_standard,
	read_buffered,
};

constexpr LoadApproaches DefaultLoadType = read_none;

#endif