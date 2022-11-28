#pragma once

class TTracer
{
	friend std::ostream& operator<<(std::ostream& out, TTracer const& data);
private:
	int i;
public:
	TTracer(void);
	TTracer(TTracer const& rhs);
	TTracer(int val);
	~TTracer();

	TTracer& operator=(int rhs);

	operator int();
};
