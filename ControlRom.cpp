#include "ControlRom.h"
#include "EEPROM.h"


extern EEPROM programmer;
ControlRom::ControlRom()
{
}


ControlRom::~ControlRom()
{
}



void ControlRom::StoreControlROMData() {
	Serial.println(F("Storing Control ROM data *16 bit*"));

	
	Serial.print(F("First clear.."));
	for (int i = 0; i < 2048; i++) {
		if(!programmer.WriteShort(i, 0x00))
			break;
		if (i % 128 == 0) Serial.print(F("."));
	}
	Serial.println(F(" done."));

	uint16_t addr = 0x0;
	uint16_t data = 0xFF;
	uint32_t counter = 0;
	uint16_t step = 0, stepSub = 0;
	char buff[128];
	bool valid = true;
	uint16_t eC = 0x0; //effective code
					   //For each command
	for (uint16_t i = 0; i <= 0xFF; i++) { //up to 256 instructions
		Serial.print("Processing main loop: COMMAND # 0x"); Serial.println(i, HEX);
		step = 0;
		if (!Fetch(step, i))
			break;
		counter += step;
		//before you embark on this endless journey, be forewarned the case statement is larger than you might be prepared to handle. Coffee might be necessary.
		switch (i)
		{
		case LDI: //next byte in memory has value
				  //set up for each register destination
				  //need to get next byte, detemrine registry info.
			//Serial.print("LDI -- ");
			for (uint8_t dest = 0; dest < 0x8; dest++) {
				stepSub = step;
				if (dest != 0) {
					stepSub = 0;
					if (!Fetch(stepSub, dest | i))
						break;
				}
				
				OP_CLEAR(data);
				eC = (dest | i);
				if (!LoadMem(stepSub, eC)) //get next byte
					return;
									  //PC_to_IR(step, addr | eC);
				eC = (dest | i);
				if (!Nop(stepSub, eC)) break;
				counter += stepSub; // manually augment our micro steps counter
			}
			step = stepSub; //update eq to 1 loop
			break;
		case LDA:// first byte is instruction, next byte has address to read from
			if (!LoadReg(i, step, addr, OP_A_I(data))) break;
			break;
		case LDB:
			if (!LoadReg(i, step, addr, OP_B_I(data))) break;
			break;
		case LDC:
			if (!LoadReg(i, step, addr, OP_C_I(data))) break;
			break;
		case LDD:
			if (!LoadReg(i, step, addr, OP_D_I(data))) break;
			break;

		case SDA:
			if (!StoreReg(i, step, addr, OP_A_O(data))) break;
			break;
		case SDB:
			if (!StoreReg(i, step, addr, OP_B_O(data))) break;
			break;
		case SDC:
			if (!StoreReg(i, step, addr, OP_C_O(data))) break;
			break;
		case SDD:
			if (!StoreReg(i, step, addr, OP_D_O(data))) break;
			break;
		case SDF:
			if (!StoreReg(i, step, addr, OP_F_O(data))) break;
		//case SDSP:
		//	StoreReg(i, addr, step, OP_F_O(data));
		

		
		case ADD_A: //add conents of b to a, load out to a
		//	Serial.print("ADD -- to A");
			OP_CLEAR(data);
			OP_A_I(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t5
			if (!programmer.WriteShort(addr, data))
				break;

			if (! Nop(step, addr)) break;
			break;
		case ADD_B: //add conents of b to a, load out to a
		//	Serial.print("ADD -- to B");
			OP_CLEAR(data);
			OP_B_I(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t5
			if (!programmer.WriteShort(addr, data))
				break;

			if (!Nop(step, addr)) break;
			break;

		case ADD_C: //add conents of b to a, load out to a
		//	Serial.print("ADD -- to C");
			OP_CLEAR(data);
			OP_C_I(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t5
			if (!programmer.WriteShort(addr, data))
				break;

			if (!Nop(step, addr)) break;
			break;
		case ADD_IP: //add immideate
		//	Serial.print("ADD_IP -- ");
			OP_CLEAR(data);
			OP_SEG(data, 0x0); // Code Segment
			OP_M_L(data);
			OP_PC_O(data);
			addr = step++ << 8 | i; //t3
			if (!programmer.WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_M_O(data);
			OP_B_I(data);
			OP_PC_C(data);
			addr = step++ << 8 | i; //t4
			if (!programmer.WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_A_I(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t5
			if (!programmer.WriteShort(addr, data))
				break;

			if (!Nop(step, addr)) break;
			break;
		case SUB: //subtract contents of B from A
			//Serial.print("SUB -- ");
			OP_CLEAR(data);
			OP_SEG(data, 0x0); // Code Segment
			OP_A_I(data);
			OP_U_S(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t4
			if (!programmer.WriteShort(addr, data))
				break;
			//PC_to_IR(step, addr | eC);
			Nop(step, addr);
			break;
		case SUB_IP:
		//	Serial.print("SUB_IP -- ");
			OP_CLEAR(data);
			OP_SEG(data, 0x0); // Code Segment
			OP_M_L(data);
			OP_IR_O(data);
			addr = step++ << 8 | i; //t3
			if (!programmer.WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_M_O(data);
			OP_B_I(data);
			addr = step++ << 8 | i; //t4
			if (!programmer.WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_A_I(data);
			OP_U_O(data);
			OP_U_S(data);
			addr = step++ << 8 | i; //t5
			if (!programmer.WriteShort(addr, data))
				break;

			if (!Nop(step, addr)) break;
			break;
		case JMP:
		//	Serial.print("JMP -- ");

			//load next byte into PC, increment PC on falling edge
			OP_CLEAR(data);
			OP_SEG(data, 0x0); // Code Segment
			OP_PC_O(data);
			OP_PC_C(data);
			OP_M_L(data);
			addr = step++ << 8 | i; //t3
			if (!programmer.WriteShort(addr, data))
				break;
			//load byte into 
			OP_CLEAR(data);
			OP_PC_I(data);
			OP_M_O(data);
			addr = step++ << 8 | i; //t4
			if (!programmer.WriteShort(addr, data))
				break;

			//should jump so no need for nop, but what the hey, let's make it rain NOPs
			if (!Nop(step, addr)) break;
			break;
		case JLT:
			ConditionalJump(step, JLT, 0x0);
			break;
		case JLE:
			ConditionalJump(step, JLE, 0x1);
			break;
		case JE:
			ConditionalJump(step, JE, 0x2);
			break;
		case JG:
			ConditionalJump(step, JG, 0x3);
			break;
		case JGTE:
			ConditionalJump(step, JGTE, 0x4);
			break;
		case JC:
			ConditionalJump(step, JC, 0x5);
			break;
		case JZ:
			ConditionalJump(step, JZ, 0x6);
			break;
		case JNZ:
			ConditionalJump(step, JNZ, 0x7);
			break;
		case OUT:
			//Serial.print("OUT -- ");
			OP_CLEAR(data);
			OP_SEG(data, 0x0); // Code Segment, doesnt really matter here

			for (uint8_t dest = 0; dest < 0x8; dest++) {
				stepSub = step;
				if (dest != 0) {
					stepSub = 0;
					if(!Fetch(stepSub, dest | i)) break;
				}

				switch (dest) {
				case 0: //Reg A
					OP_A_O(data);
					break;
				case 1:
					OP_B_O(data);
					break;
				case 2:
					OP_C_O(data);
					break;
				case 3:
					OP_D_O(data);
					break;
				case 4:
					OP_F_O(data);
					break;
				case 5:
					OP_PC_O(data);
					break;
				case 6:
					OP_IR_O(data);
					break;
				case 7:
					OP_M_O(data);
					break;
				}


				OP_O1_I(data);
				addr = stepSub++ << 8 | dest | i; //t3
				//Serial.print("Output register "); Serial.println(i);
				if (!programmer.WriteShort(addr, data))
					break;

				if (!Nop(stepSub, addr)) break;
				
				counter += stepSub; // manually augment our micro steps counter
			}
			step = stepSub; //update eq to 1 loop
			
		//write to stack pointer line is #13, increment and decrement is #14 as in out respectivly.
		case PUSH_A		: //22 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_A_O(data));
			break;
		case PUSH_B		: //23 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_B_O(data));
			break;
		case PUSH_C		: //24 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_C_O(data));
			break;
		case PUSH_D		: //25 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_D_O(data));
			break;
		case PUSH_F		: //26 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_F_O(data));
			break;
		case PUSH_SI		: //27 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_SI_O(data));
			break;
		case PUSH_DI		: //28 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_DI_O(data));
			break;
		case PUSH_SP:		//0x29 // Push value to stack (responsible for decrementing stack)
			PushReg(i, step, addr, OP_SP_O(data));
			break;
		case PUSH_BP: //2A // POP value from stack (responsible for incrementing stack)
			PushReg(i, step, addr, OP_SP_O(data)); /* SAME AS STACK POINTER FOR NOW*/
			break;
		case PUSH_CS: //2B // POP value from stack (responsible for incrementing stack)
			OP_SEG(data, 0x0);
			PushReg(i, step, addr, OP_SR_O(data));
			break;
		case PUSH_DS: //2C // POP value from stack (responsible for incrementing stack)
			OP_SEG(data, 0x2);
			PushReg(i, step, addr, OP_SR_O(data));
			break;
		case PUSH_ES: //2D // POP value from stack (responsible for incrementing stack)
			OP_SEG(data, 0x3);
			PushReg(i, step, addr, OP_SR_O(data));
			break;
		case PUSH_SS: //2E // POP value from stack (responsible for incrementing stack)
			OP_SEG(data, 0x1);
			PushReg(i, step, addr, OP_SR_O(data));
			break;
		
		case POP_A		: //2A // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_A_I(data));
			break;
		case POP_B		: //2B // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_B_I(data));
			break;
		case POP_C: //2B // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_C_I(data));
			break;
		case POP_D		: //2C // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_D_I(data));
			break;
		case POP_F		: //2D // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_F_I(data));
			break;
		case POP_SI		: //2E // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_SI_I(data));
			break;
	
		case POP_DI: //2A // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_DI_I(data));
			break;

		case POP_CS: //2A // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_SR_I(data), 0x0);
			break;

		case POP_SP: //2A // POP value from stack (responsible for incrementing stack)
			PopReg(i, step, addr, OP_SP_I(data));
			break;

		case POP_ES: //2A // POP value from stack (responsible for incrementing stack)
			OP_SEG(data, 0x3);
			PopReg(i, step, addr, OP_SR_I(data),0x3);
			break;
		case POP_SS: //2A // POP value from stack (responsible for incrementing stack)
			OP_SEG(data, 0x1);
			PopReg(i, step, addr, OP_SR_I(data),0x1);
			break;
		case POP_DS: //2A // POP value from stack (responsible for incrementing stack)
			OP_SEG(data, 0x2);
			PopReg(i, step, addr, OP_SR_I(data),0x2);
			break;

		case CALL_S		: //30 // Pushes IP to stack.  (responsible for decrementing stack). Points IP to address in next byte from MAR
			Call(i, step, addr);
			break;
		case CALL_F		: //31 // Loads CS regsiter from next byte in MAR, points IP to byte after from MAR
			Call(i, step, addr, 0x1);
			break;			
		case RET_S: //32 // Pops value from register, loads into IP.  (responsible for incrementing stack)
			Ret(i, step, addr);			
			break;
		case RET_F: //32 // Pops value from register, loads into IP.  (responsible for incrementing stack)
			Ret(i, step, addr,1);
			break;
		case MOV_A_B:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_B_O(data))) break;
			break;
		case MOV_A_C:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_C_O(data))) break;
			break;
		case MOV_A_D:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_D_O(data))) break;
			break;
		case MOV_A_F:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_F_O(data))) break;
			break;
		case MOV_A_SI:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_SI_O(data))) break;
			break;
		case MOV_A_DI:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_DI_O(data))) break;
			break;
		case MOV_A_BP:
			//if(!MovReg(i, step, addr, OP_A_I(data), OP_BP_O(data))) break;
			break;
		case MOV_A_SP:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_SP_O(data))) break;
			break;
		case MOV_A_CS:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_SR_O(data))) break;
			break;
		case MOV_A_SS:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_SR_O(data), 0x1)) break;
			break;
		case MOV_A_DS:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_SR_O(data), 0x2)) break;
			break;
		case MOV_A_ES:
			if(!MovReg(i, step, addr, OP_A_I(data), OP_SR_O(data), 0x3)) break;
			break;


		case MOV_B_A:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_A_O(data))) break;
			break;
		case MOV_B_C:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_C_O(data))) break;
			break;
		case MOV_B_D:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_D_O(data))) break;
			break;
		case MOV_B_F:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_F_O(data))) break;
			break;
		case MOV_B_SI:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_SI_O(data))) break;
			break;
		case MOV_B_DI:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_DI_O(data))) break;
			break;
		case MOV_B_BP:
			//if(!MovReg(i, step, addr, OP_B_I(data), OP_BP_O(data))) break;
			break;
		case MOV_B_SP:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_SP_O(data))) break;
			break;
		case MOV_B_CS:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_SR_O(data))) break;
			break;
		case MOV_B_SS:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_SR_O(data), 0x1)) break;
			break;
		case MOV_B_DS:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_SR_O(data), 0x2)) break;
			break;
		case MOV_B_ES:
			if(!MovReg(i, step, addr, OP_B_I(data), OP_SR_O(data), 0x3)) break;
			break;

		case MOV_C_A:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_A_O(data))) break;
			break;
		case MOV_C_B:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_B_O(data))) break;
			break;
		case MOV_C_D:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_D_O(data))) break;
			break;
		case MOV_C_F:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_F_O(data))) break;
			break;
		case MOV_C_SI:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_SI_O(data))) break;
			break;
		case MOV_C_DI:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_DI_O(data))) break;
			break;
		case MOV_C_BP:
			//if(!MovReg(i, step, addr, OP_C_I(data), OP_BP_O(data))) break;
			break;
		case MOV_C_SP:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_SP_O(data))) break;
			break;
		case MOV_C_CS:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_SR_O(data))) break;
			break;
		case MOV_C_SS:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_SR_O(data), 0x1)) break;
			break;
		case MOV_C_DS:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_SR_O(data), 0x2)) break;
			break;
		case MOV_C_ES:
			if(!MovReg(i, step, addr, OP_C_I(data), OP_SR_O(data), 0x3)) break;
			break;

		case MOV_D_A:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_A_O(data))) break;
			break;
		case MOV_D_B:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_B_O(data))) break;
			break;
		case MOV_D_C:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_C_O(data))) break;
			break;
		case MOV_D_F:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_F_O(data))) break;
			break;
		case MOV_D_SI:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_SI_O(data))) break;
			break;
		case MOV_D_DI:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_DI_O(data))) break;
			break;
		case MOV_D_BP:
			//if(!MovReg(i, step, addr, OP_D_I(data), OP_BP_O(data))) break;
			break;
		case MOV_D_SP:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_SP_O(data))) break;
			break;
		case MOV_D_CS:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_SR_O(data))) break;
			break;
		case MOV_D_SS:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_SR_O(data), 0x1)) break;
			break;
		case MOV_D_DS:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_SR_O(data), 0x2)) break;
			break;
		case MOV_D_ES:
			if(!MovReg(i, step, addr, OP_D_I(data), OP_SR_O(data), 0x3)) break;
			break;
		/* F REG  */
		case MOV_F_A:
			if(!MovReg(i, step, addr, OP_F_I(data), OP_A_O(data))) break;
			break;
		case MOV_F_B:
			if(!MovReg(i, step, addr, OP_F_I(data), OP_B_O(data))) break;
			break;
		case MOV_F_C:
			if(!MovReg(i, step, addr, OP_F_I(data), OP_C_O(data))) break;
			break;
		case MOV_F_D:
			if(!MovReg(i, step, addr, OP_F_I(data), OP_D_O(data))) break;
			break;
		
		/* SI REG  */
		case MOV_SI_A:
			if(!MovReg(i, step, addr, OP_SI_I(data), OP_A_O(data))) break;
			break;
		case MOV_SI_B:
			if(!MovReg(i, step, addr, OP_SI_I(data), OP_B_O(data))) break;
			break;
		case MOV_SI_C:
			if(!MovReg(i, step, addr, OP_SI_I(data), OP_C_O(data))) break;
			break;
		case MOV_SI_D:
			if(!MovReg(i, step, addr, OP_SI_I(data), OP_D_O(data))) break;
			break;

		/* DI REG  */
		case MOV_DI_A:
			if(!MovReg(i, step, addr, OP_DI_I(data), OP_A_O(data))) break;
			break;
		case MOV_DI_B:
			if(!MovReg(i, step, addr, OP_DI_I(data), OP_B_O(data))) break;
			break;
		case MOV_DI_C:
			if(!MovReg(i, step, addr, OP_DI_I(data), OP_C_O(data))) break;
			break;
		case MOV_DI_D:
			if(!MovReg(i, step, addr, OP_DI_I(data), OP_D_O(data))) break;
			break;

		/* BP REG  */
		/*case MOV_BP_A:
			if(!MovReg(i, step, addr, OP_BP_I(data), OP_A_O(data))) break;
			break;
		case MOV_BP_B:
			if(!MovReg(i, step, addr, OP_BP_I(data), OP_B_O(data))) break;
			break;
		case MOV_BP_C:
			if(!MovReg(i, step, addr, OP_BP_I(data), OP_C_O(data))) break;
			break;
		case MOV_BP_D:
			if(!MovReg(i, step, addr, OP_BP_I(data), OP_D_O(data))) break;
			break;*/

		/* SP REG  */
		case MOV_SP_A:
			if(!MovReg(i, step, addr, OP_SP_I(data), OP_A_O(data))) break;
			break;
		case MOV_SP_B:
			if(!MovReg(i, step, addr, OP_SP_I(data), OP_B_O(data))) break;
			break;
		case MOV_SP_C:
			if(!MovReg(i, step, addr, OP_SP_I(data), OP_C_O(data))) break;
			break;
		case MOV_SP_D:
			if(!MovReg(i, step, addr, OP_SP_I(data), OP_D_O(data))) break;
			break;
		
		/* CS REG  */
		case MOV_CS_A:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_A_O(data))) break;
			break;
		case MOV_CS_B:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_B_O(data))) break;
			break;
		case MOV_CS_C:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_C_O(data))) break;
			break;
		case MOV_CS_D:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_D_O(data))) break;
			break;

		/* SS REG  */
		case MOV_SS_A:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_A_O(data), 0X1)) break;
			break;
		case MOV_SS_B:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_B_O(data), 0X1)) break;
			break;
		case MOV_SS_C:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_C_O(data), 0X1)) break;
			break;
		case MOV_SS_D:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_D_O(data), 0X1)) break;
			break;

		/* DS REG  */
		case MOV_DS_A:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_A_O(data), 0X2)) break;
			break;
		case MOV_DS_B:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_B_O(data), 0X2)) break;
			break;
		case MOV_DS_C:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_C_O(data), 0X2)) break;
			break;
		case MOV_DS_D:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_D_O(data), 0X2)) break;
			break;

			/* ES REG  */
		case MOV_ES_A:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_A_O(data), 0X3)) break;
			break;
		case MOV_ES_B:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_B_O(data), 0X3)) break;
			break;
		case MOV_ES_C:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_C_O(data), 0X3)) break;
			break;
		case MOV_ES_D:
			if(!MovReg(i, step, addr, OP_SR_I(data), OP_D_O(data), 0X3)) break;
			break;



		case LDAI_A: //		0xE4 //Load from DS + A in MAR to A
			if(!LoadRegFromOffsetReg(i, step, addr,  OP_A_I(data), OP_A_O(data), 0x2)) break;
			break;
		case LDAI_B: //		0xE5 //Load from DS + B in MAR to A
			if(!LoadRegFromOffsetReg(i, step, addr, OP_A_I(data), OP_B_O(data), 0x2)) break;
			break;
		case LDAI_C: //		0xE6 //Load from DS + C in MAR to A
			if(!LoadRegFromOffsetReg(i, step, addr, OP_A_I(data), OP_C_O(data), 0x2)) break;
			break;
		case LDAI_D: //		0xE7 //Load from DS + D in MAR to A
			if(!LoadRegFromOffsetReg(i, step, addr, OP_A_I(data), OP_D_O(data), 0x2)) break;
			break;
					/* Load from Memory using offset in register  - to B */
		case LDBI_A: //		0xE8 //Load from DS + A in MAR to B
			if(!LoadRegFromOffsetReg(i, step, addr, OP_B_I(data), OP_A_O(data), 0x2)) break;
			break;
		case LDBI_B: //		0xE9 //Load from DS + B in MAR to B
			if(!LoadRegFromOffsetReg(i, step, addr, OP_B_I(data), OP_B_O(data), 0x2)) break;
			break;
		case LDBI_C: //	0xEA //Load from DS + C in MAR to B
			if(!LoadRegFromOffsetReg(i, step, addr, OP_B_I(data), OP_C_O(data), 0x2)) break;
			break;
		case LDBI_D: //	0xEB
			if(!LoadRegFromOffsetReg(i, step, addr, OP_B_I(data), OP_D_O(data), 0x2)) break;
			break;
		
		/* Load from Memory using offset in register  - to C */
		case LDCI_A: //		0xEC //Load from DS + A in MAR to C
			if(!LoadRegFromOffsetReg(i, step, addr, OP_C_I(data), OP_A_O(data), 0x2)) break;
			break;
		case LDCI_B: //		0xED //Load from DS + B in MAR to C
			if(!LoadRegFromOffsetReg(i, step, addr, OP_C_I(data), OP_B_O(data), 0x2)) break;
			break;
		case LDCI_C: //	0xEE //Load from DS + C in MAR to C
			if(!LoadRegFromOffsetReg(i, step, addr, OP_C_I(data), OP_C_O(data), 0x2)) break;
			break;
		case LDCI_D: //	0xEF
			if(!LoadRegFromOffsetReg(i, step, addr, OP_C_I(data), OP_D_O(data), 0x2)) break;
			break;
		/*Save to memory with offest in output register, to input register */
		case SDAI_A: //		0xF0 //Load from DS + A in MAR to A
			if (!SaveRegFromOffsetReg(i, step, addr, OP_A_I(data), OP_A_O(data), 0x2)) break;
			break;
		case SDAI_B: //		0xF1 //Load from DS + B in MAR to A
			if (!SaveRegFromOffsetReg(i, step, addr, OP_A_I(data), OP_B_O(data), 0x2)) break;
			break;
		case SDAI_C: //		0xF2 //Load from DS + C in MAR to A
			if (!SaveRegFromOffsetReg(i, step, addr, OP_A_I(data), OP_C_O(data), 0x2)) break;
			break;
		case SDAI_D: //		0xF3 //Load from DS + D in MAR to A
			if (!SaveRegFromOffsetReg(i, step, addr, OP_A_I(data), OP_D_O(data), 0x2)) break;
			break;

		case HLT:
			//Serial.print("HLT -- ");
			OP_CLEAR(data);
			OP_Hlt(data);
			addr = step++ << 8 | i; //t3
			if (!programmer.WriteShort(addr, data))
				break;
		case 0x1F:
			//undefined op code
			if(!Nop(step, addr | i)) break;
			break;
		case NOP:
		default:
			//if(!(addr & 0x1 << 6)) Nop(step, addr);	 //avoid out command
			break;
		}

		counter += (step * 2);// add number of steps * 2 (since each step is a word) to counter

	}
	Serial.print(F("Wrote ")); Serial.print(counter); Serial.println(F(" bytes of microinstructions"));
}


