/******************************************************************
	Name: 28C16_Programmer
	Date: 2017-12-21
	Author:Lev Vayner
	Description: Arduino code (mega or due footprint) for programming
		and inspecting EEPROMs needed for 8 bit computer.
		8 bit computer design was inspired by Ben Eater (You Tube)
*/
//TODO: if time is laying around, write code to detect number of chips, in a non-violatile kind of manner, for now prompt user at start up
#pragma region Pins

//address pins
#define PIN_ADDR0	23
#define PIN_ADDR1	25
#define PIN_ADDR2	27
#define PIN_ADDR3	29
#define PIN_ADDR4	31
#define PIN_ADDR5	33
#define PIN_ADDR6	35
#define PIN_ADDR7	37
#define PIN_ADDR8	39
#define PIN_ADDR9	41
#define PIN_ADDR10	43

//data pins
#define PIN_DATA0	22
#define PIN_DATA1	24
#define PIN_DATA2	26
#define PIN_DATA3	28
#define PIN_DATA4	30
#define PIN_DATA5	32
#define PIN_DATA6	34
#define PIN_DATA7	36
#define PIN_DATA8	38
#define PIN_DATA9	40
#define PIN_DATA10	42
#define PIN_DATA11	44
#define PIN_DATA12	46
#define PIN_DATA13	48
#define PIN_DATA14	50
#define PIN_DATA15	52


//control pins
#define PIN_CE		49	//ACTIVE LOW	- pull down to use chip.
#define PIN_WE		51	//ACTIVE LOW	- pull down for 100ns - 1000ns *AFTER* setting address and data. OE should be HIGH. CE should be HIGH.
#define PIN_OE		53	//ACTIVE LOW	- pull down *AFTER* setting address and data. WE should be HIGH. CE should be HIGH.

#pragma endregion

uint8_t chipCount = 2;
#define DEBUG	1
//states
enum DeviceState {
	dsOff = 0,
	dsRead = 1,
	dsWrite = 2
};
DeviceState promState;

/*
	ROM based on control codes laid out as follows(from bit 15 to bit 0)

	TBD TBD TBD [C1_b2] [C1_b1] [C1_b0] [R] [W] [S3] [S2] [S1] [S0] [D3] [D2] [D1] [D0]

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
		1001 - ST (Stack Pointer Register)
		1010 - IR (Instruction Register)
		1011 - MAR (Read data from device using address in IR?!?)
		1100 - CS (Code Segment Register)
		1101 - DS (Data Segment Register)
		1110 - SS (Stack Segment Register)
		1111 - ES (Extra Segment Register)

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
		1001 - ST (Stack Pointer Register)
		1010 - IR (Instruction Register)
		1011 - MAR (Read data from device using address in IR?!?)
		1100 - CS (Code Segment Register)
		1101 - DS (Data Segment Register)
		1110 - SS (Stack Segment Register)
		1111 - ES (Extra Segment Register)
*/
#define READ_IN_FLAG_IDX 0x9
#define WRITE_OUT_FLAG_IDX 0x8
#define OP_CLEAR(var) var = 0x0  
#define C_C1(var) var &= 0xE3FF //clear C1 flags in var
//use S_C1 to set
#define S_C1(var,x)  var =( C_C1(var)) |  x << 10 //clear bits and set per above order


/* Registers, set read flag and bits 4-7 multiplexed for read, write flag and bits 0-3*/
#define C_R(var) var &= 0xFD0F //clear Read flag and index
#define C_W(var) var &= 0xFEF0 //clear Write flag and index
#define S_R(var,x) var = (C_R(var)) | 1 << READ_IN_FLAG_IDX | x << 4 // clear, set read flag, and item
#define S_W(var,x) var = (C_W(var)) | 1 << WRITE_OUT_FLAG_IDX | x << 0 // clear, set read flag, and item
/* Now that convenience methods are done, instructions to be called by humans.*/




#define OP_Hlt(var)	 var = (S_C1(var,1)) & 0x1C00 //Halt, exclusive
#define OP_NOP(var)	 var = (S_C1(var,2)) & 0x1C00//No operation, exclusive
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
/* Stack */
#define OP_ST_I(var)	S_R(var,9)
#define OP_ST_O(var)	S_W(var,9)

/* Instrcution Register */
#define OP_IR_I(var)	S_R(var,10)
#define OP_IR_O(var)	S_W(var,10)
/* MAR Address Load and Out*/
#define OP_M_L(var)		S_R(var,11)
#define OP_M_O(var)		S_W(var,11)

#define OP_CS_I(var)	S_R(var,12)
#define OP_CS_O(var)	S_W(var,12)
#define OP_DS_I(var)	S_R(var,13)
#define OP_DS_O(var)	S_W(var,13)
#define OP_SS_I(var)	S_R(var,14)
#define OP_SS_O(var)	S_W(var,14)
#define OP_ES_I(var)	S_R(var,15)
#define OP_ES_O(var)	S_W(var,15)

#pragma endregion
#pragma region Lookups

//data
byte digits[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00 };

uint8_t ReadWriteFlag = 0x0;
uint8_t SourceDestMask = 0x0;
//opcodes

#define NOP					0x00 //nothing to do, just passing time
#define LDI		 			0x80 // load immediate	  (value in next byte)
#define LDA					0x01 // load from address (address in next byte) 
#define LDB					0x02 // load from base register + offset (register in next byte)
#define LDC					0x03 // load from base register + offset (register in next byte)
#define LDD					0x04 // Load from D
#define LDF					0x05 // Load from F
#define SDA					0x06 // write a (to address in next byte)
#define MOV_REG_REG			0x07 // move A to B, C to D
#define MOV_REG_MEM			0x08 // Move to registry in next bye, address from byte after
#define MOV_MEM_REG			0x09 // Move to memory address in next byte, from register in byte after
#define MOV_MEM_MEM			0x0A //	Move from memory address to memory address (uses A register as temp)
#define MOV_REG_IMM 		0x0B // Move to register in next byte, value in byte after
#define MOV_MEM_IMM 		0x0C // Place an imm. value in memory. Address in next byte. value in byte after Like if I want all executables to start with 'MZ'
#define ADD					0x0D //Add B to A, store to A, registry being added to A in next byte
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
#define OUT			0x1A //Output content of A to decimal display (Output register 1)
#define HLT			0x1B //Terminate the program


