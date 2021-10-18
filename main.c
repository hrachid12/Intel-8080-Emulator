/* CS 467 project for emulating Intel 8080 and playing Space Invaders ROM */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "./disassembler/disassembler.c"
#include "./emulator/emulator.c"

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

State8080* Init8080(void)
{
	State8080* state = calloc(1,sizeof(State8080));
	state->memory = malloc(0x10000);  //16K
	return state;
}

int main (int argc, char**argv)
{
	int done = 0;
	int vblankcycles = 0;
	State8080* state = Init8080();

	ReadFileIntoMemoryAt(state, "./ROMs/invaders.h", 0);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "./ROMs/invaders.e", 0x1800);
	
	while (done == 0)
	{
		done = Emulate8080Op(state);
	}
	return 0;
}
