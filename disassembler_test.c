/* Tests our Disassembler using the Space Invaders ROM */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "./disassembler/disassembler.c"

int main (int argc, char**argv)    
   {    
    FILE *f= fopen("./ROMs/invaders", "rb");    
    if (f==NULL)    
    {    
        printf("error: Couldn't open file\n");    
        exit(1);    
    }    

    //Get the file size and read it into a memory buffer    
    fseek(f, 0L, SEEK_END);    
    int fsize = ftell(f);    
    fseek(f, 0L, SEEK_SET);    

    unsigned char *buffer=malloc(fsize);    

    fread(buffer, fsize, 1, f);    
    fclose(f);    

    int pc = 0;                         // program counter

    while (pc < fsize)                  // loop through buffer and disassemble one instruction at a time    
    {    
        pc += Disassemble8080(buffer, pc); // increment pc by the number of bytes used by the instruction
    }    
    return 0;    
   } 
      