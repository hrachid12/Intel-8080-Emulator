/* CS 467 project for emulating Intel 8080 and playing Space Invaders ROM */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>
#include <time.h>

#include "./disassembler/disassembler.h"
#include "./emulator/emulator.h"

//Global variables
SDL_Surface *surface;
int resizef;
SDL_Window *window;
SDL_Surface *winsurface;
mem_t *ram;

#define HEIGHT 256
#define WIDTH 224

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

State8080* Init8080(void)
{
	State8080* state = calloc(1,sizeof(State8080));
	state->memory = malloc(0x10000);  //16K

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
	int lastInterrupt = 0;
	time_t seconds;
     
	State8080* state = Init8080();

	ReadFileIntoMemoryAt(state, "./ROMs/invaders.h", 0);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.e", 0x1800);

	while (done == 0)
	{
		done = Emulate8080(state);
		seconds = time(NULL);
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
		draw_video_ram(state);
	}
	return 0;
}