#pragma region Bulk Programming Helpers

bool ControlRom::Fetch(uint16_t &step, uint16_t addr) {
	/*   Fetch Cycle    */
	// Ep + Lm  t1
	uint16_t data = 0x0;
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_SEG(data, 0x0); // Code Segment
	OP_M_L(data);
	//data = 0x2800;
	addr = step++ << 8 | addr & 0xFF;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//Cp + Ce + Li  t2
	OP_CLEAR(data);
	OP_PC_C(data);
	OP_SEG(data, 0x0); // Code Segment
	OP_M_O(data);
	OP_IR_I(data);
	addr = step++ << 8 | addr & 0xFF;
	if (!programmer.WriteShort(addr, data))
		return false;

	return true;
}

//Load next byte, put into A or other dest
bool ControlRom::LoadMem(uint16_t &step, uint16_t addr) {
	uint8_t outTo = (addr & 0x7); //get highest 5 bytes, use for output
								  //Serial.print("********     Out to: 0x"); Serial.print(outTo, HEX); Serial.print("  ********     ");
	uint16_t data = 0x0;
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_SEG(data, 0x0); // Code Segment, since we want to load the next value in code, not data segment.
	OP_M_L(data);
	//data = 0x2800;
	addr = step++ << 8 | addr & 0xFF;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//Cp + Ce + Li  t2
	OP_CLEAR(data);
	OP_PC_C(data);
	OP_M_O(data);
	switch (outTo) {
	case 0x0:
		OP_A_I(data);
		//Serial.println("Write to A");
		break;
	case 0x1:
		OP_B_I(data);
		//Serial.println("Write to B");
		break;
	case 0x2:
		OP_C_I(data);
		//Serial.println("Write to C");
		break;
	case 0x3:
		OP_D_I(data);
		//Serial.println("Write to D");
		break;
	case 0x4:
		OP_F_I(data);
		//Serial.println("Write to F");
		break;
	case 0x5:
		OP_PC_I(data);
		//Serial.println("Write to PC");
		break;
	case 0x6:
		OP_IR_I(data);
		//Serial.println("Write to IR");
		break;
	case 0x7:
		OP_M_L(data);
		//Serial.println("Write to Mem");
		break;
	default:
		Serial.println("Attempted to load invalid memory");
		return false;
		break;
	}
	addr = step++ << 8 | addr & 0xFF | outTo;
	if (!programmer.WriteShort(addr, data))
		return false;

	return true;
}

