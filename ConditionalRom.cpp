#include "ConditionalRom.h"
#include "EEPROM.h"
extern EEPROM programmer;



ConditionalRom::ConditionalRom()
{
}


ConditionalRom::~ConditionalRom()
{
}


//stores data to conditional logic rom
void ConditionalRom::StoreConditionalLogicData() {
	Serial.println(F("Storing conditional logic (JUMP) data"));
	uint16_t addr;
	uint16_t data = 0xFF;
	//unsigned
	for (uint16_t addr = 0; addr < 0x800; addr++) {
		if (addr % 128 == 0) Serial.print(".");
		
		//Decode address into convenient variables
		//increment counter signal in (bit 4)
		bool PC_Inc = addr & (0x1 << 4);
		//jump signal in (bit 5)
		bool PC_Input = addr & (0x1 << 5);
		//nop signal in (bit 6)
		bool Nop_In = addr & (0x1 << 6);
		//conditional signal in (bit 7)
		bool CondJump = addr & (0x1 << 7);

		//flags received
		//subtract flag from ALU (bit 8)
		bool SubFlag = addr & (0x1 << 8);
		//carry flag from ALU (bit 9)
		bool CarryFlag = addr & (0x1 << 9);
		//zero flag from ALU (bit 10)
		bool ZeroFlag = addr & (0x1 << 10);
		//sign flag from subtract and carry op
		bool SignFlag = SubFlag && CarryFlag;


		//operations to perform
		uint8_t _jump = 0x1 << 7;
		uint8_t _nop = 0x1 << 1;
		uint8_t _count = 0x1 << 0;

		//set flags for data out
		uint8_t dataOut = 0x0;
		dataOut |= SubFlag << 4;
		dataOut |= ZeroFlag << 3;
		dataOut |= CarryFlag << 2;

		//first handle unconditional cases (coming from master control rom)
		if (PC_Inc && !CondJump) {
			dataOut |= _count;
			programmer.WriteByte(addr, dataOut);
			//Serial.print("Handing unc increment case at address 0x"); Serial.println(addr, HEX);
			continue;
		}
		if (PC_Input) {
			dataOut |= _jump;
			programmer.WriteByte(addr, dataOut);
			//Serial.print("Handing unc jump case at address 0x"); Serial.println(addr, HEX);
			continue;
		}
		if (Nop_In) {
			dataOut |= _nop;
			programmer.WriteByte(addr, dataOut);
			//Serial.print("Handing unc nop case at address 0x"); Serial.println(addr, HEX);
			continue;
		}

		//and now the conditional cases, get first 3 bytes
		uint8_t conditionFlag = addr & 0x7;

		//if we have been asked to make a conditional jump, either jump or nop it
		if (CondJump) {
			//Serial.print("Read condition flag: "); Serial.print(conditionFlag); Serial.print("  -  ");
			switch (conditionFlag)
			{
			case 0x0:	// LT(Address source in IR) SUB, carry flag, no zero flag
				if (SubFlag && CarryFlag && !ZeroFlag) {
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional LT case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 0x1: // LTE(Address source in IR) SUB, carry flag, zero flag
				if (SubFlag && CarryFlag && ZeroFlag) {
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional LTE case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 0x2: //	EQ(Address source in IR) SUB, zero flag
				if (SubFlag && ZeroFlag) {
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional EQ case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 3:// 	GT(Address source in IR) SUB, no flag
				if (SubFlag && !CarryFlag && !ZeroFlag && !SignFlag) {
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional GT case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 4:// 	GTE(Address source in IR) SUB, no flag or zero flag
				if ((SubFlag && !CarryFlag && !SignFlag) || (!SubFlag && !CarryFlag && !ZeroFlag)) {
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional GTE case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 5:// 	CF(Address source in IR) SUB, carry flag
				if (CarryFlag) {
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional CF case at address 0x"); Serial.println(addr, HEX);
					continue;
				}

				break;
			case 6:// 	ZF(Address source in IR) SUB, zero flag
				if (ZeroFlag) {
					//Serial.print("Processing zero flag at address 0x"); Serial.print(addr);				
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional ZF case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 7:// 	NZF(Address source in IR) SUB, zero flag
				if (!ZeroFlag) {
					programmer.WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional NZF case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			default:
				break;
			}
			//if we are here, nop it
			programmer.WriteByte(addr, dataOut | _nop);
			//Serial.print("Handing conditional  nop case at address 0x"); Serial.println(addr, HEX);
			continue;
		}

		//otherwise, we should not do anything, all outputs stay low
		//Serial.print("Handing no command/branch case at address 0x"); Serial.println(addr, HEX);
		programmer.WriteByte(addr, dataOut);
	}
}
