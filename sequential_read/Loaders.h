#pragma once
#ifndef LOADERS_H
#define LOADERS_H
#include "TypeConfigs.h"


template<LoadApproaches TLoadType=DefaultLoadType>
__int64 load(data_type& data) {
	return 0;
}



template<>
__int64 load<read_standard>(data_type& data) {
	extern __int64 _loadByIfStream(data_type& data);
	return _loadByIfStream(data);
}

template<>
__int64 load<read_buffered>(data_type& data) {
	extern __int64 _loadBuffered(data_type& data);
	return _loadBuffered(data);
}

template<>
__int64 load<read_async_buffered>(data_type& data) {
	//extern __int64 _loadBufferedAsync(data_type& data);
	//return _loadBufferedAsync(data);
	return 0LL;
}


#endif