#include <Windows.h>
#include <stdio.h>
#include "../sequential_read/StopWatch.h"

constexpr int loopCount = 100000;
void helloPrint0()
{
	printf("Hello World!\n");
	for (int i = 0; i < loopCount; ++i)
	{
		if ((i % 4) == 0) { printf("%d\r", i); }
	}
}

void helloPrint0_unfiltered()
{
	printf("Hello World!\n");
	for (int i = 0; i < loopCount; ++i)
	{
		printf("%d\r", i);
	}
}

void helloPrint1()
{
	printf("Hello World!\n");
	for (int i = 0; i < loopCount; ++i)
	{
		int z = i % 4;
		switch (z) {
		case(0):printf("|\r"); break;
		case(1):printf("/\r"); break;
		case(2):printf("-\r"); break;
		case(3):printf("\\\r"); break;
		default: break;
		}
	}
}

void helloPrint2()
{
	static const char rot[4][3]{ "|\r","/\r","-\r","\\\r" };
	printf("Hello World!\n");
	for (int i = 0; i < loopCount; ++i)
	{
		printf(rot[i % 4]);
	}
}

void helloPrint3()
{
	static char rot[4][3]{ "|\r","/\r","-\r","\\\r" };
	static const size_t N = 2 << 7;
	static char* table[N];
	for (int i = 0; i < N; ++i)
	{
		table[i] = &rot[i % 4][0];
	}

	printf("Hello World!\n");
	byte pos = 255;
	for (int i = 0; i < loopCount; ++i)
	{
		printf(table[(byte)++pos]);
	}
}


void helloPrint4()
{
	static const WORD attr = FOREGROUND_INTENSITY;
	static const CHAR_INFO rot[4]{ {'|',attr},{'/',attr},{'-',attr},{'\\',attr} };
	COORD bufSize = { 4,1 };
	printf("Hello World!\n");
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO cursor{};
	GetConsoleScreenBufferInfo(console, &cursor);
	COORD pos = cursor.dwCursorPosition;
	SMALL_RECT outRect{ pos.X,pos.Y,pos.X,pos.Y };
	for (int i = 0; i < loopCount; ++i)
	{
		WriteConsoleOutput(console, rot, bufSize, { (SHORT)(i % 4),0 }, &outRect);
	}
}

void helloPrint5()
{
	static char rot[4] = { '|','/','-','\\' };
	printf("Hello World!\n");
	RECT r{ 0,20 * 10,40,20 * 10 + 80 };
	HWND wnd = GetConsoleWindow();
	HDC dc = ::GetDC(wnd);
	for (int i = 0; i < loopCount; ++i)
	{
		DrawText(dc, &rot[i % 4], 1, &r, DT_INTERNAL);
	}
	::ReleaseDC(wnd, dc);
}

void helloPrint6()
{
	static char rot[4] = { '|','/','-','\\' };
	printf("Hello World!\n");

	HWND wnd = GetConsoleWindow();
	RECT bounds;
	::GetWindowRect(wnd, &bounds);
	POINT topleft{ bounds.left,bounds.top };
	POINT bottomRight{ bounds.right,bounds.bottom };
	::ScreenToClient(wnd, &topleft);
	::ScreenToClient(wnd, &bottomRight);
	bounds = RECT{ topleft.x,topleft.y,bottomRight.x,bottomRight.y };
	const int r = 40;
	const int penWidth = 8;
	POINT center{ bottomRight.x - r * 2, topleft.y + r * 2 };
	RECT drawRect{ center.x - r - penWidth,center.y - r - penWidth, center.x + r + penWidth,center.y + r + penWidth };

	HDC dc = ::GetDC(wnd);
	HBRUSH backBrush = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	HPEN pen = CreatePen(PS_SOLID, 8, RGB(0xBA, 0xAA, 0xCC));
	HPEN oldpen = (HPEN)::SelectObject(dc, pen);
	for (int i = 0; i < loopCount; ++i)
	{
		const int y = static_cast<int>( std::sin(i) * r );
		const int x = static_cast<int>(std::cos(i) * r);
		FillRect(dc, &drawRect, backBrush);
		MoveToEx(dc, center.x + -x, center.y + -y, NULL);
		LineTo(dc, center.x + x, center.y + y);
	}
	FillRect(dc, &drawRect, backBrush);
	::SelectObject(dc, oldpen);

	::DeleteObject(pen);
	::ReleaseDC(wnd, dc);
}

void helloPrint7()
{
	static char rot[4] = { '|','/','-','\\' };
	printf("Hello World!\n");

	HWND wnd = GetConsoleWindow();
	RECT bounds;
	::GetWindowRect(wnd, &bounds);
	POINT topleft{ bounds.left,bounds.top };
	POINT bottomRight{ bounds.right,bounds.bottom };
	::ScreenToClient(wnd, &topleft);
	::ScreenToClient(wnd, &bottomRight);
	bounds = RECT{ topleft.x,topleft.y,bottomRight.x,bottomRight.y };
	const int r = 40;
	const int penWidth = 8;
	POINT center{ bottomRight.x - r * 2, topleft.y + r * 2 };
	RECT drawRect{ center.x - r - penWidth,center.y - r - penWidth, center.x + r + penWidth,center.y + r + penWidth };

	HDC dc = ::GetDC(wnd);
	HBRUSH backBrush = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	HPEN pen = CreatePen(PS_SOLID, 8, RGB(0xBA, 0xAA, 0xCC));
	HPEN oldpen = (HPEN)::SelectObject(dc, pen);
	for (int i = 0; i < loopCount; ++i)
	{
		if ((i % (loopCount / 10)) == 0)
		{
			const int x = static_cast<int>(std::sin(i) * r);
			const int y = static_cast<int>(std::cos(i) * r);
			FillRect(dc, &drawRect, backBrush);
			MoveToEx(dc, center.x + -x, center.y + -y, NULL);
			LineTo(dc, center.x + x, center.y + y);
		}
	}
	FillRect(dc, &drawRect, backBrush);
	::SelectObject(dc, oldpen);

	::DeleteObject(pen);
	::ReleaseDC(wnd, dc);
}

void helloPrint8()
{
	printf("Hello World!\n");
	for (int i = 0; i < loopCount; ++i)
	{
		if ((i % (loopCount / 10)) == 0) { printf("%d\r", i); }
	}
}




typedef void(*fct)();
void measure(fct f, const char* lbl)
{
	StopWatch watch(std::cout);
	f();
	watch.checkpoint(lbl);
}
int runHellos()
{
	measure(helloPrint0,			"helloPrint0 printf mit i%4 gefiltert     ");
	measure(helloPrint0_unfiltered, "helloPrint0 ohne modulo                  ");
	measure(helloPrint1,			"helloPrint1 switch(0..4)                 ");
	measure(helloPrint2,			"helloPrint2 print array[i%4]             ");
	measure(helloPrint3,			"helloPrint3 table mit byteueberlauf      ");
	measure(helloPrint4,			"helloPrint4 writeconsoleoutput array[i%4]");
	measure(helloPrint5,			"helloPrint5 drawtext i%4                 ");
	measure(helloPrint6,			"helloPrint6 gdi drawline i%1             ");
	measure(helloPrint7,			"helloPrint7 gdi drawline i%(count/10)    ");
	measure(helloPrint8,			"helloPrint8 printf mit i%(count/10)      ");
	return 0;
}