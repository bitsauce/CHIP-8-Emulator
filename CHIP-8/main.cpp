#include "config.h"
#include "chip-8.h"
#include <conio.h>
#include "SDL.h"

using namespace std;

BOOL setCursorXY(short x, short y)
{
	COORD c = { x, y };
	return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

SDL_Surface *screenSurface;

int main(int argc, char *argv[])
{
	SDL_Window *mainWindow;
	SDL_GLContext mainContext;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		return 0;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	mainWindow = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		64*8, 32*8, SDL_WINDOW_SHOWN);

	if (!mainWindow)
		return 0;

	//Get window surface
	screenSurface = SDL_GetWindowSurface(mainWindow);

	Chip8 emulator;
	emulator.init();

	bool running = true;
	SDL_Event event;
	Uint32 frametime;
	const Uint32 fps = 600;
	const Uint32 minframetime = 1000 / fps;

	while (running)
	{
		frametime = SDL_GetTicks();

		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
				else if (event.key.keysym.sym == SDLK_x) emulator.key[0] = 1;
				else if (event.key.keysym.sym == SDLK_1) emulator.key[1] = 1;
				else if (event.key.keysym.sym == SDLK_2) emulator.key[2] = 1;
				else if (event.key.keysym.sym == SDLK_3) emulator.key[3] = 1;
				else if (event.key.keysym.sym == SDLK_q) emulator.key[4] = 1;
				else if (event.key.keysym.sym == SDLK_w) emulator.key[5] = 1;
				else if (event.key.keysym.sym == SDLK_e) emulator.key[6] = 1;
				else if (event.key.keysym.sym == SDLK_a) emulator.key[7] = 1;
				else if (event.key.keysym.sym == SDLK_s) emulator.key[8] = 1;
				else if (event.key.keysym.sym == SDLK_d) emulator.key[9] = 1;
				else if (event.key.keysym.sym == SDLK_z) emulator.key[10] = 1;
				else if (event.key.keysym.sym == SDLK_c) emulator.key[11] = 1;
				else if (event.key.keysym.sym == SDLK_4) emulator.key[12] = 1;
				else if (event.key.keysym.sym == SDLK_r) emulator.key[13] = 1;
				else if (event.key.keysym.sym == SDLK_f) emulator.key[14] = 1;
				else if (event.key.keysym.sym == SDLK_v) emulator.key[15] = 1;
				break;

			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_x) emulator.key[0] = 0;
				else if (event.key.keysym.sym == SDLK_1) emulator.key[1] = 0;
				else if (event.key.keysym.sym == SDLK_2) emulator.key[2] = 0;
				else if (event.key.keysym.sym == SDLK_3) emulator.key[3] = 0;
				else if (event.key.keysym.sym == SDLK_q) emulator.key[4] = 0;
				else if (event.key.keysym.sym == SDLK_w) emulator.key[5] = 0;
				else if (event.key.keysym.sym == SDLK_e) emulator.key[6] = 0;
				else if (event.key.keysym.sym == SDLK_a) emulator.key[7] = 0;
				else if (event.key.keysym.sym == SDLK_s) emulator.key[8] = 0;
				else if (event.key.keysym.sym == SDLK_d) emulator.key[9] = 0;
				else if (event.key.keysym.sym == SDLK_z) emulator.key[10] = 0;
				else if (event.key.keysym.sym == SDLK_c) emulator.key[11] = 0;
				else if (event.key.keysym.sym == SDLK_4) emulator.key[12] = 0;
				else if (event.key.keysym.sym == SDLK_r) emulator.key[13] = 0;
				else if (event.key.keysym.sym == SDLK_f) emulator.key[14] = 0;
				else if (event.key.keysym.sym == SDLK_v) emulator.key[15] = 0;
				break;
			}
		}

		// Tick emulator
		emulator.emulate();
		if (emulator.drawFlag)
		{
			// Draw game
			SDL_Surface surface;
			char buffer[32 * 64];
			for (int y = 0; y < 32; ++y)
			{
				for (int x = 0; x < 64; ++x)
				{
					SDL_Rect rect;
					rect.x = x * 8;
					rect.y = y * 8;
					rect.w = rect.h = 8;
					SDL_FillRect(screenSurface, &rect, emulator.gfx[x + y * 64] == 0 ? 0x00000000 : 0xFFFFFFFF);
				}
			}
			SDL_UpdateWindowSurface(mainWindow);
			emulator.drawFlag = false;
		}

		if (SDL_GetTicks() - frametime < minframetime)
			SDL_Delay(minframetime - (SDL_GetTicks() - frametime));
	}

	return 0;
}