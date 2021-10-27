/* CS 467 project for emulating Intel 8080 and playing Space Invaders ROM */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "./disassembler/disassembler.h"
#include "./emulator/emulator.h"

void PrintReg(State8080* state) {
    printf("\t");
    printf("%c", state->cc.z ? 'z' : '.');
    printf("%c", state->cc.s ? 's' : '.');
    printf("%c", state->cc.p ? 'p' : '.');
    printf("%c", state->cc.cy ? 'c' : '.');
    printf("%c  ", state->cc.ac ? 'a' : '.');
    printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->a, state->b, state->c,
           state->d, state->e, state->h, state->l, state->sp);
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

    state->memory[0] = 0xa8;

    state->a = 0xFF;
    state->b = 0xff;

    PrintReg(state);
    done = Emulate8080(state);
    PrintReg(state);

    return 0;
}