//Load next byte, put into A or other dest
bool ControlRom::SumTo(uint16_t &step, uint16_t addr) {
	uint8_t outTo = (addr >> 4 & 0x0F); //get bytes 7 - 4, use for output
	Serial.print("********     Out to: 0x"); Serial.print(outTo, HEX); Serial.print("  ********     ");
	uint16_t data = 0x0;
	OP_CLEAR(data);
	OP_SEG(data, 0x0); // Code Segment
	OP_PC_O(data);
	OP_M_L(data);
	//data = 0x2800;
	addr = step++ << 8 | addr & 0xFF;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//Cp + Ce + Li  t2
	OP_CLEAR(data);
	OP_PC_C(data);
	OP_M_O(data);
	OP_IR_I(data);
	addr = step++ << 8 | addr & 0xFF;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//and now based on address, we write to the selected register from output

	OP_CLEAR(data);
	OP_U_O(data);
	switch (outTo) {
	case 0x0:
		OP_A_I(data);
		Serial.println("Write to A");
		break;
	case 0x1:
		OP_B_I(data);
		Serial.println("Write to B");
		break;
	case 0x2:
		OP_C_I(data);
		Serial.println("Write to C");
		break;
	case 0x3:
		OP_D_I(data);
		Serial.println("Write to D");
		break;
	case 0x4:
		OP_F_I(data);
		Serial.println("Write to F");
		break;
	case 0x5:
		OP_PC_I(data);
		Serial.println("Write to PC");
		break;
	case 0x6:
		OP_IR_I(data);
		Serial.println("Write to IR");
		break;
	case 0x7:
		OP_M_L(data);
		Serial.println("Write to Mem");
		break;
	default:
		Serial.println("Attempted to write invalid memory");
		return false;
		break;
	}
	addr = step++ << 8 | addr & 0xFF;
	if (!programmer.WriteShort(addr, data))
		return false;

	return true;
}

