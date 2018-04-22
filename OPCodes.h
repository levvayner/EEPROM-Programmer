#ifndef _OPCODES_H
#define _OPCODES_H
// OP Codes

#define NOP					0x00 //nothing to do, just passing time
#define LDI		 			0x80 // load immediate	  (value in next byte)
#define LDA					0x01 // load from address (address in next byte) 
#define LDB					0x02 // load from base register + offset (register in next byte)
#define LDC					0x03 // load from base register + offset (register in next byte)
#define LDD					0x04 // Load from D
#define LDF					0x05 // Load from F
#define SDA					0x06 // write a (to address in next byte)
#define SDB					0x50 // write a (to address in next byte)
#define SDC					0x51 // write a (to address in next byte)
#define SDD					0x52 // write a (to address in next byte)
#define SDF					0x53 // write a (to address in next byte)
#define MOV_REG_REG			0x07 // move A to B, C to D
#define MOV_REG_MEM			0x08 // Move to registry in next bye, address from byte after
#define MOV_MEM_REG			0x09 // Move to memory address in next byte, from register in byte after
#define MOV_MEM_MEM			0x0A //	Move from memory address to memory address (uses A register as temp)
#define MOV_REG_IMM 		0x0B // Move to register in next byte, value in byte after
#define MOV_MEM_IMM 		0x0C // Place an imm. value in memory. Address in next byte. value in byte after Like if I want all executables to start with 'MZ'
#define ADD_A				0x0D //Add B to A, Output to A
#define ADD_B				0x20 //Add B to A, Output to B
#define ADD_C				0x21 //Add B to A, Output to C
#define ADD_IP				0x0E //add contents of memory to A (address in next byte) * pointed by IP
#define SUB					0x0F //Subtract B from A, store to A, register to subtract from A in next byte
#define SUB_IP				0x10 //Subtract value pointed to by IP in MAR from A, write to A

#define JMP			0x11 //Jump to address pointed by lower 4 bits of IP Register
#define JLT			0x12 //Jump to address pointed by lower 4 bits of IP Register
#define JLE			0x13 //Jump to address pointed by lower 4 bits of IP Register
#define JE			0x14 //Jump to address pointed by lower 4 bits of IP Register
#define JG			0x15 //Jump to address pointed by lower 4 bits of IP Register
#define JGTE		0x16 //Jump if carry bit is active. TODO: Implement hardware
#define JC			0x17 //Jump if carry bit is active. TODO: Implement hardware
#define JZ			0x18 //Jump if carry bit is active. TODO: Implement hardware
#define JNZ			0x19
#define OUT			0x40 //Output content of A to decimal display (Output register 1)
#define HLT			0x1B //Terminate the program


#define SCF			0x1C //Set Carry Flag
#define CCF			0x1D //Clear Carry Flag

#define SZF			0x1E // Set Zero Flag
#define CZF			0x1F // Clear Zero Flag

