/* CS 467 project for emulating Intel 8080 and playing Space Invaders ROM */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#ifdef _WIN32
    #include <SDL.h>
#elif __APPLE__
    #include <SDL2/SDL.h>
#endif

#include "./disassembler/disassembler.h"
#include "./emulator/emulator.h"

//Global variables
SDL_Surface *surface;
int resizef;
SDL_Window *window;
SDL_Surface *winsurface;
mem_t *ram;

static uint16_t shift_register;
uint8_t     shift_offset;   // offset for external shift hardware

#define HEIGHT 256
#define WIDTH  224

#define FRAMERATE         (1000.0 / 60.0)   // ms per frame
#define CYCLES_PER_MS      2000             // 8080 runs at 2 Mhz
#define CYCLES_PER_FRAME  (CYCLES_PER_MS * FRAMERATE)

void ReadFileIntoMemoryAt(State8080* state, char* filename, uint32_t offset)
{
	FILE *f= fopen(filename, "rb");
	if (f==NULL)
	{
		printf("error: Couldn't open %s\n", filename);
		exit(1);
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);
	
	uint8_t *buffer = &state->memory[offset];
	fread(buffer, fsize, 1, f);
	fclose(f);
}

void DrawVideoRAM(State8080* state) {
    uint32_t *pix = surface->pixels;

    int i = 0x2400;  // Start of Video RAM
    for (int col = 0; col < WIDTH; col ++) {
        for (int row = HEIGHT; row > 0; row -= 8) {
            for (int j = 0; j < 8; j++) {
                int idx = (row - j) * WIDTH + col;

                if (state->memory[i] & 1 << j) {
                    pix[idx] = 0xFFFFFF;
                } else {
                    pix[idx] = 0x000000;
                }
            }
            i++;
        }
    }

    if (resizef) {
    winsurface = SDL_GetWindowSurface(window);
    }

    SDL_BlitScaled(surface, NULL, winsurface, NULL);

    // Update window
    if (SDL_UpdateWindowSurface(window)) {
        puts(SDL_GetError());
    }
}

uint8_t HandleSpaceInvadersIN(uint8_t port)
{
    // returns value to be put into state->a
    unsigned char a = 0;
    switch(port)
    {
        case 0:
                return 1;
                break;
        case 1: 
                return 0;
                break;
        case 3: // returns data shifted by the shift amount
            {
                a = shift_register >> (8 - shift_offset);;
            }
                break;
    }
    return a;
}

void HandleSpaceInvadersOUT(uint8_t port, uint8_t value)
{
    switch(port)
    {
        case 2: // sets the shift amount
                shift_offset = value;
                break;
        case 4: // sets the data in the shift registers
                shift_register = (value << 8) | (shift_register >> 8);
                break;
    }
}

void HandleInput() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT){
            exit(0);
        }
        if (ev.type == SDL_KEYDOWN){
            exit(0);
        }
    }  
}

State8080* Init8080(void)
{
	State8080* state = calloc(1,sizeof(State8080));
	state->memory = malloc(0x10000);  //16K

	// SDL Init returns zero on success
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        exit(1);
    }

	// Creates a window for the game
    window = SDL_CreateWindow(
            "Space Invaders",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            2*WIDTH, 2*HEIGHT,
            SDL_WINDOW_RESIZABLE
            );

    if (!window) {
        puts("Failed to create window");
        exit(1);
    }

    // Get surface
    winsurface = SDL_GetWindowSurface(window);
    if (!winsurface) {
        puts("Failed to get surface");
        exit(1);
    }

    // Handle resize events
    //SDL_AddEventWatch(HandleResize, NULL);

    // Create backbuffer surface
    surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);

	return state;
}