bool ControlRom::Nop(uint16_t &step, uint16_t addr, uint16_t d) {
	uint16_t data = 0x0;
	OP_NOP(data);
	addr = step++ << 8 | (addr & d);
	if (!programmer.WriteShort(addr, data))
		return false;

	return true;
}

bool ControlRom::PC_to_IR(uint16_t &step, uint16_t addr) {
	uint16_t data = 0;
	OP_CLEAR(data);
	OP_SEG(data, 0x0); // Code Segment
	OP_PC_O(data);
	OP_IR_I(data);
	addr = step++ << 8 | (addr & 0xFF);
	if (!programmer.WriteShort(addr, data))
		return false;
	return true;
}

bool ControlRom::ConditionalJump(uint16_t &step, uint16_t addr, uint8_t conditionType) {
	uint16_t data = 0x0;
	//Serial.print("CJ -- Received Addr: 0x"); Serial.print(addr, HEX); Serial.print(" ");
	//load next byte into PC, increment PC on falling edge
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_PC_C(data);
	OP_M_L(data);
	OP_SEG(data, 0x0); // Code Segment
					   //data | conditionType << 4;
	addr = step++ << 8 | addr & 0xFF; //t3
	if (!programmer.WriteShort(addr, data))
		return false;
	//load byte into 
	OP_CLEAR(data);
	OP_PC_JC(data);
	data |= (conditionType << 0x4);
	OP_M_O(data);
	addr = step++ << 8 | addr & 0xFF;// | conditionType; //t3
	if (!programmer.WriteShort(addr, data))
		return false;
	//if jump did not occur
	//increment pc in conditional logic unit
	//should jump so no need for nop, but what the hey, let's make it rain NOPs
	return Nop(step, addr & 0xFF);
}


