#pragma once
#ifndef LOADERS_H
#define LOADERS_H
#include "TypeConfigs.h"


template<LoadApproaches TLoadType=DefaultLoadType>
__int64 load(data_type& data) {
	return 0;
}

namespace LoadImpl {
	__int64 _loadByIfStream(data_type& data);
}

template<>
__int64 load<read_standard>(data_type& data) {
	return LoadImpl::_loadByIfStream(data);
}

#endif