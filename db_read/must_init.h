#pragma once
#include <type_traits>

//vermeidung von leerkonstruktionen auf einfachen wertetypen
template <typename T>
class must_init final
{
private:
	T value;
public:
	must_init() = delete;
	must_init(T t) :value{t} {}
	must_init(T&& t):value{ std::forward<T>(t) } {}
	operator T& () { return value; }
	operator T const& () const { return value; }
};