#define PUSH_A		0x22 // Push value to stack (responsible for decrementing stack)
#define PUSH_B		0x23 // Push value to stack (responsible for decrementing stack)
#define PUSH_C		0x24 // Push value to stack (responsible for decrementing stack)
#define PUSH_D		0x25 // Push value to stack (responsible for decrementing stack)
#define PUSH_F		0x26 // Push value to stack (responsible for decrementing stack)
#define PUSH_SI		0x27 // Push value to stack (responsible for decrementing stack)
#define PUSH_DI		0x28 // Push value to stack (responsible for decrementing stack)
#define PUSH_SP		0x29 // Push value to stack (responsible for decrementing stack)
#define PUSH_BP		0x2A // Push value to stack (responsible for decrementing stack)
#define PUSH_CS		0x2B // Push value to stack (responsible for decrementing stack)
#define PUSH_DS		0x2C // Push value to stack (responsible for decrementing stack)
#define PUSH_ES		0x2D // Push value to stack (responsible for decrementing stack)
#define PUSH_SS		0x2E // Push value to stack (responsible for decrementing stack)
#define POP_A		0x30 // POP value from stack (responsible for incrementing stack)
#define POP_B		0x31 // POP value from stack (responsible for incrementing stack)
#define POP_C		0x32 // POP value from stack (responsible for incrementing stack)
#define POP_D		0x33 // POP value from stack (responsible for incrementing stack)
#define POP_F		0x34 // POP value from stack (responsible for incrementing stack)
#define POP_SI		0x35 // POP value from stack (responsible for incrementing stack)
#define POP_DI		0x36 // POP value from stack (responsible for incrementing stack)
//#define POP_BP		0x37 // POP value from stack (responsible for incrementing stack)
#define POP_SP		0x38 // POP value from stack (responsible for incrementing stack)
#define POP_ES		0x39 // POP value from stack (responsible for incrementing stack)
#define POP_SS		0x3A // POP value from stack (responsible for incrementing stack)
#define POP_DS		0x3B // POP value from stack (responsible for incrementing stack)
#define POP_CS		0x37 // POP value from stack (responsible for incrementing stack)
//for short jump user should push A, B, C, D, F registers onto stack before CALL_S. **For long jump, push CS register first.
#define CALL_S		0X3C // Pushes IP to stack.  (responsible for decrementing stack). Points IP to address in next byte from MAR
#define CALL_F		0X3D // Loads CS regsiter from next byte in MAR, points IP to byte after from MAR
//for short jump user should pop F, D, C, B, A registers from stack after RET. for long jump, pop CS next.
#define RET_S			0x3E // Pops value from register, loads into IP.  (responsible for incrementing stack)
#define RET_F		0x3F // Pops value from register into ip, pops next to CS

/* Mov REG to REG  - to A */
#define MOV_A_B		0x90 //Move from B to A
#define MOV_A_C		0x91 //Move from C to A
#define MOV_A_D		0x92 //Move from D to A
#define MOV_A_F		0x93 //Move from F to A
#define MOV_A_SI	0x94 //Move from SI to A
#define MOV_A_DI	0x95 //Move from DI to A
#define MOV_A_BP	0x96 //Move from BP to A
#define MOV_A_SP	0x97 //Move from SP to A
#define MOV_A_CS	0x98 //Move from CS to A
#define MOV_A_SS	0x99 //Move from SS to A
#define MOV_A_DS	0x9A //Move from DS to A
#define MOV_A_ES	0x9B //Move from ES to A

/* Mov REG to REG  - to B */
#define MOV_B_A		0x9C //Move from A to B
#define MOV_B_C		0x9D //Move from C to B
#define MOV_B_D		0x9E //Move from D to B
#define MOV_B_F		0x9F //Move from F to B
#define MOV_B_SI	0xA0 //Move from SI to B
#define MOV_B_DI	0xA1 //Move from DI to B
#define MOV_B_BP	0xA2 //Move from BP to B
#define MOV_B_SP	0xA3 //Move from SP to B
#define MOV_B_CS	0xA4 //Move from CS to B
#define MOV_B_SS	0xA5 //Move from SS to B
#define MOV_B_DS	0xA6 //Move from DS to B
#define MOV_B_ES	0xA7 //Move from ES to B

/* Mov REG to REG  - to C */
#define MOV_C_A		0xA8 //Move from A to C
#define MOV_C_B		0xA9 //Move from B to C
#define MOV_C_D		0xAA //Move from D to C
#define MOV_C_F		0xAB //Move from F to C
#define MOV_C_SI	0xAC //Move from SI to C
#define MOV_C_DI	0xAD //Move from DI to C
#define MOV_C_BP	0xAE //Move from BP to C
#define MOV_C_SP	0xAF //Move from SP to C
#define MOV_C_CS	0xB0 //Move from CS to C
#define MOV_C_SS	0xB1 //Move from SS to C
#define MOV_C_DS	0xB2 //Move from DS to C
#define MOV_C_ES	0xB3 //Move from ES to C

/* Mov REG to REG  - to D */
#define MOV_D_A		0xB4 //Move from A to D
#define MOV_D_B		0xB5 //Move from C to D
#define MOV_D_C		0xB6 //Move from D to D
#define MOV_D_F		0xB7 //Move from F to D
#define MOV_D_SI	0xB8 //Move from SI to D
#define MOV_D_DI	0xB9 //Move from DI to D
#define MOV_D_BP	0xBA //Move from BP to D
#define MOV_D_SP	0xBB //Move from SP to D
#define MOV_D_CS	0xBC //Move from CS to D
#define MOV_D_SS	0xBD //Move from SS to D
#define MOV_D_DS	0xBE //Move from DS to D
#define MOV_D_ES	0xBF //Move from ES to D

