#include "chip-8.h"

// Font set
unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

// Opcodes
ushort opcode;

// Memory
uchar memory[4096]; // The chip-8 has 4KB memory

					// SYSTEM MEMORY MAP:
					// 0x000 - 0x1FF: Chip-8 interpreter
					// 0x050 - 0x0A0: Built in 4x5 pixel font set
					// 0x200 - 0xFFF: Program ROM and work RAM

					// Registries
uchar V[16];        // V0 to VE. VF is a carry flag

					// Index register and program counter
ushort I;
ushort pc;

// Timers
uchar delayTimer;
uchar soundTimer;   // When this reaches 0 the buzzer makes a noise

					// Stack
ushort stack[16];   // The chip-8 stack of 16
ushort sp;          // Stack pointer

					// Keypad
uchar key[16];

Chip8::Chip8()
{
}

void Chip8::init()
{
	pc = 0x200;         // Start address of the ROM
	opcode = 0;         // Reset op code
	I = 0;              // Reset index register
	sp = 0;             // Reset stack pointer
	drawFlag = true;    // Reset draw flag
	delayTimer = 0;     // Reset delay timer
	soundTimer = 0;     // Reset sound timer

						// Clear gfx
	for (uint i = 0; i < 2048; i++)
		gfx[i] = 0;

	// Clear stack, register and keys
	for (uint i = 0; i < 16; i++)
		stack[i] = V[i] = key[i] = 0;

	// Clear memory
	for (uint i = 0; i < 4096; i++)
		memory[i] = 0;

	// Load fontset
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];

	// Load ROM
	ifstream file("ROMS/SPACEINVADERS.CH8", iostream::binary);
	if (file)
	{
		// get length of file:
		file.seekg(0, file.end);
		int length = (int) file.tellg();
		file.seekg(0, file.beg);

		char *buffer = new char[length];
		file.read(buffer, length);

		memcpy(memory + 0x200, buffer, length);
	}
	file.close();
}

uchar toKey[16] = {
	0x30, // 0 -> X
	0x31, // 1 -> 1
	0x32, // 2 -> 2
	0x33, // 3 -> 3
	0x51, // 4 -> Q
	0x57, // 5 -> W
	0x45, // 6 -> E
	0x41, // 7 -> A
	0x53, // 8 -> S
	0x44, // 9 -> D
	0x5A, // A -> Z
	0x43, // B -> C
	0x34, // C -> 4
	0x52, // D -> R
	0x46, // E -> F
	0x56  // F -> V
};

#include <stdarg.h>

void print(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int size = _scprintf(fmt, args) + 1;
	char *buffer = new char[size];
	vsprintf_s(buffer, size, fmt, args);

	va_end(args);

	wchar_t *wbuffer = new wchar_t[size];
	size_t outSize;
	mbstowcs_s(&outSize, wbuffer, size, buffer, size - 1);
	OutputDebugString(wbuffer);
	OutputDebugString(L"\n");

	delete[] buffer;
	delete[] wbuffer;
}