#define SCF			0x1C //Set Carry Flag
#define CCF			0x1D //Clear Carry Flag

#define SZF			0x1E // Set Zero Flag
#define CZF			0x1F // Clear Zero Flag



#pragma region Future place of code build assister thing
typedef enum destMask {
	dmA = 0x0,
	dmB = 0x1,
	dmC = 0x2,
	dmD = 0x3,
	dmF = 0x4,
	dmPC = 0x5,
	dmIR = 0x6,
	dmM = 0x7,

};/* used by one byte commands (with second byte optional operand)*/
typedef enum ParamOptions {
	poZero = 0,
	//poZeroOrOne = 1,
	//poZeroOrOneOrTwo = 2,
	poOne = 3,
	//poOneOrTwo = 4,
	poTwo = 5
};
typedef enum readIntoCodes {
	riA = 0x0,
	riB = 0x1,
	riC = 0x2,
	riD = 0x3,
	riF = 0x4,
	riOut = 0x5,
	ri_BIT_SIX_TBD = 0x6,
	riMI = 0x7,
	riPC = 0x8,
	riST = 0x9,
	riIR = 0xA,
	riMAR= 0xB,
	riCS = 0xC,
	riDS = 0xD,
	riSS = 0xE,
	riES = 0xF,
};

typedef enum writeOutCodes {
	woA = 0x0,
	woB = 0x1,
	woC = 0x2,
	woD = 0x3,
	woF = 0x4,
	woIn = 0x5,
	woALU = 0x6,
	wo_BIT_SEVEN_TBD = 0x7,
	woPC = 0x8,
	woST = 0x9,
	woIR = 0xA,
	woMAR = 0xB,
	woCS = 0xC,
	woDS = 0xD,
	woSS = 0xE,
	woES = 0xF,
};

#pragma endregion

#pragma region EEPROM Chip Methods

void ModeOff() {
	digitalWrite(PIN_WE, HIGH);
	digitalWrite(PIN_OE, HIGH);
	digitalWrite(PIN_CE, HIGH);

	for (int pin = PIN_ADDR0; pin <= PIN_ADDR10; pin += 2)
		pinMode(pin, OUTPUT);

	promState = dsOff;
}
void ModeOutput() {
	if (promState == dsWrite) //if alredy in write mode, skip
	{
		//Serial.println("Already in write mode, skip setting mode");
		return;
	}

	if (promState == dsOff) {
		digitalWrite(PIN_CE, LOW); delay(1);
	} //make sure chip is enabled

	for (int pin = PIN_DATA0; pin <= PIN_DATA15; pin += 2)
		pinMode(pin, OUTPUT);

	digitalWrite(PIN_WE, HIGH);
	digitalWrite(PIN_OE, HIGH);

	promState = dsWrite; //update state

}
void ModeInput() {
	if (promState == dsRead) { //if alredy in read mode, skip
		//Serial.println("Already in read mode, skip setting mode");
		return;
	}
	if (promState == dsOff) {
		digitalWrite(PIN_CE, LOW); delay(1);
	} //make sure chip is enabled
	digitalWrite(PIN_OE, LOW);

	for (int pin = PIN_DATA0; pin <= PIN_DATA15; pin += 2)
		pinMode(pin, INPUT);

	digitalWrite(PIN_WE, HIGH);


	promState = dsRead; //update state
}
//MAX ADDR is 2048 with 11 address lines
void SetAddress(uint16_t addr) {
	//each bit of address is going to addr pin 0 - 10. 
	for (int pinAddr = PIN_ADDR0; pinAddr <= PIN_ADDR10; pinAddr += 2)
		digitalWrite(pinAddr, addr & 0x1 << ((pinAddr - PIN_ADDR0) / 2));

}
void SetDataLines(uint8_t data, uint8_t byteNum = 1) {
	if (byteNum == 1) {
		for (int pinData = PIN_DATA0; pinData <= PIN_DATA7; pinData += 2)
		{
			digitalWrite(pinData, data & 0x1 << ((pinData - PIN_DATA0) / 2));

		}
	}
	else if (byteNum == 2) {
		for (int pinData = PIN_DATA8; pinData <= PIN_DATA15; pinData += 2)
		{
			/*if (data == 0x0) {
				Serial.print("Writing zero, bit "); Serial.print((pinData - PIN_DATA8) / 2); Serial.print(" value ");
				Serial.println(data & 0x1 << ((pinData - PIN_DATA8) / 2));
			}*/
			digitalWrite(pinData, data & 0x1 << ((pinData - PIN_DATA8) / 2));

		}
	}
}

#pragma endregion

#pragma region Value Read/Write