/* Mov REG to REG  - to F */
#define MOV_F_A		0xC0 //Move from A to F
#define MOV_F_B		0xC1 //Move from B to F
#define MOV_F_C		0xC2 //Move from C to F
#define MOV_F_D		0xC3 //Move from D to F

/* Mov REG to REG  - to SI */
#define MOV_SI_A		0xC4 //Move from A to SI
#define MOV_SI_B		0xC5 //Move from B to SI
#define MOV_SI_C		0xC6 //Move from C to SI
#define MOV_SI_D		0xC7 //Move from D to SI


/* Mov REG to REG  - to DI */
#define MOV_DI_A		0xC8 //Move from A to DI
#define MOV_DI_B		0xC9 //Move from B to DI
#define MOV_DI_C		0xCA //Move from C to DI
#define MOV_DI_D		0xCB //Move from D to DI


/* Mov REG to REG  - to BP */
#define MOV_BP_A		0xCC //Move from A to BP
#define MOV_BP_B		0xCD //Move from B to BP
#define MOV_BP_C		0xCE //Move from C to BP
#define MOV_BP_D		0xCF //Move from D to BP


/* Mov REG to REG  - to SP */ /* Yeah, we can do it. But do we need to ?!? */
#define MOV_SP_A		0xD0 //Move from A to SP
#define MOV_SP_B		0xD1 //Move from B to SP
#define MOV_SP_C		0xD2 //Move from C to SP
#define MOV_SP_D		0xD3 //Move from D to SP


							  /* Mov REG to REG  - to CS */
#define MOV_CS_A		0xD4 //Move from A to CS
#define MOV_CS_B		0xD5 //Move from B to CS
#define MOV_CS_C		0xD6 //Move from C to CS
#define MOV_CS_D		0xD7 //Move from D to CS


							  /* Mov REG to REG  - to SS */
#define MOV_SS_A		0xD8 //Move from A to SS
#define MOV_SS_B		0xD9 //Move from B to SS
#define MOV_SS_C		0xDA //Move from C to SS
#define MOV_SS_D		0xDB //Move from D to SS


							  /* Mov REG to REG  - to DS */
#define MOV_DS_A		0xDC //Move from A to DS
#define MOV_DS_B		0xDD //Move from B to DS
#define MOV_DS_C		0xDE //Move from C to DS
#define MOV_DS_D		0xDF //Move from D to DS


							  /* Mov REG to REG  - to ES */
#define MOV_ES_A		0xE0 //Move from A to ES
#define MOV_ES_B		0xE1 //Move from B to ES
#define MOV_ES_C		0xE2 //Move from C to ES
#define MOV_ES_D		0xE3 //Move from D to ES


							  /* Load from Memory using offset in register  - to A */
#define LDAI_A		0xE4 //Load from DS + A in MAR to A
#define LDAI_B		0xE5 //Load from DS + B in MAR to A
#define LDAI_C		0xE6 //Load from DS + C in MAR to A
#define LDAI_D		0xE7 //Load from DS + D in MAR to A

							  /* Load from Memory using offset in register  - to B */
#define LDBI_A		0xE8 //Load from DS + A in MAR to B
#define LDBI_B		0xE9 //Load from DS + B in MAR to B
#define LDBI_C		0xEA //Load from DS + C in MAR to B
#define LDBI_D		0xEB //Load from DS + D in MAR to B

							  /* Load from Memory using offset in register  - to C */
#define LDCI_A		0xEC //Load from DS + A in MAR to C
#define LDCI_B		0xED //Load from DS + B in MAR to C
#define LDCI_C		0xEE //Load from DS + C in MAR to C
#define LDCI_D		0xEF //Load from DS + D in MAR to C

							  /* Save to Memory using offset in register  - value in A */
#define SDAI_A		0xF0 //Save to DS + A in MAR value in A
#define SDAI_B		0xF1 //Save to DS + B in MAR value in A
#define SDAI_C		0xF2 //Save to DS + C in MAR value in A
#define SDAI_D		0xF3 //Save to DS + D in MAR value in A



#endif