void Chip8::emulate()
{
	// Get input
	for (uint i = 0; i < 16; ++i)
	{
		key[i] = (GetKeyState(toKey[i]) & 0x8000) != 0;
	}

	// Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Decode opcode
	switch (opcode & 0xF000) // Keeps the 4 first bytes
	{
	case 0x0000:
	{
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x00E0: Clears the screen
			for (int i = 0; i < 2048; ++i)
				gfx[i] = 0;
			drawFlag = true;
			pc += 2;
			break;

		case 0x000E: // 0x00EE: Returns from subroutine
			sp--;			// 16 levels of stack, decrease stack pointer to prevent overwrite
			pc = stack[sp];	// Put the stored return address from the stack back into the program counter
			pc += 2;		// Don't forget to increase the program counter!
			break;

		default:
			print("Unknown opcode [0x0000]: 0x%X", opcode);
		}
	}
	break;

	case 0x1000: // 1NNN: Jumps to address at NNN
	{
		// Set address
		pc = opcode & 0x0FFF;
	}
	break;

	case 0x2000: // 2NNN: Call subroutine at NNN
	{
		// Store current routine in stack
		stack[sp] = pc;
		sp++;                   // Increment stack pointer (count)
		pc = opcode & 0x0FFF;   // Call subroutine
	}
	break;

	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
	{
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4; // Skip next call
		else
			pc += 2; // Perceed
	}
	break;

	case 0x4000: // 4XNN: Skips the next instruction if VX isn't equal to NN
	{
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4; // Skip next call
		else
			pc += 2; // Perceed
	}
	break;

	case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
	{
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4; // Skip next call
		else
			pc += 2; // Perceed
	}
	break;

	case 0x6000: // 6XNN: Sets VX to NN
	{
		// NOTE TO SELF: opcode & 0x0F00 masks out the 3rd byte
		// and >> 8 moves the bits so that we get integer value
		// of the 3rd byte
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
	}
	break;

	case 0x7000: // 7XNN: Adds NN to VX
	{
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
	}
	break;

	// 0x8000 opcodes
	case 0x8000:
	{
		switch (opcode & 0x000F)
		{
		case 0x0000: // 8XY0: Sets VX to the value of VY
		{
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
		}
		break;

		case 0x0001: // 8XY1: Sets VX to VX or VY
		{
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		}
		break;

		case 0x0002: // 8XY2: Sets VX to VX and VY
		{
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		}
		break;

		case 0x0003: // 8XY3: Sets VX to VX xor VY
		{
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		}
		break;

		case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
		{
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) // TODO: Figgure out why
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
		}
		break;

		case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
		{
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 0; // there is a borrow
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		}
		break;

		case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
		{
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
		}
		break;

		case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
		{
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])	// VY-VX
				V[0xF] = 0; // there is a borrow
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
		}
		break;

		case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
		{
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
		}
		break;

		default:
			print("Unknown opcode [0x8000]: 0x%X", opcode);
		}
	}
	break;

	case 0xA000: // ANNN: Sets I to the address NNN
	{
		// Execute opcode
		I = opcode & 0x0FFF;
		pc += 2;
	}
	break;

	case 0xB000: // BNNN: Jumps to the address NNN plus V0
	{
		pc = (opcode & 0x0FFF) + V[0];
	}
	break;

	case 0xC000: // CXNN: Sets VX to a random number and NN
	{
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
	}
	break;

	case 0xD000: // DXYN: Draws a sprite at XY with a height of N
	{
		uchar x = V[(opcode & 0x0F00) >> 8];
		uchar y = V[(opcode & 0x00F0) >> 4];
		uchar height = (opcode & 0x000F);
		ushort pixels;

		V[0xF] = 0;
		for (uchar row = 0; row < height; row++)
		{
			pixels = memory[I + row];
			for (uchar col = 0; col < 8; col++)
			{
				if (pixels & (0x80 >> col))
				{
					// Set pixel
					if (gfx[x + col + ((y + row) * 64)] == 1)
						V[0xF] = 1;
					gfx[x + col + ((y + row) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;

	// 0xE000 opcodes
	case 0xE000:
	{
		switch (opcode & 0x00FF)
		{
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;

		default:
			print("Unknown opcode [0xE000]: 0x%X", opcode);
		}
	}
	break;

	// 0xF000 opcodes
	case 0xF000:
	{
		switch (opcode & 0x00FF)
		{
		case 0x0007: // FX07: Sets VX to the value of the delay timer
		{
			V[(opcode & 0x0F00) >> 8] = delayTimer;
			pc += 2;
		}

		case 0x000A: // FX0A: A key press is awaited, and then stored in VX
		{
			bool keyPress = false;
			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}

			// If we didn't received a keypress, skip this cycle and try again.
			if (!keyPress)
				return;

			pc += 2;
		}

		case 0x0015: // FX15: Sets the delay timer to VX
		{
			delayTimer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
		}
		break;

		case 0x0018: // FX15: Sets the sound timer to VX
		{
			soundTimer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
		}
		break;

		case 0x001E: // FX1E: Adds VX to I
		{
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
				V[0xF] = 1;
			else
				V[0xF] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
		}
		break;

		case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
		{
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
		}
		break;

		case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
		{
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
		}
		break;

		case 0x0055: // FX55: Stores V0 to VX in memory starting at address I
		{
			// Copy values
			for (uchar i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
				memory[I + i] = V[i];

			// Move memory address to I + x + 1
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
		}
		break;

		case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I
		{
			// Copy values
			for (uchar i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
				V[i] = memory[I + i];

			// Move memory address to I + x + 1
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
		}
		break;

		default:
			print("Unknown opcode [0xF000]: 0x%X", opcode);
		}
	}
	break;

	default:
		print("Unknown opcode: 0x%X", opcode);
	}

	// Update timers
	if (delayTimer > 0)
		delayTimer--;

	if (soundTimer > 0)
	{
		//if (soundTimer == 1)
		//	cout << ("BEEP!");
		soundTimer--;
	}
}
