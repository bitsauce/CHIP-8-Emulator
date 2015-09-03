#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"

const int gfxWidth = 64;
const int gfxHeight = 32;
const float pixelSize = 32;

class Chip8
{
public:
	Chip8();
	void init();
	void emulate();

	// Graphics
	unsigned char gfx[2048];   // The screen is 64x32


	bool drawFlag;
};

#endif // CHIP8_H
