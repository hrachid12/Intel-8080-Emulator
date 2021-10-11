
/* CS 467 project for emulating Intel 8080 and playing Space Invaders ROM */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int Disassembler(unsigned char *buffer, int pc) {
    unsigned char *code = &buffer[pc];
    int opbytes = 1;
    printf("%04x ", pc);

    switch (*code) {
        //add cases
    }

    printf("\n");
    return opbytes;
}