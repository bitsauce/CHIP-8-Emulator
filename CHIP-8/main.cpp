#include "config.h"
#include "chip-8.h"
#include <conio.h>

using namespace std;

BOOL setCursorXY(short x, short y)
{
	COORD c = { x, y };
	return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void draw(uchar *gfx)
{
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			setCursorXY(x, y);
			if (gfx[x + y * 64] > 0)
			{
				cout << char(178);
			}
			else
			{
				cout << char(0);
			}
		}
	}
}

/*#define MAXPATHLEN 512

std::wstring get_working_path()
{
	wchar_t temp[MAXPATHLEN];
	return (_wgetcwd(temp, MAXPATHLEN) ? std::wstring(temp) : std::wstring());
}*/

int main()
{
	SMALL_RECT window;
	window.Top = 0;
	window.Bottom = 31;
	window.Left = 0;
	window.Right = 63;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &window);

	//cout << get_working_path().c_str();
	Chip8 emulator;
	emulator.init();
	while (true)
	{
		emulator.emulate();
		if (emulator.drawFlag)
		{
			draw(emulator.gfx);
		}
	}
	_getch();
	return 0;
}