//construct byte from data bits
uint8_t ReadByte(uint16_t addr, uint8_t chipNumber = 1) {
	ModeInput();
	SetAddress(addr);
	delay(1);
	uint8_t readValue = 0;
	switch (chipNumber)
	{
	case 1:
		for (int pinData = PIN_DATA0; pinData <= PIN_DATA7; pinData += 2)
			readValue |= digitalRead(pinData) << ((pinData - PIN_DATA0) / 2);
		break;

	case 2:
		for (int pinData = PIN_DATA8; pinData <= PIN_DATA15; pinData += 2)
			readValue |= digitalRead(pinData) << ((pinData - PIN_DATA8) / 2);
		break;

	default:
		break;
	}

	delay(1);
	return readValue;
}
uint16_t counter = 0;
void WriteFirstByte(uint8_t data,uint16_t offsetAddress = 0) {
	counter = offsetAddress;
	WriteByte(counter, data);
}
void WriteNextByte(uint8_t data) {
	counter++;
	WriteByte(counter, data);
}
bool WriteByte(uint16_t addr,uint8_t data) {	
	ModeOutput();
	SetAddress(addr);
	SetDataLines(data);
	//toggle WE low for 100ns - 1000ns
	delay(1); //give 1ms for setup time
	digitalWrite(PIN_WE, LOW);
	delayMicroseconds(1);
	digitalWrite(PIN_WE, HIGH);
	

#if DEBUG
	if (data != 0) {
		uint8_t step = addr >> 8;
		//uint8_t opDest = addr & 0xFF >> 4; //top 3 bits
		uint8_t opCode = addr & 0x1F; //bottom 5 bits
		Serial.print(F("Address: 0x")); Serial.print(addr, HEX);
		Serial.print(F("\tData:["));Serial.print(data & 0xFF, BIN);
		/*if(opDest != 0)
		Serial.print(F("]  with destination [")); Serial.print(opDest, DEC);*/
		Serial.println(F("]"));
	}

#endif

	//verify
	ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
	uint8_t readByte = ReadByte(addr);
	
	return readByte == data;
}
bool WriteShort(uint16_t addr, uint16_t data) {
	ModeOutput();
	SetAddress(addr);
	SetDataLines(data);
	SetDataLines(data>>8,2);
	//toggle WE low for 100ns - 1000ns
	delay(1); //give 1ms for setup time
	digitalWrite(PIN_WE, LOW);
	delayMicroseconds(1);
	digitalWrite(PIN_WE, HIGH);

#if DEBUG
	if (data != 0) {
		uint8_t step = addr >> 8;
		//uint8_t opDest = addr & 0xFF >> 4; //top 3 bits
		uint8_t opCode = addr & 0xFF; //bottom 5 bits
		Serial.print(F("Address: 0x")); Serial.print(addr, HEX);
		Serial.print(F("\tOpcode:"));
		bool isCondJump = (addr & (0x5 << 10)) || (addr & (0x7 << 10));
		Serial.print(isCondJump ? "[C] " : "[NC] ");
		PrintOpCode(opCode, isCondJump); Serial.print(F(" at step ")); Serial.print(step, DEC);
		Serial.print(F("\tSetting Flags: ")); PrintOperations(data >> 8);
		Serial.print(F("\tData:["));
		Serial.print(data >> 8 & 0xFF, BIN); Serial.print("  ");
		Serial.print(data & 0xFF, BIN);
		/*if(opDest != 0)
			Serial.print(F("]  with destination [")); Serial.print(opDest, DEC);*/
		Serial.println(F("]"));
	}

#endif
	//verify
	//ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
	//uint8_t readByte = ReadByte(addr);

	return true;// readByte == data;
	//TODO: verify outpput
}
#pragma endregion

#pragma region UI'ish

bool ledState = 0;
unsigned long lastToggle = 0;
unsigned long toggleDuration = 500;

void blinkLED() {
	//has been long enough since last toggle
	if (millis() - lastToggle > toggleDuration)
	{
		ledState = !ledState; //togle state
		digitalWrite(PIN_LED, ledState); //update LED
		lastToggle = millis(); //update time
	}
}

bool needPrintMenu = true;
void PrintMenu() {
	if (!needPrintMenu) return;
	Serial.println(F("EE PROM TOOL   -   v 0.11134a.6"));
	Serial.println(F("--------------------------------"));
	Serial.println(F("Press r to read"));
	Serial.println(F("Press w to write"));
	Serial.println(F("Press p to print data"));
	Serial.println(F("Press s to store data"));
	Serial.println(F("Press n to specify chip count"));
	Serial.println(F("--------------------------------"));

	needPrintMenu = false;
}