// Helper function to compare our emulator with a working one for debugging
int CompareEmulators(State8080* state, State8080* state2)
{
    int result = 0;
    if (state->a != state2->a)
    {
        printf("Error: Expected A to be %02x, was %02x\n", state2->a, state->a);
        result = 1;
    }
    if (state->b != state2->b)
    {
        printf("Error: Expected B to be %02x, was %02x\n", state2->b, state->b);
        result = 1;
    }
    if (state->c != state2->c)
    {
        printf("Error: Expected C to be %02x, was %02x\n", state2->c, state->c);
        result = 1;
    }
    if (state->d != state2->d)
    {
        printf("Error: Expected D to be %02x, was %02x\n", state2->d, state->d);
        result = 1;
    }
    if (state->e != state2->e)
    {
        printf("Error: Expected E to be %02x, was %02x\n", state2->e, state->e);
        result = 1;
    }
    if (state->h != state2->h)
    {
        printf("Error: Expected H to be %02x, was %02x\n", state2->h, state->h);
        result = 1;
    }
    if (state->l != state2->l)
    {
        printf("Error: Expected L to be %02x, was %02x\n", state2->l, state->l);
        result = 1;
    }
    if (state->sp != state2->sp)
    {
        printf("Error: Expected SP to be %04x, was %04x\n", state->sp, state->sp);
        result = 1;
    }
    if (state->pc != state2->pc)
    {
        printf("Error: Expected PC to be %04x, was %04x\n", state2->pc, state->pc);
        result = 1;
    }
    if (state->cc.cy != state2->cc.cy)
    {
        printf("Error: Expected CYF to be %02x, was %02x\n", state2->cc.cy, state->cc.cy);
        result = 1;
    }
    if (state->cc.z != state2->cc.z)
    {
        printf("Error: Expected ZF to be %02x, was %02x\n", state2->cc.z, state->cc.z);
        result = 1;
    }
    if (state->cc.s != state2->cc.s)
    {
        printf("Error: Expected SF to be %02x, was %02x\n", state2->cc.s, state->cc.s);
        result = 1;
    }
    if (state->cc.p != state2->cc.p)
    {
        printf("Error: Expected PF to be %02x, was %02x\n", state2->cc.p, state->cc.p);
        result = 1;
    }
    return result;
}

int main (int argc, char**argv)
{     
	State8080* state = Init8080();

	ReadFileIntoMemoryAt(state, "./ROMs/invaders.h", 0);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.e", 0x1800);

    uint32_t lastTime = SDL_GetTicks();
	while (1)
	{
        unsigned char *op;
        int cycles = 0;
        if (SDL_GetTicks() - lastTime >= FRAMERATE)
        {
            lastTime = SDL_GetTicks();
            while (cycles < CYCLES_PER_FRAME / 2)
            {
                op = &state->memory[state->pc];
                if (*op == 0xdb) // machine IN instruction
                {
                    state->a = HandleSpaceInvadersIN(op[1]);
                    state->pc += 2;
                    cycles += 3;
                }
                else if (*op == 0xd3) // machine OUT instruction
                {
                    HandleSpaceInvadersOUT(op[1], state->a);
                    state->pc+=2;
                    cycles+=3;
                }
                else
                {   
                    cycles += Emulate8080(state);
                }
            }

            if (state->int_enable)
            {
                GenerateInterrupt(state, 1);
            }

            HandleInput();
            DrawVideoRAM(state);

            while (cycles < CYCLES_PER_FRAME)
            {
                op = &state->memory[state->pc];
                if (*op == 0xdb) // machine IN instruction
                {
                    state->a = HandleSpaceInvadersIN(op[1]);
                    state->pc += 2;
                    cycles += 3;
                }
                else if (*op == 0xd3) // machine OUT instruction
                {
                    HandleSpaceInvadersOUT(op[1], state->a);
                    state->pc+=2;
                    cycles+=3;
                }
                else
                {   
                    cycles += Emulate8080(state);
                }
            }

            if (state->int_enable) 
            {
                GenerateInterrupt(state, 2);
            }
        }
	}
	return 0;
}
