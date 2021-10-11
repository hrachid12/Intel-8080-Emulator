
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
        case 0x56: printf("MOV     D,M"); break;
        case 0x57: printf("MOV     D,A"); break;
        case 0x58: printf("MOV     E,B"); break;
        case 0x59: printf("MOV     E,C"); break;
        case 0x5a: printf("MOV     E,D"); break;
        case 0x5b: printf("MOV     E,E"); break;
        case 0x5c: printf("MOV     E,H"); break;
        case 0x5d: printf("MOV     E,L"); break;
        case 0x5e: printf("MOV     E,M"); break;
        case 0x5f: printf("MOV     E,A"); break;
        case 0x60: printf("MOV     H,B"); break;
        case 0x61: printf("MOV     H,C"); break;
        case 0x62: printf("MOV     H,D"); break;
        case 0x63: printf("MOV     H,E"); break;
        case 0x64: printf("MOV     H,H"); break;
        case 0x65: printf("MOV     H,L"); break;
        case 0x66: printf("MOV     H,M"); break;
        case 0x67: printf("MOV     H,A"); break;
        case 0x68: printf("MOV     L,B"); break;
        case 0x69: printf("MOV     L,C"); break;
        case 0x6a: printf("MOV     L,D"); break;
        case 0x6b: printf("MOV     L,E"); break;
        case 0x6c: printf("MOV     L,H"); break;
        case 0x6d: printf("MOV     L,L"); break;
        case 0x6e: printf("MOV     L,M"); break;
        case 0x6f: printf("MOV     L,A"); break;
        case 0x70: printf("MOV     M,B"); break;
        case 0x71: printf("MOV     M,C"); break;
        case 0x72: printf("MOV     M,D"); break;
        case 0x73: printf("MOV     M,E"); break;
        case 0x74: printf("MOV     M,H"); break;
        case 0x75: printf("MOV     M,L"); break;
        case 0x76: printf("HLT"); break;
        case 0x77: printf("MOV     M,A"); break;
        case 0x78: printf("MOV     A,B"); break;
        case 0x79: printf("MOV     A,C"); break;
        case 0x7a: printf("MOV     A,D"); break;
        case 0x7b: printf("MOV     A,E"); break;
        case 0x7c: printf("MOV     A,H"); break;
        case 0x7d: printf("MOV     A,L"); break;
        case 0x7e: printf("MOV     A,M"); break;
        case 0x7f: printf("MOV     A,A"); break;
        case 0x80: printf("ADD     B"); break;      // A = A + B (A is the accumulator (used for arithmetic operations))
        case 0x81: printf("ADD     C"); break;
        case 0x82: printf("ADD     D"); break;
        case 0x83: printf("ADD     E"); break;
        case 0x84: printf("ADD     H"); break;
        case 0x85: printf("ADD     L"); break;
        case 0x86: printf("ADD     M"); break;
        case 0x87: printf("ADD     A"); break;
        case 0x88: printf("ADC     B"); break;      // A = A + B + CY
        case 0x89: printf("ADC     C"); break;
        case 0x8a: printf("ADC     D"); break;
        case 0x8b: printf("ADC     E"); break;
        case 0x8c: printf("ADC     H"); break;
        case 0x8d: printf("ADC     L"); break;
        case 0x8e: printf("ADC     M"); break;
        case 0x8f: printf("ADC     A"); break;
        case 0x90: printf("SUB     B"); break;      // A = A - B
        case 0x91: printf("SUB     C"); break;
        case 0x92: printf("SUB     D"); break;
        case 0x93: printf("SUB     E"); break;
        case 0x94: printf("SUB     H"); break;
        case 0x95: printf("SUB     L"); break;
        case 0x96: printf("SUB     M"); break;
        case 0x97: printf("SUB     A"); break;
        case 0x98: printf("SBB     B"); break;      // A = A - B - CY
        case 0x99: printf("SBB     C"); break;
        case 0x9a: printf("SBB     D"); break;
        case 0x9b: printf("SBB     E"); break;
        case 0x9c: printf("SBB     H"); break;
        case 0x9d: printf("SBB     L"); break;
        case 0x9e: printf("SBB     M"); break;
        case 0x9f: printf("SBB     A"); break;
        case 0xa0: printf("ANA     B"); break;      // A = A && B (Logical AND)
        case 0xa1: printf("ANA     C"); break;
        case 0xa2: printf("ANA     D"); break;
        case 0xa3: printf("ANA     E"); break;
        case 0xa4: printf("ANA     H"); break;
        case 0xa5: printf("ANA     L"); break;
        case 0xa6: printf("ANA     M"); break;
        case 0xa7: printf("ANA     A"); break;
        case 0xa8: printf("XRA     B"); break;      // A = A XOR B (Logical exclusive OR)
        case 0xa9: printf("XRA     C"); break;
        case 0xaa: printf("XRA     D"); break;
        case 0xab: printf("XRA     E"); break;
        case 0xac: printf("XRA     H"); break;
        case 0xad: printf("XRA     L"); break;
        case 0xae: printf("XRA     M"); break;
        case 0xaf: printf("XRA     A"); break;
    }

    printf("\n");
    return opbytes;
}