void DumpROM() {
	Serial.print(F("        **************      LOW        ******************"));
	if (chipCount == 2) {
		Serial.println(F("        **************      HIGH       ******************"));
	}
	else Serial.println();

	for (int sector = 0; sector < 8; sector++) {
		for (int base = sector * 256; base <= sector * 256 + 255; base += 16) {
			byte data[16];
			for (int offset = 0; offset <= 15; offset += 1) {
				data[offset] = ReadByte(base + offset);
			}

			char buf[80];
			sprintf(buf, "%05x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
				base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
				data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
			Serial.print(buf);

			if (chipCount == 2) {
				byte data2[16];
				for (int offset = 0; offset <= 15; offset += 1) {
					data2[offset] = ReadByte(base + offset, 2);
				}
				
				sprintf(buf, "\t %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
					data2[0], data2[1], data2[2], data2[3], data2[4], data2[5], data2[6], data2[7],
					data2[8], data2[9], data2[10], data2[11], data2[12], data2[13], data2[14], data2[15]);
				Serial.print(buf);
			}
			Serial.println();
		}
		Serial.println();
	}
}

void ProcessInput() {
	char c = Serial.read();
	if (c == 'r' || c == 'R') {
		//read
		Serial.println("Enter address to read");
		delay(50);
		while (!Serial.available()) {
			blinkLED();
		}
		String addrS = Serial.readString();
		uint16_t addr = addrS.toInt();

		byte data = ReadByte(addr);
		Serial.print("Read: "); Serial.print(data); Serial.print(" from address "); Serial.print(addr, BIN);
		Serial.println();
		needPrintMenu = true;
	}
	else if (c == 'w' || c == 'W') {
		//write

		Serial.println("Enter address to write");
		delay(50);
		while (!Serial.available());
		String addrS = Serial.readString();
		uint16_t addr = addrS.toInt();

		Serial.println("Enter data to store in decimal form");
		delay(50);
		while (!Serial.available());
		String dataS = Serial.readString();
		byte data = dataS.toInt();

#ifdef DEBUG

		if (WriteByte(addr, data)) {
			Serial.print("Sucess writing data: 0x"); Serial.print(data, HEX); Serial.print(" to address "); Serial.println(addr, BIN);
		}
		else {
			Serial.print("Error writing data: 0x"); Serial.print(data, HEX); Serial.print(" to address "); Serial.println(addr, BIN);
		}

#endif

	}
	else if (c == 'p' || c == 'P') {
		DumpROM();

		Serial.println();
		needPrintMenu = true;
	}
	else if (c == 's' || c == 'S') {
		//store predetermined contents
		Serial.println("What type of data to store? ");
		Serial.println("\t1 for decimal counter");
		Serial.println("\t2 for Control ROM");
		Serial.println("\t3 for Program Code");
		Serial.println("\t4 for Conditional Logic ROM");
		delay(50);
		while (!Serial.available()) {
			blinkLED();
		}
		char dataType = Serial.read();
		switch (dataType)
		{
		case '1':
			StoreCounterData();
			break;
		case '2':
			StoreControlROMData();
			break;
		case '3':
			StoreProgramData();
			break;
		case '4':
			StoreConditionalLogicData();
			break;
		default:
			Serial.println("Invalid Selection");
			break;
		}

		Serial.println("Done!");

		Serial.println();
		needPrintMenu = true;
	}
	else if (c == 'n' || c == 'N') {
		GetChipCount();
	}
}

void GetChipCount() {
	Serial.println("How many chips will be dining with us today?");
	delay(50);
	while (!Serial.available()) {
		blinkLED();
	}
	String chipC = Serial.readString();
	chipCount = (uint8_t)chipC.toInt();
	Serial.print("Ok, assuming "); Serial.print(chipCount); Serial.println(" chips.. but only because you seem trustworthy"); //and programmer does not devote time to develop detect
}

void PrintOpCode(uint8_t opCode, bool condJump) {
	switch (opCode) {
	case 0:
		Serial.print(condJump ? " JLT " : " NOP ");
		break;
	case 1:
		Serial.print(condJump ? " JLTE " : " LDA ");
		break;
	case 2:
		Serial.print(condJump ? " JE " : " LDB ");
		break;
	case 3:
		Serial.print(condJump ? " JGT " : " LDC ");
		break;
	case 4:
		Serial.print(condJump ? " JGTE " : " LDD ");
		break;
	case 5:
		Serial.print(condJump ? " JC " : " LDF ");
		break;
	case 6:
		Serial.print(condJump ? " JZ " : " LDU ");
		break;
	case 7:
		Serial.print(condJump ? " JNZ " : " MOV_REG_REG ");
		break;
	case 8:
		Serial.print(" MOV_REG_MEM ");
		break;
	case 9:
		Serial.print(" MOV_MEM_REG ");
		break;
	case 0xA:
		Serial.print(" MOV_MEM_MEM ");
		break;

	case 0xB:
		Serial.print(" MOV_REG_IMM ");
		break;
	case 0xC:
		Serial.print(" MOV_MEM_IMM ");
		break;
	case 0xD:
		Serial.print(" ADD ");
		break;
	case 0xE:
		Serial.print(" ADD_IP ");
		break;
	case 0xF:
		Serial.print(" SUB ");
		break;
	case 0x10:
		Serial.print(" SUB_IP ");
		break;
	case 0x11:
		Serial.print(" JMP ");
		break;
	case 0x12:
		Serial.print(" JLT ");
		break;
	case 0x13:
		Serial.print(" JLE ");
		break;
	case 0x14:
		Serial.print(" JE ");
		break;
	case 0x15:
		Serial.print(" JG ");
		break;
	case 0x16:
		Serial.print(" JGTE ");
		break;
	case 0x17:
		Serial.print(" JC ");
		break;
	case 0x18:
		Serial.print(" JZ ");
		break;
	case 0x19:
		Serial.print(" JNZ ");
	case 0x1A:
		Serial.print(" OUT ");
		break;
	case 0x1B:
		Serial.print(" HLT ");
		break;
	case 0x1C:
		Serial.print(" SCF ");
		break;
	case 0x1D:
		Serial.print(" CCF ");
		break;
	case 0x1E:
		Serial.print(" SZF ");
		break;
	case 0x1F:
		Serial.print(" CZF ");
		break;
	case 0x80:
		Serial.print("LDI");
		break;
	}
}
void PrintOperations(uint8_t op) {
	if (op & 0x1) Serial.print(" WR ");
	if (op &(0x1 << 1)) Serial.print(" RD ");
	uint8_t subOp = op >> 2;
	switch (subOp)
	{
	case 0x0:
		Serial.print(" [-] ");
		break;
	case 0x1:
		Serial.print(" [HLT] ");
		break;
	case 0x2:
		Serial.print(" [NOP] ");
		break;
	case 0x3:
		Serial.print(" [PC+] ");
		break;
	case 0x4:
		Serial.print(" [SUB] ");
		break;
	case 0x5:
		Serial.print("[CJ] ");
		break;
	default:
		break;
	}

}

#pragma endregion

#pragma region Bulk Programming Helpers

bool Fetch(uint16_t &step, uint16_t addr) {
	/*   Fetch Cycle    */
	// Ep + Lm  t1
	uint16_t data = 0x0;
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_M_L(data);
	//data = 0x2800;
	addr = step++ << 8 | addr & 0xFF;
	if (!WriteShort(addr, data)) {
		return false;
	}
	//Cp + Ce + Li  t2
	OP_CLEAR(data);
	OP_PC_C(data);
	OP_M_O(data);
	OP_IR_I(data);
	addr = step++ << 8 | addr & 0xFF;
	if (!WriteShort(addr, data))
		return false;

	return true;
}

//Load next byte, put into A or other dest
bool LoadMem(uint16_t &step, uint16_t addr) {
	uint8_t outTo = (addr & 0x7); //get highest 5 bytes, use for output
	Serial.print("********     Out to: 0x"); Serial.print(outTo, HEX); Serial.print("  ********     ");
	uint16_t data = 0x0;
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_M_L(data);
	//data = 0x2800;
	addr = step++ << 8 | addr & 0xFF;
	if (!WriteShort(addr, data)) {
		return false;
	}
	//Cp + Ce + Li  t2
	OP_CLEAR(data);
	OP_PC_C(data);
	OP_M_O(data);
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
		Serial.println("Attempted to load invalid memory");
		return false;
		break;
	}
	addr = step++ << 8 | addr & 0xFF;
	if (!WriteShort(addr, data))
		return false;

	return true;
}