#pragma region Register and Stack Methods

//Loads register specified by input parameter with value at address which is stored in next byte.
bool ControlRom::LoadReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t input) {
	uint16_t data = 0x0;
	//get address of data memory location [CS + IP]
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_M_L(data);
	OP_SEG(data, 0x0); // Code Segment
	
	//data = 0x2800;
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}

	//Get Address of data [DS + MO]
	OP_CLEAR(data);
	OP_M_L(data);
	OP_PC_C(data);
	OP_M_O(data);
	OP_SEG(data, 0x2); // Data Segment
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data))
		return false;

	//store data in register specified by input parameter
	OP_CLEAR(data);
	data |= input;
	OP_M_O(data);
	OP_SEG(data, 0x2); // Data Segment
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data))
		return false;
	

	return Nop(step, addr | opcode);
}
//Store value from register to memory location. memory location is specified in the next byte
bool ControlRom::StoreReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t output) {
	uint16_t data = 0x0;

	//Serial.print("Store from register");
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_SEG(data, 0x0); //Code Segment
	OP_M_L(data);
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}

	//latch address to store data [DS + Mem]
	OP_CLEAR(data);
	OP_PC_C(data);
	OP_M_O(data);
	OP_M_L(data);
	OP_SEG(data, 0x2); //Data Segment
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data))
		return false;

	//store from register to address
	OP_CLEAR(data);
	OP_M_I(data);
	OP_SEG(data, 0x2); //Data Segment
	data |= output;
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data))
		return false;
	
	return Nop(step, addr | opcode);
}
//used to load from DS + offset (from input reg ). result to output reg
bool ControlRom::LoadRegFromOffsetReg(uint8_t opcode, uint16_t & step, uint16_t & addr, uint16_t input, uint16_t output, uint8_t segment)
{
	uint16_t data = 0x0;
	//get address of data memory location [DS + input]
	OP_CLEAR(data);
	data |= output;
	OP_M_L(data);
	OP_SEG(data, segment); // Data Segment by default

					   //data = 0x2800;
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}

	
	//store data in register specified by input parameter
	OP_CLEAR(data);
	data |= input;
	OP_M_O(data);
	OP_SEG(data, 0x2); // Data Segment
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data))
		return false;


	return Nop(step, addr | opcode);
}
bool ControlRom::SaveRegFromOffsetReg(uint8_t opcode, uint16_t & step, uint16_t & addr, uint16_t dataRegister, uint16_t offsetRegister, uint8_t segmentRegister)
{
	uint16_t data = 0x0;
	//latch address of data memory location [DS + input]
	OP_CLEAR(data);
	data |= offsetRegister;
	OP_M_L(data);
	OP_SEG(data, segmentRegister); // Data Segment by default

	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}


	//store data in memory specified by input parameter
	OP_CLEAR(data);
	data |= dataRegister;
	OP_M_I(data);
	OP_SEG(data, 0x2); // Data Segment
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data))
		return false;


	return Nop(step, addr | opcode);
}
bool ControlRom::Call(uint8_t opcode, uint16_t & step, uint16_t & addr, uint8_t condition)
{
	return true;
	uint16_t data = 0x0;
	//if far call
	if (condition > 0x0) {	
		OP_CLEAR(data);
		OP_SEG(data, 0x1); //stack segment
		OP_SP_O(data);
		OP_M_L(data);
		//latch address at mar
		addr = step++ << 8 | opcode;
		if (!programmer.WriteShort(addr, data)) {
			return false;
		}
		//Save CS to memory
		OP_CLEAR(data);
		OP_SEG(data, 0x0); // code segment
		OP_M_I(data);
		OP_SR_O(data);
		addr = step++ << 8 | opcode; //t3
		if (!programmer.WriteShort(addr, data))
			return false;

		//decrement counter
		OP_CLEAR(data);
		OP_SP_DEC(data);
		addr = step++ << 8 | opcode; //t3
		if (!programmer.WriteShort(addr, data))
			return false;

	}

	//Store current program counter to stack
	//latch stack address
	OP_CLEAR(data);
	OP_SEG(data, 0x1); //stack segment
	OP_SP_O(data);
	OP_M_L(data);
	//latch address at mar
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//save program counter to stack
	OP_CLEAR(data);
	OP_M_I(data);
	OP_PC_O(data);
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;

	//decrement program counter
	OP_CLEAR(data);
	OP_SP_DEC(data);
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;

	/*******************************/
	/* Done storing current location to stack. Next get new location*/
	//get next byte in memory, store to CS

	if (condition > 0x0) // far
	{
		//get address of data memory location [CS + PC]
		OP_CLEAR(data);
		OP_PC_O(data);
		OP_M_L(data);
		OP_SEG(data, 0x0); // Code Segment

						   //data = 0x2800;
		addr = step++ << 8 | opcode;
		if (!programmer.WriteShort(addr, data)) {
			return false;
		}

		//Get Address of data [DS + MO]
		OP_CLEAR(data);
		OP_M_L(data);
		OP_PC_C(data);
		OP_M_O(data);
		OP_SEG(data, 0x2); // Data Segment
		addr = step++ << 8 | opcode;
		if (!programmer.WriteShort(addr, data))
			return false;


		//store data in c for now
		OP_CLEAR(data);
		OP_C_I(data);
		OP_PC_C(data);
		OP_M_O(data);
		addr = step++ << 8 | opcode;
		if (!programmer.WriteShort(addr, data))
			return false;

	}

	//get address of next jump to location [CS + PC]
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_M_L(data);
	OP_SEG(data, 0x0); // Code Segment

	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}

	//Output location to program counter
	OP_CLEAR(data);
	OP_PC_I(data);	
	OP_M_O(data);
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	if (condition > 0) //far
	{
		//had to use C register to hold value for segment. put it there
		OP_CLEAR(data);
		OP_SR_I(data);
		OP_C_O(data);
		OP_SEG(data, 0x2); // Data Segment
		addr = step++ << 8 | opcode;
		if (!programmer.WriteShort(addr, data))
			return false;
	}

	return Nop(step, addr | opcode);
}

