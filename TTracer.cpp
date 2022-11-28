#include "pch.h"
#include "TTracer.h"

TTracer::TTracer(void)
	: i(0)
{
	std::cout << "default ctor " << i << std::endl;
}

TTracer::TTracer(TTracer const & rhs)
	: i(rhs.i)
{
	std::cout << "copy ctor " << i << std::endl;
}

TTracer::TTracer(int val)
	: i(val)
{
	std::cout << "value ctor " << i << std::endl;
}

TTracer::~TTracer()
{
	std::cout << "default dtor " << i << std::endl;
}
TTracer& TTracer::operator=(int rhs)
{
	this->i = rhs;
	std::cout << "assignment " << i << std::endl;
	return *this;
}

TTracer::operator int()
{
	std::cout << "cast int() " << i << std::endl;
	return i;
}

std::ostream& operator<<(std::ostream& out, TTracer const& data)
{
	out << "<< " << data.i;
	return out;
}