bool Nop(uint16_t &step, uint16_t addr) {
	uint16_t data = 0x0;
	OP_NOP(data);
	addr = step++ << 8 | (addr & 0xFF);
	if (!WriteShort(addr, data))
		return false;

	return true;
}

bool PC_to_IR(uint16_t &step, uint16_t addr) {
	uint16_t data = 0;
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_IR_I(data);
	addr = step++ << 8 | (addr & 0xFF);
	if (!WriteShort(addr, data))
		return false;
	return true;
}

bool ConditionalJump(uint16_t &step, uint16_t addr, uint8_t conditionType) {
	uint16_t data = 0x0;
	//Serial.print("CJ -- Received Addr: 0x"); Serial.print(addr, HEX); Serial.print(" ");
	//load next byte into PC, increment PC on falling edge
	OP_CLEAR(data);
	OP_PC_O(data);
	OP_PC_C(data);
	OP_M_L(data);
	//data | conditionType << 4;
	addr = step++ << 8 | addr & 0xFF; //t3
	if (!WriteShort(addr, data))
		return false;
	//load byte into 
	OP_CLEAR(data);
	OP_PC_JC(data);
	data |= (conditionType << 0x4);
	OP_M_O(data);
	addr = step++ << 8 | addr & 0xFF;// | conditionType; //t3
	if (!WriteShort(addr, data))
		return false;
	//if jump did not occur
	//increment pc in conditional logic unit
	//should jump so no need for nop, but what the hey, let's make it rain NOPs
	Nop(step, addr & 0xFF);
}

#pragma endregion

#pragma region Bulk Programming

