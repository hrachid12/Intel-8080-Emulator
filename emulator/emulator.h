#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct ConditionCodes {
	uint8_t		z:1;
	uint8_t		s:1;
	uint8_t		p:1;
	uint8_t		cy:1;
	uint8_t		ac:1;
	uint8_t		pad:3;
} ConditionCodes;

ConditionCodes CC_ZSPAC = {1,1,1,0,1};

typedef struct State8080 {
	uint8_t		a;
	uint8_t		b;
	uint8_t		c;
	uint8_t		d;
	uint8_t		e;
	uint8_t		h;
	uint8_t		l;
	uint16_t	sp;
	uint16_t	pc;
	uint8_t		*memory;
	struct ConditionCodes		cc;
	uint8_t		int_enable;
} State8080;


void UnimplementedInstruction(State8080* state)
{
	//pc will have advanced one, so undo that
	printf ("Error: Unimplemented instruction\n");
	state->pc--;
	Disassembler(state->memory, state->pc);
	printf("\n");
	exit(1);
}


void JMP (State8080* state, char *code) {
    // Create 16bit address from the opcodes
    // Left shift larger byte due to format being little endian
    // Jump to the 16bit address
    state->pc = (code[2] << 8) | code[1];
}

void CALL (State8080* state, char *code) {
    uint16_t ret = state->pc+2;

    //Save upper byte
    state->memory[state->sp-1] = (ret >> 8) & 0xff;

    // Save lower byte
    state->memory[state->sp-2] = (ret & 0xff);

    // Update stack pointer
    state->sp = state->sp - 2;

    // Create 16bit address from the codes
    // Leftshift larger byte due to format being little endian
    // Set pc to the target 16bit address
    state->pc = (code[2] << 8) | code[1];
}


void RET(State8080* state) {
    // Set pc to the 16bit address taken from the stack
    // Left shift the upper byte and use inclusive OR to create the
    // 16bit address
    state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);

    // Increment stack pointer
    state->sp += 2;
}


