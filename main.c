/* CS 467 project for emulating Intel 8080 and playing Space Invaders ROM */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <sys/time.h>
#include <time.h>

#include "./disassembler/disassembler.h"
#include "./emulator/emulator.h"

//Global variables
SDL_Surface *surface;
int resizef;
SDL_Window *window;
SDL_Surface *winsurface;
mem_t *ram;

uint8_t     shift0;         // Least significant byte of Space Invader's external shift hardware
uint8_t     shift1;         // Most significant byte of Space Invaders external shift hardware
uint8_t     shift_offset;   // offset for external shift hardware

#define HEIGHT 256
#define WIDTH 224

double GetTimeStamp() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return ((double)time.tv_sec * 1E6) + ((double)time.tv_usec);
    }

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

void draw_video_ram(State8080* state) {
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

    // if (resizef) {
    winsurface = SDL_GetWindowSurface(window);
    // }

    SDL_BlitScaled(surface, NULL, winsurface, NULL);

    // Update window
    if (SDL_UpdateWindowSurface(window)) {
        puts(SDL_GetError());
    }
}

uint8_t handleSpaceInvadersIN(uint8_t port)
{
    unsigned char a;
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
                uint16_t v = (shift1 << 8) | shift0;
                a = ((v >> (8-shift_offset)) & 0xff);
            }
                break;
    }
    return a;
}

void handleSpaceInvadersOUT(uint8_t port, uint8_t value)
{
    switch(port)
    {
        case 2: // sets the shift amount
                shift_offset = value & 0x7;
                break;
        case 4: // sets the data in the shift registers
                shift0 = shift1;
                shift1 = value;
                break;
    }
}

State8080* Init8080(void)
{
	State8080* state = calloc(1,sizeof(State8080));
	state->memory = malloc(0x10000);  //16K

	state->done = 0;

	// SDL Init returns zero on success
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
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

int main (int argc, char**argv)
{
	int done = 0;
	int vblankcycles = 0;
	//time_t seconds;
	double now;                  // current time
	double lastTime = 0.;        // last time recorded
	double nextInterrupt;        // time next interrupt should fire
	int whichInterrupt;          // which interrupt should be sent
     
	State8080* state = Init8080();

	ReadFileIntoMemoryAt(state, "./ROMs/invaders.h", 0);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.e", 0x1800);

	while (1)
	{
        now = GetTimeStamp();
        
        if (lastTime == 0.){
            lastTime = now;
            nextInterrupt = lastTime + 16000.0;
            whichInterrupt = 1;
        }

        if ((state->int_enable) && (now > nextInterrupt))
        {
            if (whichInterrupt == 1)
            {
                GenerateInterrupt(state, 1);
                whichInterrupt = 2;
            }
            else
            {
                GenerateInterrupt(state, 2);
                whichInterrupt = 1;
            }
            lastTime = now;
            nextInterrupt = now + 8000.0;
        }

        double sinceLast = now - lastTime;
        int cycles_to_catch_up = 2 * sinceLast;
        int cycles = 0;

        while (cycles_to_catch_up > cycles)
        {
            unsigned char *op;
            op = &state->memory[state->pc];
            if (*op == 0xdb) // machine IN instruction
            {
                state->a = handleSpaceInvadersIN(op[1]);
                state->pc += 2;
                cycles += 3;
            }
            else if (*op == 0xd3) // machine OUT instruction
            {
                handleSpaceInvadersOUT(op[1], state->a);
                state->pc+=2;
                cycles+=3;
            }
            else
            {
                cycles += Emulate8080(state);
								state->done += 1;
            }            
        }
        lastTime = GetTimeStamp();
        /*

		//printf("Time is %d - %d = %d", seconds, lastInterrupt, seconds - lastInterrupt);
		if (seconds - lastInterrupt > 1.0/60.0)  //1/60 second has elapsed    
        {    
			printf("Interrupt is %d\n", state->int_enable);
            //only do an interrupt if they are enabled    
            if (state->int_enable)    
            {    
				printf("Test!");
                GenerateInterrupt(state, 2);    //interrupt 2    

                //Save the time we did this    
                lastInterrupt = seconds;    
            }    
        }
        */
		draw_video_ram(state);
	}
	return 0;
}
