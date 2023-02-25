#pragma once
#ifndef TYPECONFIGS_H
#define TYPECONFIGS_H
#include <vector>
#include "GeoLocation.h"

using data_type = std::vector<GeoLoc>;

enum LoadApproaches : int {
	read_none,
	read_standard,
};

constexpr LoadApproaches DefaultLoadType = read_none;

#endif