bool ControlRom::Ret(uint8_t opcode, uint16_t & step, uint16_t & addr, uint8_t condition)
{
	uint16_t data = 0x0;

	//increment stack pointer
	OP_CLEAR(data);
	OP_SP_INC(data);
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;


	//Get program counter from  stack
	//latch stack address
	OP_CLEAR(data);
	OP_SEG(data, 0x1); //stack segment
	OP_SP_O(data);
	OP_M_L(data);
	//latch address at mar
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//output program counter from stack
	OP_CLEAR(data);
	OP_M_O(data);
	OP_PC_I(data);
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;

	
	//if far call
	if (condition > 0x0) {

		//increment Stack Pointer
		OP_CLEAR(data);
		OP_SP_INC(data);
		addr = step++ << 8 | opcode; //t3
		if (!programmer.WriteShort(addr, data))
			return false;


		OP_CLEAR(data);
		OP_SEG(data, 0x1); //stack segment
		OP_SP_O(data);
		OP_M_L(data);
		//latch address at mar
		addr = step++ << 8 | opcode;
		if (!programmer.WriteShort(addr, data)) {
			return false;
		}
		//Get CS from memory
		OP_CLEAR(data);
		OP_SEG(data, 0x0); // code segment
		OP_M_O(data);
		OP_SR_I(data);
		addr = step++ << 8 | opcode; //t3
		if (!programmer.WriteShort(addr, data))
			return false;
	}

	//increment to next byte in code
	OP_CLEAR(data);
	OP_PC_C(data);
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;


	return Nop(step, addr | opcode);
}







