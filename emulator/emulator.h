#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define  ADD   0
#define  SUB   1

#define NO_CARRY  0
#define CARRY     1

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

int ParityCheck(uint8_t value)
{
  // Checks if passed value has even or odd parity
  uint8_t count = 0;
  uint8_t check = 0x01;
  for (int i = 0; i < 8; i++)
  {
    if(value & check){count += 1;}
    check = check << 1;
  }
  // 8080 sets with even parity
  if (count % 2 == 0)
  {
    return 1;
  }
  return 0;
}

void HandleZSP_Flags(State8080* state, uint16_t result)
{
  // Handle zero flag
  if ((result & 0xff) == 0) { state->cc.z = 1; }
  else { state->cc.z = 0; }

  // Handle sign flag
  if (result & 0x80) { state->cc.s = 1; }
  else { state->cc.s = 0; }

  // Handle parity flag
  state->cc.p = ParityCheck(result & 0xff);
  return;
}

void Arithmetic(State8080* state, uint8_t operand, uint8_t operation, uint8_t carry)
{
  // Handles ADD, ADI, ADC, ACI, SUB, SUI, SBB, SBI instructions
  uint16_t result;
  // Handle operations that use carry bit
  if (carry)
  {
    operand += state->cc.cy;
  }

  // Addition
  if (operation == ADD)
  {
    result = state->a + operand;
  }
  // Subtraction
  else if (operation == SUB)
  {
    result = state->a + (uint8_t)(~operand + 1); // add two's complement of the operand
  }

  // Handle Zero, Sign, and Parity flags
  HandleZSP_Flags(state, result);

  // Handle carry flag
  if (result > 0xff) { state->cc.cy = 1; }
  else { state->cc.cy = 0; }
  if (operation == 1) { state->cc.cy ^= 1; }  // carry works opposite in subtraction, so flip bit

  // Store result in A
  state->a = (result & 0xff);
  return;
}

void DAD(State8080 *state, uint32_t reg_pair)
{
  // Add register pair to HL (16 bit add)
  uint32_t hl = (state->h << 8 | state->l);
  uint32_t result = reg_pair += hl;

  // Handle carry flag
  if (result & 0x10000) { state->cc.cy = 1; }
  else { state->cc.cy = 0; }
  
  // Store results back in h and l
  state->l = (uint8_t)result & 0xff;
  state->h = (uint8_t)(result >> 8) & 0xff;
  return;
}

void INR(State8080 *state, uint8_t *reg)
{
  // Increments register and handles flags
  *reg += 0x01;
  HandleZSP_Flags(state, *reg);
  return;
}

void DCR(State8080 *state, uint8_t *reg)
{
  // Decrements register and handles flags
  *reg -= 0x01;
  HandleZSP_Flags(state, *reg);
  return;
}

void MOV(uint8_t *reg, uint8_t value)
{
  // Moves a value into a specified register
  *reg = value;
  return;
}

void JMP (State8080* state, unsigned char *code) {
    // Create 16bit address from the opcodes
    // Left shift larger byte due to format being little endian
    // Jump to the 16bit address
    state->pc = (code[2] << 8) | code[1];
}

void CALL (State8080* state, unsigned char *code) {
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

void RST (State8080* state, uint8_t num) {
    uint16_t ret = state->pc+2;

    //Save upper byte
    state->memory[state->sp-1] = (ret >> 8) & 0xff;

    // Save lower byte
    state->memory[state->sp-2] = (ret & 0xff);

    // Update stack pointer
    state->sp = state->sp - 2;

    // Create 16bit address from the number provided
    // Leftshift number by three to match the required placement
    // as per the Intel 8080 programming manual (page 37)
    // Set pc to the target 16bit address
    state->pc = (num << 3) | 0x0000;
}


void RET(State8080* state) {
    // Set pc to the 16bit address taken from the stack
    // Left shift the upper byte and use inclusive OR to create the
    // 16bit address
    state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);

    // Increment stack pointer
    state->sp += 2;
}