int Emulate8080(State8080* state)
{
	unsigned char *code = &state->memory[state->pc];

	// Disassemble8080(state->memory, state->pc);

	state->pc += 1;													// inc pc by 1

	switch(*code) {
	    case 0x00: UnimplementedInstruction(state); break;		//	NOP
        case 0x01: UnimplementedInstruction(state); break;		//  LXI     B, 16bit_data
        case 0x02: UnimplementedInstruction(state); break;		//	STAX    B
        case 0x03: UnimplementedInstruction(state); break;		//	INX     B
        case 0x04: UnimplementedInstruction(state); break;		//	INR     B
        case 0x05: UnimplementedInstruction(state); break;		//	DCR     B
        case 0x06: UnimplementedInstruction(state); break;		//	MVI     B, 8bit_data
        case 0x07: UnimplementedInstruction(state); break;		//	RLC
        case 0x08: UnimplementedInstruction(state); break;		//	NOP
        case 0x09: UnimplementedInstruction(state); break;		//	DAD     B
        case 0x0a: UnimplementedInstruction(state); break;		//	LDAX    B
        case 0x0b: UnimplementedInstruction(state); break;		//	DCX     B
        case 0x0c: UnimplementedInstruction(state); break;		//	INR     C
        case 0x0d: UnimplementedInstruction(state); break;		//	DCR     C
        case 0x0e: UnimplementedInstruction(state); break;		//  MVI     C, 8bit_data
        case 0x0f: UnimplementedInstruction(state); break;		//	RRC
        case 0x10: UnimplementedInstruction(state); break;		//	NOP
        case 0x11: UnimplementedInstruction(state); break;		//	LXI     D, 16bit_data
        case 0x12: UnimplementedInstruction(state); break;		//	STAX    D
        case 0x13: UnimplementedInstruction(state); break;		//	INX     D
        case 0x14: UnimplementedInstruction(state); break;		//	INR     D
        case 0x15: UnimplementedInstruction(state); break;		//	DCR     D
        case 0x16: UnimplementedInstruction(state); break;		//	MVI     D, 8bit_data
        case 0x17: UnimplementedInstruction(state); break;		//	RAL
        case 0x18: UnimplementedInstruction(state); break;		//	NOP
        case 0x19: UnimplementedInstruction(state); break;		//	DAD     D
        case 0x1a: UnimplementedInstruction(state); break;		//	LDAX    D
        case 0x1b: UnimplementedInstruction(state); break;		//	DCX     D
        case 0x1c: UnimplementedInstruction(state); break;		//	INR     E
        case 0x1d: UnimplementedInstruction(state); break;		//	DCR     E
        case 0x1e: UnimplementedInstruction(state); break;		//	MVI     E, 8bit_data
        case 0x1f: UnimplementedInstruction(state); break;		//	RAR
        case 0x20: UnimplementedInstruction(state); break;		//	NOP
        case 0x21: UnimplementedInstruction(state); break;		//	LXI     H, 16bit_data
        case 0x22: UnimplementedInstruction(state); break;		//	SHLD    address
        case 0x23: UnimplementedInstruction(state); break;		//	INX     H
        case 0x24: UnimplementedInstruction(state); break;		//	INR     H
        case 0x25: UnimplementedInstruction(state); break;		//	DCR     H
        case 0x26: UnimplementedInstruction(state); break;		//	MVI     H, #$%02x
        case 0x27: UnimplementedInstruction(state); break;		//	DAA
        case 0x28: UnimplementedInstruction(state); break;		//	NOP
        case 0x29: UnimplementedInstruction(state); break;		//	DAD     H
        case 0x2a: UnimplementedInstruction(state); break;		//	LHLD    address
        case 0x2b: UnimplementedInstruction(state); break;		//	DCX     H
        case 0x2c: UnimplementedInstruction(state); break;		//	INR     L
        case 0x2d: UnimplementedInstruction(state); break;		//	DCR     L
        case 0x2e: UnimplementedInstruction(state); break;		//	MVI     L, 8bit_data
        case 0x2f: UnimplementedInstruction(state); break;		//	CMA
        case 0x30: UnimplementedInstruction(state); break;		//	NOP
        case 0x31: UnimplementedInstruction(state); break;		//	LXI     SP, 16bit_data
        case 0x32: UnimplementedInstruction(state); break;		//	STA     address
        case 0x33: UnimplementedInstruction(state); break;		//	INX     SP
        case 0x34: UnimplementedInstruction(state); break;		//	INR     M
        case 0x35: UnimplementedInstruction(state); break;		//	DCR     M
        case 0x36: UnimplementedInstruction(state); break;		//	MVI     M, 8bit_data
        case 0x37: UnimplementedInstruction(state); break;		//	STC
        case 0x38: UnimplementedInstruction(state); break;		//	NOP
        case 0x39: UnimplementedInstruction(state); break;		//	DAD     SP
        case 0x3a: UnimplementedInstruction(state); break;		//	LDA     address
        case 0x3b: UnimplementedInstruction(state); break;		//	DCX     SP
        case 0x3c: UnimplementedInstruction(state); break;		//	INR     A
        case 0x3d: UnimplementedInstruction(state); break;		//	DCR     A
        case 0x3e: UnimplementedInstruction(state); break;		//	MVI     A, 8bit_data
        case 0x3f: UnimplementedInstruction(state); break;		//	CMC
        case 0x40: UnimplementedInstruction(state); break;		//	MOV     B, B
        case 0x41: UnimplementedInstruction(state); break;		//	MOV     B, C
        case 0x42: UnimplementedInstruction(state); break;		//	MOV     B, D
        case 0x43: UnimplementedInstruction(state); break;		//	MOV     B, E
        case 0x44: UnimplementedInstruction(state); break;		//	MOV     B, H
        case 0x45: UnimplementedInstruction(state); break;		//	MOV     B, L
        case 0x46: UnimplementedInstruction(state); break;		//	MOV     B, M
        case 0x47: UnimplementedInstruction(state); break;		//	MOV     B, A
        case 0x48: UnimplementedInstruction(state); break;		//	MOV     C, B
        case 0x49: UnimplementedInstruction(state); break;		//	MOV     C, C
        case 0x4a: UnimplementedInstruction(state); break;		//	MOV     C, D
        case 0x4b: UnimplementedInstruction(state); break;		//	MOV     C, E
        case 0x4c: UnimplementedInstruction(state); break;		//	MOV     C, H
        case 0x4d: UnimplementedInstruction(state); break;		//	MOV     C, L
        case 0x4e: UnimplementedInstruction(state); break;		//	MOV     C, M
        case 0x4f: UnimplementedInstruction(state); break;		//	MOV     C, A
        case 0x50: UnimplementedInstruction(state); break;		//	MOV     D, B
        case 0x51: UnimplementedInstruction(state); break;		//	MOV     D, C
        case 0x52: UnimplementedInstruction(state); break;		//	MOV     D, D
        case 0x53: UnimplementedInstruction(state); break;		//	MOV     D, E
        case 0x54: UnimplementedInstruction(state); break;		//	MOV     D, H
        case 0x55: UnimplementedInstruction(state); break;		//	MOV     D, L

        case 0x56: UnimplementedInstruction(state); break;		//	MOV     D, M
        case 0x57: UnimplementedInstruction(state); break;		//	MOV     D, A
        case 0x58: UnimplementedInstruction(state); break;		//	MOV     E, B
        case 0x59: UnimplementedInstruction(state); break;		//	MOV     E, C
        case 0x5a: UnimplementedInstruction(state); break;		//	MOV     E, D
        case 0x5b: UnimplementedInstruction(state); break;		//	MOV     E, E
        case 0x5c: UnimplementedInstruction(state); break;		//	MOV     E, H
        case 0x5d: UnimplementedInstruction(state); break;		//	MOV     E, L
        case 0x5e: UnimplementedInstruction(state); break;		//	MOV     E, M
        case 0x5f: UnimplementedInstruction(state); break;		//	MOV     E, A
        case 0x60: UnimplementedInstruction(state); break;		//	MOV     H, B
        case 0x61: UnimplementedInstruction(state); break;		//	MOV     H, C
        case 0x62: UnimplementedInstruction(state); break;		//	MOV     H, D
        case 0x63: UnimplementedInstruction(state); break;		//	MOV     H, E
        case 0x64: UnimplementedInstruction(state); break;		//	MOV     H, H
        case 0x65: UnimplementedInstruction(state); break;		//	MOV     H, L
        case 0x66: UnimplementedInstruction(state); break;		//	MOV     H, M
        case 0x67: UnimplementedInstruction(state); break;		//	MOV     H, A
        case 0x68: UnimplementedInstruction(state); break;		//	MOV     L, B
        case 0x69: UnimplementedInstruction(state); break;		//	MOV     L, C
        case 0x6a: UnimplementedInstruction(state); break;		//	MOV     L, D
        case 0x6b: UnimplementedInstruction(state); break;		//	MOV     L, E
        case 0x6c: UnimplementedInstruction(state); break;		//	MOV     L, H
        case 0x6d: UnimplementedInstruction(state); break;		//	MOV     L, L
        case 0x6e: UnimplementedInstruction(state); break;		//	MOV     L, M
        case 0x6f: UnimplementedInstruction(state); break;		//	MOV     L, A
        case 0x70: UnimplementedInstruction(state); break;		//	MOV     M, B
        case 0x71: UnimplementedInstruction(state); break;		//	MOV     M, C
        case 0x72: UnimplementedInstruction(state); break;		//	MOV     M, D
        case 0x73: UnimplementedInstruction(state); break;		//	MOV     M, E
        case 0x74: UnimplementedInstruction(state); break;		//	MOV     M, H
        case 0x75: UnimplementedInstruction(state); break;		//	MOV     M, L
        case 0x76: UnimplementedInstruction(state); break;		//	HLT
        case 0x77: UnimplementedInstruction(state); break;		//	MOV     M, A
        case 0x78: UnimplementedInstruction(state); break;		//	MOV     A, B
        case 0x79: UnimplementedInstruction(state); break;		//	MOV     A, C
        case 0x7a: UnimplementedInstruction(state); break;		//	MOV     A, D
        case 0x7b: UnimplementedInstruction(state); break;		//	MOV     A, E
        case 0x7c: UnimplementedInstruction(state); break;		//	MOV     A, H
        case 0x7d: UnimplementedInstruction(state); break;		//	MOV     A, L
        case 0x7e: UnimplementedInstruction(state); break;		//	MOV     A, M
        case 0x7f: UnimplementedInstruction(state); break;		//	MOV     A, A
        case 0x80: UnimplementedInstruction(state); break;		//	ADD     B
        case 0x81: UnimplementedInstruction(state); break;		//	ADD     C
        case 0x82: UnimplementedInstruction(state); break;		//	ADD     D
        case 0x83: UnimplementedInstruction(state); break;		//	ADD     E
        case 0x84: UnimplementedInstruction(state); break;		//	ADD     H
        case 0x85: UnimplementedInstruction(state); break;		//	ADD     L
        case 0x86: UnimplementedInstruction(state); break;		//	ADD     M
        case 0x87: UnimplementedInstruction(state); break;		//	ADD     A
        case 0x88: UnimplementedInstruction(state); break;		//	ADC     B
        case 0x89: UnimplementedInstruction(state); break;		//	ADC     C
        case 0x8a: UnimplementedInstruction(state); break;		//	ADC     D
        case 0x8b: UnimplementedInstruction(state); break;		//	ADC     E
        case 0x8c: UnimplementedInstruction(state); break;		//	ADC     H
        case 0x8d: UnimplementedInstruction(state); break;		//	ADC     L
        case 0x8e: UnimplementedInstruction(state); break;		//	ADC     M
        case 0x8f: UnimplementedInstruction(state); break;		//	ADC     A
        case 0x90: UnimplementedInstruction(state); break;		//	SUB     B
        case 0x91: UnimplementedInstruction(state); break;		//	SUB     C
        case 0x92: UnimplementedInstruction(state); break;		//	SUB     D
        case 0x93: UnimplementedInstruction(state); break;		//	SUB     E
        case 0x94: UnimplementedInstruction(state); break;		//	SUB     H
        case 0x95: UnimplementedInstruction(state); break;		//	SUB     L
        case 0x96: UnimplementedInstruction(state); break;		//	SUB     M
        case 0x97: UnimplementedInstruction(state); break;		//	SUB     A
        case 0x98: UnimplementedInstruction(state); break;		//	SBB     B
        case 0x99: UnimplementedInstruction(state); break;		//	SBB     C
        case 0x9a: UnimplementedInstruction(state); break;		//	SBB     D
        case 0x9b: UnimplementedInstruction(state); break;		//	SBB     E
        case 0x9c: UnimplementedInstruction(state); break;		//	SBB     H
        case 0x9d: UnimplementedInstruction(state); break;		//	SBB     L
        case 0x9e: UnimplementedInstruction(state); break;		//	SBB     M
        case 0x9f: UnimplementedInstruction(state); break;		//	SBB     A
        case 0xa0: UnimplementedInstruction(state); break;		//	ANA     B
        case 0xa1: UnimplementedInstruction(state); break;		//	ANA     C
        case 0xa2: UnimplementedInstruction(state); break;		//	ANA     D
        case 0xa3: UnimplementedInstruction(state); break;		//	ANA     E
        case 0xa4: UnimplementedInstruction(state); break;		//	ANA     H
        case 0xa5: UnimplementedInstruction(state); break;		//	ANA     L
        case 0xa6: UnimplementedInstruction(state); break;		//	ANA     M
        case 0xa7: UnimplementedInstruction(state); break; 		// 	ANA     A
        case 0xa8: UnimplementedInstruction(state); break;		//	XRA     B
        case 0xa9: UnimplementedInstruction(state); break;		//  XRA     C
        case 0xaa: UnimplementedInstruction(state); break;		//  XRA     D
        case 0xab: UnimplementedInstruction(state); break;		//  XRA     E
        case 0xac: UnimplementedInstruction(state); break;		//  XRA     H
        case 0xad: UnimplementedInstruction(state); break;		//  XRA     L
        case 0xae: UnimplementedInstruction(state); break;		//  XRA     M
        case 0xaf: UnimplementedInstruction(state); break;		//  XRA     A

        case 0xb0: UnimplementedInstruction(state); break;		//  ORA     B
        case 0xb1: UnimplementedInstruction(state); break;		//  ORA     C
        case 0xb2: UnimplementedInstruction(state); break;		//  ORA     D
        case 0xb3: UnimplementedInstruction(state); break;		//  ORA     E
        case 0xb4: UnimplementedInstruction(state); break;		//  ORA     H
        case 0xb5: UnimplementedInstruction(state); break;		//  ORA     L
        case 0xb6: UnimplementedInstruction(state); break;		//  ORA     M
        case 0xb7: UnimplementedInstruction(state); break;		//  ORA     A
        case 0xb8: UnimplementedInstruction(state); break;		//  CMP     B
        case 0xb9: UnimplementedInstruction(state); break;		//  CMP     C
        case 0xba: UnimplementedInstruction(state); break;		//  CMP     D
        case 0xbb: UnimplementedInstruction(state); break;		//  CMP     E
        case 0xbc: UnimplementedInstruction(state); break;		//  CMP     H
        case 0xbd: UnimplementedInstruction(state); break;		//  CMP     L
        case 0xbe: UnimplementedInstruction(state); break;		//  CMP     M
        case 0xbf: UnimplementedInstruction(state); break;		//  CMP     A
        case 0xc0: UnimplementedInstruction(state); break;		//  RNZ
        case 0xc1: UnimplementedInstruction(state); break;		//  POP     B
        case 0xc2:
            //  JNZ address
            if (0 == state->cc.z) {
                JMP(state, code);
            } else {
                state->pc += 2;
            }
            break;
        case 0xc3:
            //  JMP address
            JMP(state, code);
            break;
        case 0xc4: UnimplementedInstruction(state); break;		//  CNZ     address
        case 0xc5: UnimplementedInstruction(state); break;		//  PUSH    B
        case 0xc6: UnimplementedInstruction(state); break;		//  ADI     8bit_data
        case 0xc7: UnimplementedInstruction(state); break;		//  RST     0
        case 0xc8: UnimplementedInstruction(state); break;		//  RZ
        case 0xc9:
            // RET
            RET(state);
            break;
        case 0xca:
            //  JZ address
            if (1 == state->cc.z) {
                JMP(state, code);
            } else {
                state->pc += 2;
            }
            break;
        case 0xcb: UnimplementedInstruction(state); break;		//  NOP
        case 0xcc:
            // CZ addr

            if (1 == state->cc.z) {
                CALL(state, code);
            } else {
                state->pc += 2
            }
            break;
        case 0xcd:
        case 0xcd:
            //CALL address
            CALL(state, code);
            break;
        case 0xce: UnimplementedInstruction(state); break;		//  ACI     8bit_data
        case 0xcf: UnimplementedInstruction(state); break;		//  RST     1
        case 0xd0: UnimplementedInstruction(state); break;		//  RNC
        case 0xd1: UnimplementedInstruction(state); break;		//  POP     D
        case 0xd2:
            //  JNC address
            if (0 == state->cc.cy) {
                JMP(state, code);
            } else {
                state->pc += 2;
            }
            break;
        case 0xd3: UnimplementedInstruction(state); break;		//  OUT     output_device_num
        case 0xd4: UnimplementedInstruction(state); break;		//  CNC     address
        case 0xd5: UnimplementedInstruction(state); break;		//  PUSH    D
        case 0xd6: UnimplementedInstruction(state); break;		//  SUI     8bit_data
        case 0xd7: UnimplementedInstruction(state); break;		//  RST     2
        case 0xd8: UnimplementedInstruction(state); break;		//  RC
        case 0xd9: UnimplementedInstruction(state); break;		//  NOP
        case 0xda:
            //  JC address
            if (1 == state->cc.cy) {
                JMP(state, code);
            } else {
                state->pc += 2;
            }
            break;
        case 0xdb: UnimplementedInstruction(state); break;		//  IN      input_device_num
        case 0xdc: UnimplementedInstruction(state); break;		//  CC      address
        case 0xdd: UnimplementedInstruction(state); break;		//  NOP
        case 0xde: UnimplementedInstruction(state); break;		//  SBI     8bit_data
        case 0xdf: UnimplementedInstruction(state); break;		//  RST     3
        case 0xe0: UnimplementedInstruction(state); break;		//  RPO
        case 0xe1: UnimplementedInstruction(state); break;		//  POP     H
        case 0xe2:
            //  JPO address
            if (0 == state->cc.p) {
                JMP(state, code);
            } else {
                state->pc += 2
            }
            break;
        case 0xe3: UnimplementedInstruction(state); break;		//  XTHL
        case 0xe4: UnimplementedInstruction(state); break;		//  CPO     address
        case 0xe5: UnimplementedInstruction(state); break;		//  PUSH    H
        case 0xe6: UnimplementedInstruction(state); break;		//  ANI     8bit_data
        case 0xe7: UnimplementedInstruction(state); break;		//  RST     4
        case 0xe8: UnimplementedInstruction(state); break;		//  RPE
        case 0xe9: UnimplementedInstruction(state); break;		//  PCHL
        case 0xea:
            // JPE address
            if (1 == state->cc.p) {
                JMP(state, code);
            } else {
                state->pc += 2;
            }
            break;
        case 0xeb: UnimplementedInstruction(state); break;		//  XCHG
        case 0xec: UnimplementedInstruction(state); break;		//  CPE     address
        case 0xed: UnimplementedInstruction(state); break;		//  NOP
        case 0xee: UnimplementedInstruction(state); break;		//  XRI     8bit_data
        case 0xef: UnimplementedInstruction(state); break;		//  RST     5
        case 0xf0: UnimplementedInstruction(state); break;		//  RP
        case 0xf1: UnimplementedInstruction(state); break;		//  POP     PSW
        case 0xf2:
            //  JP address
            if (0 == state->cc.s) {
                JMP(state, code);
            } else {
                state->pc += 2;
            }
            break;
        case 0xf3: UnimplementedInstruction(state); break;		//  DI
        case 0xf4: UnimplementedInstruction(state); break;		//  CP      address
        case 0xf5: UnimplementedInstruction(state); break;		//  PUSH    PSW
        case 0xf6: UnimplementedInstruction(state); break;		//  ORI     8bit_data
        case 0xf7: UnimplementedInstruction(state); break;		//  RST     6
        case 0xf8: UnimplementedInstruction(state); break;		//  RM
        case 0xf9: UnimplementedInstruction(state); break;		//  SPHL
        case 0xfa:
            //  JM address
            if (1 == state->cc.s) {
                JMP(state, code);
            } else {
                state->pc += 2;
            }
            break;
        case 0xfb: UnimplementedInstruction(state); break;		//  EI
        case 0xfc: UnimplementedInstruction(state); break;		//  CM      address
        case 0xfd: UnimplementedInstruction(state); break;		//  NOP
        case 0xfe: UnimplementedInstruction(state); break;		//  CPI     8bit_data
        case 0xff: UnimplementedInstruction(state); break;		//  RST     7
	}
	// Print out flag condition codes and address data here to keep track of them after each instruction for debugging
	/*
	printf("\t");
	printf("%c", state->cc.z ? 'z' : '.');
	printf("%c", state->cc.s ? 's' : '.');
	printf("%c", state->cc.p ? 'p' : '.');
	printf("%c", state->cc.cy ? 'c' : '.');
	printf("%c  ", state->cc.ac ? 'a' : '.');
	printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->a, state->b, state->c,
				state->d, state->e, state->h, state->l, state->sp);
	*/
	return 0;
}
