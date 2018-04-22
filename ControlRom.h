#ifndef _CONTROLROM_H
#define _CONTROLROM_H
#include "Arduino.h"
#include "Constructs.h"
#include "OPCodes.h"
/*
ROM based on control codes laid out as follows(from bit 15 to bit 0)

TBD [SR_b1 SR_b0] [C1_b2 C1_b1 C1_b0] [R] [W] [S3 S2 S1 S0] [D3 D2 D1 D0]

******************************************************************************************
** 		   Have lots tha can be done with lowest 8 bits if R and W are both 0!!         **
******************************************************************************************

SR - Segment register control lines b1 b0
------------------------------------
00 - Code Segment
01 - Stack Segment
10 - Data Segment
11 - Extra Segment


C1 - control lines, multiplexed where 0 is 000 and 3 is 011
000 - N/A - nothing selected
001 - Halt
010 - No Operation
011 - Program Counter Increment
100 - Subtract Flag
101 - Conditional Jump

R - read flag. If active, use S3 - S0 to multiplex device *READING FROM BUS*.
W - write flag. If acgive, use D3 - D0 to multiplex device *WRITING TO BUS*.

Device *Reading* From Bus (with read flag)
----------------------------------------------
0000 - A (Accumulator Register)
0001 - B (Base Register)
0010 - C (Counter Register)
0011 - D (Data Register)
0100 - F (Flag Register)
0101 - Output 1
0110 - TBD
0111 - MAR (Read data from bus, address in IR?!?)
1000 - PC (Program Counter)
1001 - SR (Segment Register *must be configured with optional segment selection flag *)
1010 - IR (Instruction Register)
1011 - MAR (Read data from device using address in IR?!?)
1100 - SP (Stack Pointer Register)
1101 - SP+- (Stack Pointer Inc/Dec)
1110 - SI (Source Pointer Register)
1111 - DI (Destination Pointer Register)

Device *Writing* to Bus
----------------------------------------------
0000 - A (Accumulator Register)
0001 - B (Base Register)
0010 - C (Counter Register)
0011 - D (Data Register)
0100 - F (Flag Register)
0101 - Input 1
0110 - ALU
0111 - TBD
1000 - PC (Program Counter)
1001 - SR (Segment Register *must be configured with optional segment selection flag *)
1010 - IR (Instruction Register)
1011 - MAR (Read data from device using address in IR?!?)
1100 - SP (Stack Pointer Register)
1101 - SP+- (Stack Pointer Inc/Dec)
1110 - SI (Source Pointer Register)
1111 - DI (Destination Pointer Register)
*/

/*
50s, 60s, 70s, 90 - FF all open
*/


#pragma region "Mechanics of writing to ROM"
#define READ_IN_FLAG_IDX 0x9
#define WRITE_OUT_FLAG_IDX 0x8
#define OP_CLEAR(var) var = var & 0xE000  
#define C_C1(var) var &= 0xE3FF //clear C1 flags in var
//use S_C1 to set
#define S_C1(var,x)  var =( C_C1(var)) |  x << 10 //clear bits and set per above order


/* Registers, set read flag and bits 4-7 multiplexed for read, write flag and bits 0-3*/
#define C_R(var) var &= 0xFD0F //clear Read flag and index
#define C_W(var) var &= 0xFEF0 //clear Write flag and index
#define S_R(var,x) var = (C_R(var)) | 1 << READ_IN_FLAG_IDX | x << 4 // clear, set read flag, and item
#define S_W(var,x) var = (C_W(var)) | 1 << WRITE_OUT_FLAG_IDX | x << 0 // clear, set read flag, and item
/* Now that convenience methods are done, instructions to be called by humans.*/

#define C_SR(var) var &= 0x1FFF // clear status register flag
#define OP_SEG(var,x) var = (C_SR(var)) | x << 13 ;



#define OP_Hlt(var)	 var = 1 << 10 //(S_C1(var,1)) & 0x1C00 //Halt, exclusive
#define OP_NOP(var)	 var = (S_C1(var,2)) & 0xFC00//No operation, exclusive
#define OP_PC_C(var) S_C1(var,3) //Program Counter Count
#define OP_U_S(var)	 S_C1(var,4) //Subtract Flag
#define OP_PC_JC(var) var =( S_C1(var,5))| 0 << 9 //Conditional Jump flag (lowest 3 bits have jump type)
// clear read flag and encode type


/* Operations can use shortcutzies*/
#define OP_A_I(var)		S_R(var,0)
#define OP_A_O(var)		S_W(var,0)
#define OP_B_I(var)		S_R(var,1)
#define OP_B_O(var)		S_W(var,1)
#define OP_C_I(var)		S_R(var,2)
#define OP_C_O(var)		S_W(var,2)
#define OP_D_I(var)		S_R(var,3)
#define OP_D_O(var)		S_W(var,3)
#define OP_F_I(var)		S_R(var,4)
#define OP_F_O(var)		S_W(var,5)
#define OP_O1_I(var)	S_R(var,5)
#define OP_I1_O(var)	S_W(var,5)
/* ALU */
#define OP_U_O(var)		S_W(var,6)
/* MEMORY READ */
#define OP_M_I(var)		S_R(var,7)
/* Program Counter */
#define OP_PC_I(var)	S_R(var,8)
#define OP_PC_O(var)	S_W(var,8)
/* Segment */
#define OP_SR_I(var)	S_R(var,9)
#define OP_SR_O(var)	S_W(var,9)

/* Instrcution Register */
#define OP_IR_I(var)	S_R(var,10)
#define OP_IR_O(var)	S_W(var,10)
/* MAR Address Load and Out*/
#define OP_M_L(var)		S_R(var,11)
#define OP_M_O(var)		S_W(var,11)

#define OP_SP_I(var)	S_R(var,12)
#define OP_SP_O(var)	S_W(var,12)
#define OP_SP_INC(var)	S_W(var,13)
#define OP_SP_DEC(var)	S_R(var,13)

#define OP_SI_I(var)	S_R(var,14)
#define OP_SI_O(var)	S_W(var,14)
#define OP_DI_I(var)	S_R(var,15)
#define OP_DI_O(var)	S_W(var,15)

#pragma endregion

class ControlRom
{
public:



	
#pragma region Lookups

	
	ControlRom();
	~ControlRom();


	void StoreControlROMData();

private:

#pragma region Bulk Programming Helpers

	bool Fetch(uint16_t &step, uint16_t addr);
	//Load next byte, put into A or other dest
	bool LoadMem(uint16_t &step, uint16_t addr);

	//Load next byte, put into A or other dest
	bool SumTo(uint16_t &step, uint16_t addr);
	bool Nop(uint16_t &step, uint16_t addr, uint16_t d = 0xFF);
	bool PC_to_IR(uint16_t &step, uint16_t addr);

	bool ConditionalJump(uint16_t &step, uint16_t addr, uint8_t conditionType);

	bool PushReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t output, uint8_t segment = 0x0);
	bool PopReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t input, uint8_t segment = 0x0);

	bool MovReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t input, uint16_t output , uint8_t segment = 0x0);

	bool LoadReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t output);
	bool StoreReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t output);
	
	bool LoadRegFromOffsetReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t input, uint16_t output, uint8_t segment = 0x2);
	bool SaveRegFromOffsetReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t input, uint16_t output, uint8_t segment = 0x2);

	
	bool Call(uint8_t opcode, uint16_t& step, uint16_t &addr, uint8_t condition=0x0);
	bool Ret(uint8_t opcode, uint16_t& step, uint16_t &addr, uint8_t condition = 0x0);
	

#pragma endregion
};

#endif;