//Push register onto stack. Decrement stack pointer after.
bool ControlRom::PushReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t output, uint8_t segment ) {
	uint16_t data = 0x0;
	OP_SEG(data, 0x1); //stack segment
	OP_SP_O(data);
	OP_M_L(data);
	//latch address at mar
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//write from segment or register to memory
	OP_CLEAR(data);
	OP_SEG(data, segment); // from variable
	OP_M_I(data);
	data |= output;
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;
	
	OP_CLEAR(data);
	OP_SP_DEC(data);
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;

	return Nop(step, addr);
}
//Pop register from stack. Increment stack pointer first.
bool ControlRom::PopReg(uint8_t opcode, uint16_t& step, uint16_t &addr, uint16_t input, uint8_t segment) {
	uint16_t data = 0x0;

	OP_CLEAR(data);
	OP_SP_INC(data);
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;


	OP_SP_O(data);
	OP_SEG(data, 0x1); //reading data from stack segment : addr
	OP_M_L(data);
	//latch address at mar
	addr = step++ << 8 | opcode;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	//write from memory to desired register or segment
	OP_CLEAR(data);
	OP_M_O(data);
	OP_SEG(data, segment); // from variable
	data |= input;
	addr = step++ << 8 | opcode; //t3
	if (!programmer.WriteShort(addr, data))
		return false;

	
	return Nop(step, addr);
}
//Move data from one register to another, including segment
bool ControlRom::MovReg(uint8_t opcode, uint16_t & step, uint16_t & addr, uint16_t input, uint16_t output, uint8_t segment)
{
	uint16_t data = 0x0;
	OP_CLEAR(data);
	OP_SEG(data, segment); // from variable
	data |= (input & 0x3FF) | (output & 0x3FF);
	addr = step++ << 8 | opcode & 0xFF;
	if (!programmer.WriteShort(addr, data)) {
		return false;
	}
	
	return Nop(step, addr);
}
#pragma endregion

#pragma endregion