void StoreCounterData() {
	Serial.println(F("Storing program counter data"));
	uint16_t addr;
	uint16_t data = 0xFF;
	//unsigned
	for (uint16_t i = 0; i < 4; i++) {
		uint16_t upaddr = i << 8;
		for (uint16_t num = 0; num < 256; num++) {
			addr = upaddr | num;
			uint16_t n1, n2, n3;
			n1 = num % 10;
			n2 = (num - n1) % 100 / 10;
			n3 = (num - n1 - (n2 * 10)) % 1000 / 100;
			data = i == 0 ? digits[n1] : i == 1 ? digits[n2] : digits[n3]; //select nibble
			if (i == 3)
				data = digits[10];
			//Serial.print("Storing "); Serial.print(data,HEX); Serial.print(" ("); Serial.print(data, BIN); Serial.print(") to address"); Serial.println(addr);
			delay(1);
			if (!WriteByte(addr, data)) {
				Serial.print(F("Failed to write at address: ")); Serial.print(addr);
				Serial.print(F("  data: ")); Serial.println(data, BIN);
			}
		}
	}
}
//stores data to conditional logic rom
void StoreConditionalLogicData() {
	Serial.println(F("Storing conditional logic (JUMP) data"));
	uint16_t addr;
	uint16_t data = 0xFF;
	//unsigned
	for (uint16_t addr = 0; addr < 0x800; addr++) {
		if (addr % 128 == 0) Serial.print(".");
		/*
		Address lines (INPUT):
		A0 - Control Data 0
		A1 - Control Data 1
		A2 - Control Data 2
		A3 - N/C
		A4 - Program Counter Increment (signal from control rom)
		A5 - Program Counter Input (signal from control rom,demuxed)
		A6 - Nop Input (signal from control rom, demuxed)
		A7 - Conditional Jump Flag (from control rom, demuxed)
		A8 - Substract flag (from F register)
		A9 - Carry flag (from F register)
		A10 - Zero Flag (from F register) 

		Data Lines (OUTPUT):
		D0 - PC + (Increment Counter)
		D1 - NOP
		D7 - PC_IN (Jump)
		D2 - Carry Flag (out)
		D3 - Zero Flag (out)
		-- not needed -- D4 - Subtract Flag (out)
		*/
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
		if (PC_Inc) {
			dataOut |= _count;
			WriteByte(addr, dataOut);
			//Serial.print("Handing unc increment case at address 0x"); Serial.println(addr, HEX);
			continue;
		}
		if (PC_Input) {
			dataOut |= _jump;
			WriteByte(addr, dataOut);
			//Serial.print("Handing unc jump case at address 0x"); Serial.println(addr, HEX);
			continue;
		}
		if (Nop_In) {
			dataOut |= _nop;
			WriteByte(addr, dataOut);
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
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional LT case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 0x1: // LTE(Address source in IR) SUB, carry flag, zero flag
				if (SubFlag && CarryFlag && ZeroFlag) {
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional LTE case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 0x2: //	EQ(Address source in IR) SUB, zero flag
				if (SubFlag && ZeroFlag) {
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional EQ case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 3:// 	GT(Address source in IR) SUB, no flag
				if (SubFlag && !CarryFlag && !ZeroFlag && !SignFlag) {
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional GT case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 4:// 	GTE(Address source in IR) SUB, no flag or zero flag
				if (SubFlag && !CarryFlag && !SignFlag) {
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional GTE case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			case 5:// 	CF(Address source in IR) SUB, carry flag
				if (CarryFlag) {
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional CF case at address 0x"); Serial.println(addr, HEX);
					continue;
				}

				break;
			case 6:// 	ZF(Address source in IR) SUB, zero flag
				if (ZeroFlag) {
					//Serial.print("Processing zero flag at address 0x"); Serial.print(addr);				
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional ZF case at address 0x"); Serial.println(addr, HEX);
					continue;
				}					
				break;
			case 7:// 	Unconditional(Address source in IR )
				if (!ZeroFlag) {
					WriteByte(addr, dataOut | _jump);
					Serial.print("Handing conditional NZF case at address 0x"); Serial.println(addr, HEX);
					continue;
				}
				break;
			default:
				break;
			}
			//if we are here, nop it
			WriteByte(addr, dataOut | _count | _nop);
			//Serial.print("Handing conditional  nop case at address 0x"); Serial.println(addr, HEX);
			continue;
		}
		
		//otherwise, we should not do anything, all outputs stay low
		//Serial.print("Handing no command/branch case at address 0x"); Serial.println(addr, HEX);
		WriteByte(addr, dataOut);
	}
}

void StoreControlROMData() {
	Serial.println(F("Storing Control ROM data *16 bit*"));

	if (chipCount < 2) {
		Serial.println(F("This function requires a minimum of two chips.\r\n Come back when you get more."));
		return;
	}
	Serial.print(F("First clear.."));
	for (int i = 0; i < 2048; i++) {
		WriteShort(i, 0x00);
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
		for (int dest = 0; dest < 8; dest++) {
			stepSub = step;
			Fetch(stepSub, dest | i);
			counter += stepSub; // manually augment our micro steps counter
		}
		step = stepSub;
		switch (i)
		{
			//case SDA:
			//	OP_CLEAR(data);
			//	OP_A_O(data);
			//	OP_M_I(data);
			//	addr = step++ << 8 | i; //t3
			//	if (!WriteShort(addr, data))
			//		break;
			//	
			//	OP_NOP(data);
			//	addr = step++ << 8 | i; //t4
			//	if (!WriteShort(addr, data))
			//		break;
			//	
			//	break;
		case LDI: //first byte highest 3 bits select destination, next byte in memory has value
			//set up for each register destination
			//need to get next byte, detemrine registry info.
			Serial.print("LDI -- ");
			for (uint8_t dest = 0; dest < 0x8; dest++) {
				stepSub = step;
				OP_CLEAR(data);
				eC = (dest | i);
				LoadMem(stepSub, eC); //get next byte

				//PC_to_IR(step, addr | eC);
				Nop(stepSub, addr | eC);
				counter += stepSub; // manually augment our micro steps counter
			}
			step = stepSub; //update eq to 1 loop
			break;
		case LDA:// first byte is instruction, next byte has address to read from
			//t3
			//eC = dmB << 5 | i;
			Serial.print("LDA -- ");
			OP_CLEAR(data);
			OP_PC_O(data);
			OP_M_L(data);
			//data = 0x2800;
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data)) {
				break;
			}

			//t4 //get value at this address
			OP_CLEAR(data);
			OP_PC_C(data);
			OP_M_O(data);
			OP_M_L(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;

			//t5
			OP_CLEAR(data);
			//OP_PC_C(data);
			OP_M_O(data);
			OP_A_I(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;
			//PC_to_IR(step, addr | eC);
			//Serial.print("Writing nop command for LDA at address 0x"); Serial.println(addr,HEX);
			Nop(step, addr | i);

			break;
		case LDB:
			//t3
			//eC = dmB << 5 | i;
			//LoadMem(step, eC); //get next byte and read mem at address
			Serial.print("LDB -- ");
			OP_CLEAR(data);
			OP_PC_O(data);
			OP_M_L(data);
			//data = 0x2800;
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data)) {
				break;
			}

			//t4 //get value at this address
			OP_CLEAR(data);
			OP_PC_C(data);
			OP_M_O(data);
			OP_M_L(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;

			//t5
			OP_CLEAR(data);
			//OP_PC_C(data);
			OP_M_O(data);
			OP_B_I(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;
			//PC_to_IR(step, addr | eC);
			//Serial.print("Writing nop command for LDB at address 0x"); Serial.println(addr, HEX);
			Nop(step, addr | i);

			break;
		case LDC:
			Serial.print("LDC -- ");
			//t3
			//eC = dmB << 5 | i;
			//LoadMem(step, eC); //get next byte and read mem at address
			OP_CLEAR(data);
			OP_PC_O(data);
			OP_M_L(data);
			//data = 0x2800;
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data)) {
				break;
			}

			//t4
			OP_CLEAR(data);
			OP_PC_C(data);
			OP_M_O(data);
			OP_C_I(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;

			//PC_to_IR(step, addr | eC);
			//Serial.print("Writing nop command for LDB at address 0x"); Serial.println(addr, HEX);
			Nop(step, addr | i);

			break;
		case LDD:
			Serial.print("LDD -- ");
			//t3
			//eC = dmB << 5 | i;
			//LoadMem(step, eC); //get next byte and read mem at address
			OP_CLEAR(data);
			OP_PC_O(data);
			OP_M_L(data);
			//data = 0x2800;
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data)) {
				break;
			}

			//t4
			OP_CLEAR(data);
			OP_PC_C(data);
			OP_M_O(data);
			OP_B_I(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;

			//PC_to_IR(step, addr | eC);
			//Serial.print("Writing nop command for LDB at address 0x"); Serial.println(addr, HEX);
			Nop(step, addr | i);

			break;
		
		case SDA:
			Serial.print("SDA -- ");
			OP_CLEAR(data);
			OP_PC_O(data);
			OP_M_L(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data)) {
				break;
			}

			//t4
			OP_CLEAR(data);
			OP_PC_C(data);
			OP_M_I(data);
			OP_A_O(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;
			break;
		case MOV_REG_REG:
			Serial.print("MOV_REG_REG -- ");
			//get next byte 
			OP_CLEAR(data);
			OP_PC_O(data);
			OP_M_L(data);
			//data = 0x2800;
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data)) {
				break;
			}

			//t4 move data to instruction, increment counter
			OP_CLEAR(data);
			OP_PC_C(data);
			
			OP_M_O(data);
			OP_IR_I(data);
			addr = step++ << 8 | i;
			if (!WriteShort(addr, data))
				break;

			break;

			Nop(step, addr);
		case MOV_REG_IMM:
			Serial.print("MOV_REG_IMM -- ");
			OP_CLEAR(data);

			for (int dest = 0; dest < 0x10; dest++) {
				stepSub = step;
				eC = dest << 5 | i;
				LoadMem(stepSub, eC);
				//data has destination. decode

				switch (dest)
				{
				case 0x00:
					OP_A_I(data);
					break;
				case 0x01:
					OP_B_I(data);
					break;
				case 0x02:
					OP_C_I(data);
					break;
				case 0x03:
					OP_D_I(data);
					break;
				case 0x04:
					OP_F_I(data);
					break;
				case 0x05:
					OP_O1_I(data);
					break;
				case 0x06:
					OP_M_I(data); //Memory In - store data from bus
					break;
				case 0x07:
					OP_PC_I(data);
					break;
				case 0x08:
					OP_ST_I(data);
					break;
				case 0x09:
					OP_M_L(data); //Memory Load - read from address INTO MAR
					break;
				default:
					break;
				}
				//OP_B_I(data);
				OP_M_O(data);
				addr = stepSub++ << 8 | eC; //t3
				if (!WriteShort(addr, data))
					break;

				OP_CLEAR(data);
				//TODO: get data
				//data  to store, save it to address in B			
				OP_B_O(data);
				OP_M_L(data); //set MAR address
				OP_M_I(data);
				addr = step++ << 8 | eC; //t3
				if (!WriteShort(addr, data))
					break;
				counter = counter + stepSub; // manually augment our micro steps counter
			}
			step = stepSub; //update eq to 1 loop
			
			

		case MOV_MEM_IMM:
			Serial.print("MOV_MEM_IMM -- ");
			OP_CLEAR(data);
			eC = dmB << 5 | i;
			LoadMem(step, eC);
			addr = step++ << 8 | eC; //t3
			if (!WriteShort(addr, data))
				break;

			OP_CLEAR(data);

			//data  to store, save it to address in B			
			OP_B_O(data);
			OP_M_L(data); //set MAR address
			OP_M_I(data);
			addr = step++ << 8 | eC; //t3
			if (!WriteShort(addr, data))
				break;


			OP_NOP(data);
			addr = step++ << 8 | eC; //t4
			if (!WriteShort(addr, data))
				break;

			break;

			Nop(step, addr);
		case ADD: //add conents of b to a, load out to a
			Serial.print("ADD -- ");
			OP_CLEAR(data);
			OP_A_I(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t4
			if (!WriteShort(addr, data))
				break;
			//PC_to_IR(step, addr | eC);
			Nop(step, addr);
			break;
		case ADD_IP: //add immideate
			Serial.print("ADD_IP -- ");
			OP_CLEAR(data);
			OP_M_L(data);
			OP_PC_O(data);
			addr = step++ << 8 | i; //t3
			if (!WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_M_O(data);
			OP_B_I(data);
			OP_PC_C(data);
			addr = step++ << 8 | i; //t4
			if (!WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_A_I(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t5
			if (!WriteShort(addr, data))
				break;

			Nop(step, addr);
			break;
		case SUB: //subtract contents of B from A
			Serial.print("SUB -- ");
			OP_CLEAR(data);
			OP_A_I(data);
			OP_U_S(data);
			OP_U_O(data);
			addr = step++ << 8 | i; //t4
			if (!WriteShort(addr, data))
				break;
			//PC_to_IR(step, addr | eC);
			Nop(step, addr);
			break;
		case SUB_IP:
			Serial.print("SUB_IP -- ");
			OP_CLEAR(data);
			OP_M_L(data);
			OP_IR_O(data);
			addr = step++ << 8 | i; //t3
			if (!WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_M_O(data);
			OP_B_I(data);
			addr = step++ << 8 | i; //t4
			if (!WriteShort(addr, data))
				break;

			OP_CLEAR(data);
			OP_A_I(data);
			OP_U_O(data);
			OP_U_S(data);
			addr = step++ << 8 | i; //t5
			if (!WriteShort(addr, data))
				break;

			Nop(step, addr);
			break;
		case JMP:
			Serial.print("JMP -- ");
			//load next byte into PC, increment PC on falling edge
			OP_CLEAR(data);
			OP_PC_O(data);	
			//OP_PC_C(data);
			OP_M_L(data);
			addr = step++ << 8 | i; //t3
			if (!WriteShort(addr, data))
				break;
			//load byte into 
			OP_CLEAR(data);
			OP_PC_I(data);
			OP_M_O(data);
			addr = step++ << 8 | i; //t3
			if (!WriteShort(addr, data))
				break;

			//should jump so no need for nop, but what the hey, let's make it rain NOPs
			Nop(step, addr);
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
			Serial.print("OUT -- ");
			OP_CLEAR(data);
			OP_A_O(data);
			OP_O1_I(data);
			addr = step++ << 8 | i; //t3
			if (!WriteShort(addr, data))
				break;

			Nop(step, addr);
			break;
		case HLT:
			Serial.print("HLT -- ");
			OP_CLEAR(data);
			OP_Hlt(data);
			addr = step++ << 8 | i; //t3
			if (!WriteShort(addr, data))
				break;
		case 0x1F:
			//undefined op code
			Nop(step, addr);			
			break;
		case NOP:
		default:
			Nop(step, addr);	
			break;
		}
		
		counter += (step * 2);// add number of steps * 2 (since each step is a word) to counter

	}
	Serial.print(F("Wrote ")); Serial.print(counter); Serial.println(F(" bytes of microinstructions"));
}


uint16_t _LDI(uint8_t destMask) {
	return destMask &0x7 | LDI;
}


void StoreProgramData() {
	Serial.print("Storing Program Code .. first erase..");
	uint16_t addr;
	uint16_t data = 0xFF;
	//unsigned
	for (uint16_t i = 0; i < 2048; i++) {
		//clear
		WriteByte(i, 0);
		if (i % 128 == 0)
			Serial.print(".");
	}
	/*Serial.println("Select program. (1) Add   (2) Add Loop, (3) for Advanced Comp");
	delay(50);
	while (!Serial.available()) {
		blinkLED();
	}*/
	int PROGRAM_COUNT = 0x1F;
	String programC = Serial.readString();
	uint8_t program = (uint8_t)programC.toInt();
	for (uint16_t i = 0; i <= PROGRAM_COUNT; i++) {
		switch (i)
		{
		case 0: //get data and store into a, get some more and store into b.
			//add, output, repeat
			//store 0 into a, 0 into b.
			
			WriteFirstByte(LDI | dmA,i << 8); //Load 
			WriteNextByte(0x1); // value to load into A
			WriteNextByte(LDI | dmB);
			WriteNextByte(0x2); // value to load into B
			WriteNextByte(ADD);
			WriteNextByte(OUT);
			WriteNextByte(LDI | dmB);
			WriteNextByte(0x5); // value to load into B
			WriteNextByte(ADD);
			WriteNextByte(OUT);
			WriteNextByte(JMP);
			WriteNextByte(0x2);
			WriteNextByte(HLT);
			break;
		case 1:
			WriteFirstByte(LDI | dmA, i << 8); //Load 
			WriteNextByte(0x1); // value to load into A
			WriteNextByte(_LDI(dmB));
			for (int lc = 0; lc < 10; lc++) {
				WriteNextByte(0x1); // value to load into B
				WriteNextByte(ADD);
				WriteNextByte(OUT);
			}
			
			WriteNextByte(JMP);
			WriteNextByte(0x0); //jump to beginning
			WriteNextByte(HLT);
			break;
		case 2: // read data from mem, add until zero
			WriteFirstByte(LDA,i<<8);
			WriteNextByte(0x10); //data at 0x10
			WriteNextByte(LDB);
			WriteNextByte(0x11); //data at 0x11
			WriteNextByte(ADD);
			WriteNextByte(OUT);
			WriteNextByte(JNZ);
			WriteNextByte(0x4);
			WriteNextByte(HLT);
			WriteNextByte(NOP);
			WriteNextByte(NOP);
			WriteNextByte(NOP);
			WriteNextByte(NOP);
			WriteNextByte(NOP);
			WriteNextByte(NOP);
			WriteNextByte(NOP);
			WriteNextByte(50);
			WriteNextByte(70);
			WriteNextByte(HLT);
			break;
		case 3: //increment from 10 up
			WriteFirstByte(LDI | dmA, i << 8);
			WriteNextByte(0xA);
			WriteNextByte(OUT);
			WriteNextByte(LDB);
			WriteNextByte(0xA);
			WriteNextByte(ADD);
			WriteNextByte(OUT);
			WriteNextByte(JMP);
			WriteNextByte(0x2);
			WriteNextByte(HLT);
			break;
		case 4: //decrement from 250 down
			WriteFirstByte(LDI | dmA, i << 8);
			WriteNextByte(0xFA);
			WriteNextByte(OUT);
			WriteNextByte(LDB);
			WriteNextByte(0xA);
			WriteNextByte(SUB);
			WriteNextByte(OUT);
			WriteNextByte(JMP);
			WriteNextByte(0x2);
			WriteNextByte(HLT);
			break;
		case 5: //fibanacci, not yet supported in h/w
			WriteFirstByte(LDI | dmA, i << 8);
			WriteNextByte(0x0);
			WriteNextByte(OUT);
			WriteNextByte(LDB);
			WriteNextByte(0x1);
			WriteNextByte(MOV_REG_REG);
			WriteNextByte(OUT);
			WriteNextByte(JMP);
			WriteNextByte(0x2);
			WriteNextByte(HLT);
			break;
		case 6: //up and down
			WriteFirstByte(LDI | dmA, i << 8);	//b0
			WriteNextByte(0xF0);				//b1
			WriteNextByte(OUT);					//b2
			WriteNextByte(LDB);					//b3
			WriteNextByte(0x4);					//b4
			WriteNextByte(ADD);					//b5
			WriteNextByte(OUT);					//b6
			WriteNextByte(JNZ);					//b7
			WriteNextByte(0x5);					//b8
			WriteNextByte(LDI | dmA);			//b9
			WriteNextByte(0x20);				//bA
			WriteNextByte(SUB);					//bB
			WriteNextByte(OUT);					//bC
			WriteNextByte(JNZ);					//bD
			WriteNextByte(0xB);					//bE
			WriteNextByte(JMP);					//bF
			WriteNextByte(0x0);					//b10
			WriteNextByte(HLT);					//b11

			break;
		case 7: //up and down
			WriteFirstByte(LDI | dmA, i << 8);	//b0
			WriteNextByte(0x00);				//b1
			WriteNextByte(OUT);					//b2
			WriteNextByte(LDB);					//b3
			WriteNextByte(0x18);				//b4
			WriteNextByte(ADD);					//b5
			WriteNextByte(OUT);					//b6
			WriteNextByte(JNZ);					//b7
			WriteNextByte(0x5);					//b8
			WriteNextByte(LDI | dmA);			//b9
			WriteNextByte(0xF0);				//bA
			WriteNextByte(SUB);					//bB
			WriteNextByte(OUT);					//bC
			WriteNextByte(JNZ);					//bD
			WriteNextByte(0xB);					//bE
			WriteNextByte(JMP);					//bF
			WriteNextByte(0x5);					//b10
			WriteNextByte(HLT);					//b11
			WriteNextByte(NOP);					//b12
			WriteNextByte(NOP);					//b13
			WriteNextByte(NOP);					//b14
			WriteNextByte(NOP);					//b15
			WriteNextByte(NOP);					//b16
			WriteNextByte(NOP);					//b17
			WriteNextByte(0x01);				//b18

			break;
		default:
			//Serial.println("  *Unknown selection * ");
			return;
		}
	}

}


#pragma endregion

#pragma region Arduino Stuff

void setup()
{

	Serial.begin(115200);
	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_OE, OUTPUT);
	pinMode(PIN_CE, OUTPUT);
	pinMode(PIN_WE, OUTPUT);
	//start with chip in neither read nor write.
	ModeOff();

	digitalWrite(PIN_LED, LOW);
	Serial.println("");
	Serial.println("Starting EEPROM tool");
	GetChipCount();
	//DumpROM();

}
void loop()
{
	PrintMenu();
	delay(5);
	if (Serial.available()) {
		ProcessInput();
	}

}

#pragma endregion