void UnimplementedInstruction(State8080* state)
{
	// Print error along with associated instruction
	printf ("Error: Unimplemented instruction\n");
	state->pc--;
	Disassembler(state->memory, state->pc);
	printf("\n");
	exit(1);
}

int Emulate8080(State8080* state)
{
	unsigned char *code = &state->memory[state->pc];

	Disassembler(state->memory, state->pc);

	state->pc += 1;				// inc pc by 1 since every instruction takes at least 1 byte

	switch(*code) {
				case 0x00: break;		                                                    //	NOP
        case 0x01:                                                              //  LXI     BC, 16bit_data
                  {
                    state->b = code[2];
                    state->c = code[1];
                    state->pc += 2;
                    break;
                  }
        case 0x02:                                        		                  //	STAX    BC
        {
          // Stores accumulator in memory pointed to by rp BC
          uint16_t mem_reference = state->b << 8 | state->c;
          state->memory[mem_reference] = state->a;
          break;
        }
        case 0x03:                                                              //	INX     BC
                  {
                    state->c += 0x01; 
                    if ((state->c & 0xff) == 0){
                      state->b += 0x01;
                    }
                    break;
                  }
        case 0x04: INR(state, &state->b); break;                                //  INR     B
        case 0x05: DCR(state, &state->b); break;                                //  DCR     B
        case 0x06: MOV(&state->b, code[1]); state->pc += 1; break;              //	MVI     B, 8bit_data
        case 0x07: UnimplementedInstruction(state); break;	                  	//	RLC
        case 0x08: break;		                                                    //	NOP
        case 0x09: DAD(state, (uint32_t)(state->b << 8 | state->c)); break;     //  DAD     BC
        case 0x0a:                                                              //	LDAX    BC
        {
          // Loads accumulator with value stored in memory pointed to by rp BC
          uint16_t mem_reference = state->b << 8 | state->c;
          state->a = state->memory[mem_reference];
          break;
        }
        case 0x0b:                                                              //	DCX     BC
                  {
                    state->c -= 0x01;
                    if ((state->c & 0xff) == 0xff){
                      state->b -= 0x01;
                    }
                    break;
                  }
        case 0x0c: INR(state, &state->c); break;                                //  INR     C
        case 0x0d: DCR(state, &state->c); break;                                //  DCR     C
        case 0x0e: MOV(&state->c, code[1]); state->pc += 1; break;              //  MVI     C, 8bit_data
        case 0x0f: UnimplementedInstruction(state); break;		                  //	RRC
        case 0x10: break;		                                                    //	NOP
        case 0x11:                                                              //  LXI     D, 16bit_data
                  {
                    state->d = code[2];
                    state->e = code[1];
                    state->pc += 2;
                    break;
                  }
        case 0x12:                                         		                  //	STAX    DE
                  {
                    // Stores accumulator in memory pointed to by reg pair DE
                    uint16_t mem_reference = state->d << 8 | state->e;
                    state->memory[mem_reference] = state->a;
                    break;
                  }
        case 0x13:                                                              //	INX     DE
                  {
                    state->e += 0x01; 
                    if ((state->e & 0xff) == 0){
                      state->d += 0x01;
                    }
                    break;
                  }
        case 0x14: INR(state, &state->d); break;                                //  INR     D
        case 0x15: DCR(state, &state->d); break;                                //  DCR     D
        case 0x16: MOV(&state->d, code[1]); state->pc += 1; break;		          //	MVI     D, 8bit_data
        case 0x17: UnimplementedInstruction(state); break;		                  //	RAL
        case 0x18: break;		                                                    //	NOP
        case 0x19: DAD(state, (uint32_t)(state->d << 8 | state->e)); break;     //  DAD     DE
        case 0x1a:                                          	                  //	LDAX    DE
                  {
                    // Loads accumulator with value stored in memory pointed to by rp DE
                    uint16_t mem_reference = state->d << 8 | state->e;
                    state->a = state->memory[mem_reference];
                    break;
                  }
        case 0x1b:                                                              //	DCX     DE
                  {
                    state->e -= 0x01;
                    if ((state->e & 0xff) == 0xff){
                      state->d -= 0x01;
                    }
                    break;
                  }
        case 0x1c: INR(state, &state->e); break;                                //  INR     E
        case 0x1d: DCR(state, &state->e); break;                                //  DCR     E
        case 0x1e: MOV(&state->e, code[1]); state->pc += 1; break;		          //	MVI     E, 8bit_data
        case 0x1f: UnimplementedInstruction(state); break;		                  //	RAR
        case 0x20: break;		                                                    //	NOP
        case 0x21:                                                              //  LXI     H, 16bit_data
                  {
                    state->h = code[2];
                    state->l = code[1];
                    state->pc += 2;
                    break;
                  }
        case 0x22: UnimplementedInstruction(state); break;		                  //	SHLD    address
        case 0x23:                                                              //	INX     HL
                  {
                    state->l += 0x01; 
                    if ((state->l & 0xff) == 0){
                      state->h += 0x01;
                    }
                    break;
                  }
        case 0x24: INR(state, &state->h); break;                                //  INR     H
        case 0x25: DCR(state, &state->h); break;                                //  DCR     H
        case 0x26: MOV(&state->h, code[1]); state->pc += 1; break;		          //	MVI     H, 8bit_data
        case 0x27: UnimplementedInstruction(state); break;		                  //	DAA
        case 0x28: break;		                                                    //	NOP
        case 0x29: DAD(state, (uint32_t)(state->h << 8 | state->l)); break;     //  DAD     HL
        case 0x2a: UnimplementedInstruction(state); break;		                  //	LHLD    address
        case 0x2b:                                                              //	DCX     HL
                  {
                    state->l -= 0x01;
                    if ((state->l & 0xff) == 0xff){
                      state->h -= 0x01;
                    }
                    break;
                  }
        case 0x2c: INR(state, &state->l); break;                                //  INR     L
        case 0x2d: DCR(state, &state->l); break;                                //  DCR     L
        case 0x2e: MOV(&state->l, code[1]); state->pc+=1; break;		            //	MVI     L, 8bit_data
        case 0x2f: UnimplementedInstruction(state); break;		                  //	CMA
        case 0x30: break;		                                                    //	NOP
        case 0x31:                                                              //  LXI     SP, 16bit_data
                  {
                    uint16_t value = code[2] << 8 | code[1];
                    state->sp = value;
                    state->pc += 2;
                    break;
                  }
        case 0x32:                                        		                  //	STA     address
                  {
                    // Stores accumulator in memory at passed addr
                    uint16_t mem_reference = code[2] << 8 | code[1];
                    state->memory[mem_reference] = state->a;
                    state->pc += 2;
                    break;
                  }
        case 0x33: state->sp += 0x01; break;		                                //	INX     SP
        case 0x34:                                                              //	INR     M
                  {
                    u_int16_t mem_reference = (state->h << 8 | state->l);
                    INR(state, &state->memory[mem_reference]);
                    break;
                  }
        case 0x35:                                                              //	DCR     M
                  {
                    u_int16_t mem_reference = (state->h << 8 | state->l);
                    DCR(state, &state->memory[mem_reference]);
                    break;
                  }
        case 0x36:                                                              //	MVI     M, 8bit_data
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], code[1]);
                    state->pc += 1;
                    break;
                  }                 
        case 0x37: UnimplementedInstruction(state); break;		                  //	STC
        case 0x38: break;		                                                    //	NOP
        case 0x39: DAD(state, (uint32_t)state->sp); break;                      //  DAD     SP
        case 0x3a:                                        		                  //	LDA     address
                  {
                    // Loads accumulator with value stored in memory at passed addr
                    uint16_t mem_reference = code[2] << 8 | code[1];
                    state->a = state->memory[mem_reference];
                    state->pc += 1;
                    break;
                  }
        case 0x3b: state->sp -= 0x01; break;		                                //	DCX     SP
        case 0x3c: INR(state, &state->a); break;                                //  INR     A
        case 0x3d: DCR(state, &state->a); break;                                //  DCR     A
        case 0x3e: MOV(&state->a, code[1]); state->pc += 1; break;		    		  //	MVI     A, 8bit_data
        case 0x3f: UnimplementedInstruction(state); break;		                  //	CMC
        case 0x40: MOV(&state->b, state->b); break;       		                  //	MOV     B, B
        case 0x41: MOV(&state->b, state->c); break;		                          //	MOV     B, C
        case 0x42: MOV(&state->b, state->d); break;		                          //	MOV     B, D
        case 0x43: MOV(&state->b, state->e); break;		                          //	MOV     B, E
        case 0x44: MOV(&state->b, state->h); break;		    		                  //	MOV     B, H
        case 0x45: MOV(&state->b, state->l); break;		    		                  //	MOV     B, L
        case 0x46:                                                              //	MOV     B, M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->b, state->memory[mem_reference]);
                    break;
                  }
        case 0x47: MOV(&state->b, state->a); break;		    		                  //	MOV     B, A
        case 0x48: MOV(&state->c, state->b); break;		    		                  //	MOV     C, B
        case 0x49: MOV(&state->c, state->c); break;		    		                  //	MOV     C, C
        case 0x4a: MOV(&state->c, state->d); break;		    		                  //	MOV     C, D
        case 0x4b: MOV(&state->c, state->e); break;		    		                  //	MOV     C, E
        case 0x4c: MOV(&state->c, state->h); break;		    		                  //	MOV     C, H
        case 0x4d: MOV(&state->c, state->l); break;		    		                  //	MOV     C, L
        case 0x4e:                                                              //	MOV     C, M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->c, state->memory[mem_reference]);
                    break;
                  }
        case 0x4f: MOV(&state->c, state->a); break;		    		                  //	MOV     C, A
        case 0x50: MOV(&state->d, state->b); break;		    		                  //	MOV     D, B
        case 0x51: MOV(&state->d, state->c); break;		    		                  //	MOV     D, C
        case 0x52: MOV(&state->d, state->d); break;		    		                  //	MOV     D, D
        case 0x53: MOV(&state->d, state->e); break;		    		                  //	MOV     D, E
        case 0x54: MOV(&state->d, state->h); break;		    		                  //	MOV     D, H
        case 0x55: MOV(&state->d, state->l); break;		    		                  //	MOV     D, L
        case 0x56:                                                              //	MOV     D, M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->d, state->memory[mem_reference]);
                    break;
                  }
        case 0x57: MOV(&state->d, state->a); break;		    		                  //	MOV     D, A
        case 0x58: MOV(&state->e, state->b); break;		    		                  //	MOV     E, B
        case 0x59: MOV(&state->e, state->c); break;		    		                  //	MOV     E, C
        case 0x5a: MOV(&state->e, state->d); break;		    		                  //	MOV     E, D
        case 0x5b: MOV(&state->e, state->e); break;		    		                  //	MOV     E, E
        case 0x5c: MOV(&state->e, state->h); break;		    		                  //	MOV     E, H
        case 0x5d: MOV(&state->e, state->l); break;		    		                  //	MOV     E, L
        case 0x5e:                                                              //	MOV     E, M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->e, state->memory[mem_reference]);
                    break;
                  }
        case 0x5f: MOV(&state->e, state->a); break;		    		                  //	MOV     E, A
        case 0x60: MOV(&state->h, state->b); break;		    		                  //	MOV     H, B
        case 0x61: MOV(&state->h, state->c); break;		    		                  //	MOV     H, C
        case 0x62: MOV(&state->h, state->d); break;		    		                  //	MOV     H, D
        case 0x63: MOV(&state->h, state->e); break;		    		                  //	MOV     H, E
        case 0x64: MOV(&state->h, state->h); break;		    		                  //	MOV     H, H
        case 0x65: MOV(&state->h, state->l); break;		    		                  //	MOV     H, L
        case 0x66:                                                              //	MOV     H, M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->h, state->memory[mem_reference]);
                    break;
                  }
        case 0x67: MOV(&state->h, state->a); break;		    		                  //	MOV     H, A
        case 0x68: MOV(&state->l, state->b); break;		    		                  //	MOV     L, B
        case 0x69: MOV(&state->l, state->c); break;		    		                  //	MOV     L, C
        case 0x6a: MOV(&state->l, state->d); break;		    		                  //	MOV     L, D
        case 0x6b: MOV(&state->l, state->e); break;		    		                  //	MOV     L, E
        case 0x6c: MOV(&state->l, state->h); break;		    		                  //	MOV     L, H
        case 0x6d: MOV(&state->l, state->l); break;		    		                  //	MOV     L, L
        case 0x6e:                                                              //	MOV     L, M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->l, state->memory[mem_reference]);
                    break;
                  }
        case 0x6f: MOV(&state->l, state->a); break;		    		                  //	MOV     L, A
        case 0x70:                                                              //	MOV     M, B
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], state->b);
                    break;
                  }
        case 0x71:                                                              //	MOV     M, C
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], state->c);
                    break;
                  }
        case 0x72:                                                              //	MOV     M, D
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], state->d);
                    break;
                  }
        case 0x73:                                                              //	MOV     M, E
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], state->e);
                    break;
                  }
        case 0x74:                                                              //	MOV     M, H
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], state->h);
                    break;
                  }
        case 0x75:                                                              // 	MOV     M, L
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], state->l);
                    break;
                  }
        case 0x76: UnimplementedInstruction(state); break;		                  //	HLT
        case 0x77:                                                              //	MOV     M, A
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->memory[mem_reference], state->a);
                    break;
                  }
        case 0x78: MOV(&state->a, state->b); break;		    		                  //	MOV     A, B
        case 0x79: MOV(&state->a, state->c); break;		    		                  //	MOV     A, C
        case 0x7a: MOV(&state->a, state->d); break;		    		                  //	MOV     A, D
        case 0x7b: MOV(&state->a, state->e); break;		    		                  //	MOV     A, E
        case 0x7c: MOV(&state->a, state->h); break;		    	                  	//	MOV     A, H
        case 0x7d: MOV(&state->a, state->l); break;		    	                  	//	MOV     A, L
        case 0x7e:                                                              //	MOV     A, M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    MOV(&state->a, state->memory[mem_reference]);
                    break;
                  }
        case 0x7f: MOV(&state->a, state->a); break;		    	                  	//	MOV     A, A

        case 0x80: Arithmetic(state, state->b, ADD, NO_CARRY); break;           //  ADD     B
        case 0x81: Arithmetic(state, state->c, ADD, NO_CARRY); break;	          //	ADD     C
        case 0x82: Arithmetic(state, state->d, ADD, NO_CARRY); break;		        //	ADD     D
        case 0x83: Arithmetic(state, state->e, ADD, NO_CARRY); break;		        //	ADD     E
        case 0x84: Arithmetic(state, state->h, ADD, NO_CARRY); break;		        //	ADD     H
        case 0x85: Arithmetic(state, state->l, ADD, NO_CARRY); break;		        //	ADD     L
        case 0x86:                                                              //	ADD     M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    Arithmetic(state, state->memory[mem_reference], ADD, NO_CARRY); break;
                  }
        case 0x87: Arithmetic(state, state->a, ADD, NO_CARRY); break;		        //	ADD     A
        case 0x88: Arithmetic(state, state->b, ADD, CARRY); break;		          //	ADC     B
        case 0x89: Arithmetic(state, state->c, ADD, CARRY);	break;              //	ADC     C
        case 0x8a: Arithmetic(state, state->d, ADD, CARRY);	break;  	          //	ADC     D
        case 0x8b: Arithmetic(state, state->e, ADD, CARRY);	break;		          //	ADC     E
        case 0x8c: Arithmetic(state, state->h, ADD, CARRY);	break;		          //	ADC     H
        case 0x8d: Arithmetic(state, state->l, ADD, CARRY);	break;		          //	ADC     L
        case 0x8e:                                                              //	ADC     M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    Arithmetic(state, state->memory[mem_reference], ADD, CARRY); break;
                  }
        case 0x8f: Arithmetic(state, state->a, ADD, CARRY);	break;		          //	ADC     A
        case 0x90: Arithmetic(state, state->b, SUB, NO_CARRY);	break;		      //	SUB     B
        case 0x91: Arithmetic(state, state->c, SUB, NO_CARRY);	break;		      //	SUB     C
        case 0x92: Arithmetic(state, state->d, SUB, NO_CARRY);	break;		      //	SUB     D
        case 0x93: Arithmetic(state, state->e, SUB, NO_CARRY);	break;		      //	SUB     E
        case 0x94: Arithmetic(state, state->h, SUB, NO_CARRY);	break;		      //	SUB     H
        case 0x95: Arithmetic(state, state->l, SUB, NO_CARRY);	break;		      //	SUB     L
        case 0x96:                                                              //	SUB     M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    Arithmetic(state, state->memory[mem_reference], SUB, NO_CARRY); break;
                  }
        case 0x97: Arithmetic(state, state->a, SUB, NO_CARRY);	break;		      //	SUB     A
        case 0x98: Arithmetic(state, state->b, SUB, CARRY);	break;		          //	SBB     B
        case 0x99: Arithmetic(state, state->c, SUB, CARRY);	break;		          //	SBB     C
        case 0x9a: Arithmetic(state, state->d, SUB, CARRY);	break;		          //	SBB     D
        case 0x9b: Arithmetic(state, state->e, SUB, CARRY);	break;		          //	SBB     E
        case 0x9c: Arithmetic(state, state->h, SUB, CARRY);	break;		          //	SBB     H
        case 0x9d: Arithmetic(state, state->l, SUB, CARRY);	break;		          //	SBB     L
        case 0x9e:                                                              //	SBB     M
                  {
                    uint16_t mem_reference = (state->h << 8 | state->l);
                    Arithmetic(state, state->memory[mem_reference], SUB, CARRY); break;
                  }
        case 0x9f: Arithmetic(state, state->a, SUB, CARRY);	break;		          //	SBB     A
        case 0xa0: UnimplementedInstruction(state); break;		                  //	ANA     B
        case 0xa1: UnimplementedInstruction(state); break;		                  //	ANA     C
        case 0xa2: UnimplementedInstruction(state); break;		                  //	ANA     D
        case 0xa3: UnimplementedInstruction(state); break;		                  //	ANA     E
        case 0xa4: UnimplementedInstruction(state); break;		                  //	ANA     H
        case 0xa5: UnimplementedInstruction(state); break;		                  //	ANA     L
        case 0xa6: UnimplementedInstruction(state); break;		                  //	ANA     M
        case 0xa7: UnimplementedInstruction(state); break; 		                  // 	ANA     A
        case 0xa8: UnimplementedInstruction(state); break;		                  //	XRA     B
        case 0xa9: UnimplementedInstruction(state); break;		                  //  XRA     C
        case 0xaa: UnimplementedInstruction(state); break;		                  //  XRA     D
        case 0xab: UnimplementedInstruction(state); break;		                  //  XRA     E
        case 0xac: UnimplementedInstruction(state); break;		                  //  XRA     H
        case 0xad: UnimplementedInstruction(state); break;		                  //  XRA     L
        case 0xae: UnimplementedInstruction(state); break;		                  //  XRA     M
        case 0xaf: UnimplementedInstruction(state); break;		                  //  XRA     A

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
        case 0xc0:
                  //  RNZ
                  if (0 == state->cc.z) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xc1: 	
                  //Pop a register pair on stack 					        POP    B
                  {
                    // Pop B and C from stack
                    state->c = state->memory[state->sp];
                    state->b = state->memory[state->sp+1];
                    // Increment pointer
                    state->sp += 2;
                  }
                  break;
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
        case 0xc4:
                  // CNZ addr
                  if (0 == state->cc.z) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xc5: 			
                  //Put a register pair on stack  		                PUSH   B
                  {
                  // Push B and C onto stack
                  state->memory[state->sp-1] = state->b;
                  state->memory[state->sp-2] = state->c;
                  // Decrement pointer
                  state->sp = state->sp - 2;
                  }
                  break;
        case 0xc6: Arithmetic(state, code[1], ADD, NO_CARRY); state->pc += 1;	break;	//  ADI     8bit_data
        case 0xc7:
                  //  RST     0
                  RST(state, 0);
                  break;
        case 0xc8:
                  // RZ
                  if (1 == state->cc.z) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
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
        case 0xcb: break;		                                                          //  NOP
        case 0xcc:
                  // CZ addr
                  if (1 == state->cc.z) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xcd:
                  //CALL address
                  CALL(state, code);
                  break;
        case 0xce: Arithmetic(state, code[1], ADD, CARRY); state->pc += 1; break;    //  ACI     8bit_data
        case 0xcf:
                  //  RST     1
                  RST(state, 1);
                  break;
        case 0xd0:
                  //  RNC
                  if (0 == state->cc.cy) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xd1: 			
                  // Pop a register pair on stack 			            POP    D
                  {
                            // Pop D and E from stack
                    state->e = state->memory[state->sp];
                    state->d = state->memory[state->sp+1];
                            // Increment pointer
                    state->sp += 2;
                  }
                  break;
        case 0xd2:
                  //  JNC address
                  if (0 == state->cc.cy) {
                      JMP(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xd3:
                  //OUT placeholder until we get to video and sound.
                  state->pc++;
                  break;
        case 0xd4:
                  //  CNC address
                  if (0 == state->cc.cy) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xd5: 				
                  //Puts a register pair on the stack		                PUSH   D
                  {
                        // Push D and E onto stack
                  state->memory[state->sp-1] = state->d;
                  state->memory[state->sp-2] = state->e;
                        // Decrement pointer
                  state->sp = state->sp - 2;
                  }
                  break;
        case 0xd6: Arithmetic(state, code[1], SUB, NO_CARRY); state->pc += 1; break;  //  SUI     8bit_data
        case 0xd7:
                  //  RST     2
                  RST(state, 2);
                  break;
        case 0xd8:
                  //  RC
                  if (1 == state->cc.cy) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xd9: break;		                                                          //  NOP
        case 0xda:
                  //  JC address
                  if (1 == state->cc.cy) {
                      JMP(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xdb:                                                                    //  IN      input_device_num
                  //IN placeholder until we get to video and sound.
                  state->pc++;
                  break;
        case 0xdc:
                  //  CC address
                  if (1 == state->cc.cy) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xdd: break;	                                                           //  NOP
        case 0xde: Arithmetic(state, code[1], SUB, CARRY); state->pc += 1; break;    //  SBI     8bit_data
        case 0xdf:
                  //  RST     3
                  RST(state, 3);
                  break;
        case 0xe0:
                  //  RPO
                  if (0 == state->cc.p) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xe1: 					
                  //Pop a register from the stack                         POP    H
                  {
                            // Pop H and L from stack
                    state->l = state->memory[state->sp];
                    state->h = state->memory[state->sp+1];
                            // Increment counter
                    state->sp += 2;
                  }
                  break;
        case 0xe2:
                  //  JPO address
                  if (0 == state->cc.p) {
                      JMP(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xe3: UnimplementedInstruction(state); break;		//  XTHL
        case 0xe4:
                  //  CPO     address
                  if (0 == state->cc.p) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xe5: 						
                  //Puts a register pair on the stack                   PUSH   H
                  {
                        // Push H and L onto stack
                  state->memory[state->sp-1] = state->h;
                  state->memory[state->sp-2] = state->l;
                        // Decrement pointer
                  state->sp = state->sp - 2;
                  }
                  break;
        case 0xe6: UnimplementedInstruction(state); break;		//  ANI     8bit_data
        case 0xe7:
                  //  RST     4
                  RST(state, 4);
                  break;
        case 0xe8:
                  //  RPE
                  if (1 == state->cc.p) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
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
        case 0xec:
                  //  CPE     address
                  if (1 == state->cc.p) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xed: break;		                                  //  NOP
        case 0xee: UnimplementedInstruction(state); break;		//  XRI     8bit_data
        case 0xef:
                  //  RST     5
                  RST(state, 5);
                  break;
        case 0xf0:
                  //  RP
                  if (0 == state->cc.s) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
		    case 0xf1:
                  // Pops PROGRAM STATUS WORD on the stack                POP PSW
                  // PSW combines accumulator A and flag register F
                  {
                            // Copy memory content into accumulator A
                    state->a = state->memory[state->sp+1];
                            // Set psw variable by copying memory
                            // content on top of stack. This is
                            // for flag register F.
                    uint8_t psw = state->memory[state->sp];
                            // Sets state cc struct values if equal
                            // to bitwise AND operation
                    state->cc.z  = (0x01 == (psw & 0x01));
                    state->cc.s  = (0x02 == (psw & 0x02));
                    state->cc.p  = (0x04 == (psw & 0x04));
                    state->cc.cy = (0x05 == (psw & 0x08));
                    state->cc.ac = (0x10 == (psw & 0x10));
                            // Increment pointer
                    state->sp += 2;
                  }
                  break;
        case 0xf2:
                  //  JP address
                  if (0 == state->cc.s) {
                      JMP(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xf3:
                  // DI disable processor interrupts flag set.
                  state->int_enable = 0;  
                  break;
        case 0xf4:
                  //  CP      address
                  if (0 == state->cc.s) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xf5: 						
                  // Puts PROGRAM STATUS WORD on the stack                PUSH PSW
                  // PSW combines accumulator A and flag register F
                  {
                        // Push accumulator A onto stack
                  state->memory[state->sp-1] = state->a;
                        // Create and set psw int variable by
                        // combining flag register F contained
                        // in state cc struct
                  uint8_t psw = (state->cc.z |
                          state->cc.s << 1 |
                          state->cc.p << 2 |
                          state->cc.cy << 3 |
                          state->cc.ac << 4 );
                        // Push psw onto stack and decrement pointer
                  state->memory[state->sp-2] = psw;
                  state->sp = state->sp - 2;
                  }
                  break;
        case 0xf6: UnimplementedInstruction(state); break;		//  ORI     8bit_data
        case 0xf7:
                  //  RST     6
                  RST(state, 6);
                  break;
        case 0xf8:
                  //  RM
                  if (1 == state->cc.s) {
                      RET(state);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xf9: UnimplementedInstruction(state); break;		//  SPHL
        case 0xfa:
                  //  JM address
                  if (1 == state->cc.s) {
                      JMP(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xfb:
                  // EI enable processor interrupts flag set.
                  state->int_enable = 1;  
                  break;
        case 0xfc:
                  //  CM      address
                  if (1 == state->cc.s) {
                      CALL(state, code);
                  } else {
                      state->pc += 2;
                  }
                  break;
        case 0xfd: break;		                                  //  NOP
        case 0xfe: UnimplementedInstruction(state); break;		//  CPI     8bit_data
        case 0xff:
                  //  RST     7
                  RST(state, 7);
                  break;
	}

	printf("\t");
	printf("%c", state->cc.z ? 'z' : '.');
	printf("%c", state->cc.s ? 's' : '.');
	printf("%c", state->cc.p ? 'p' : '.');
	printf("%c", state->cc.cy ? 'c' : '.');
	printf("%c  ", state->cc.ac ? 'a' : '.');
	printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->a, state->b, state->c,
				state->d, state->e, state->h, state->l, state->sp);

	return 0;
}
