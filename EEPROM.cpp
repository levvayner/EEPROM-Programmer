#include "EEPROM.h"
#include "UI.h"
extern UI ui;


EEPROM::EEPROM()
{
	pinMode(PIN_ADDR0, OUTPUT);
	pinMode(PIN_ADDR1, OUTPUT);
	pinMode(PIN_ADDR2, OUTPUT);
	pinMode(PIN_ADDR3, OUTPUT);
	pinMode(PIN_ADDR4, OUTPUT);
	pinMode(PIN_ADDR5, OUTPUT);
	pinMode(PIN_ADDR6, OUTPUT);
	pinMode(PIN_ADDR7, OUTPUT);
	pinMode(PIN_ADDR8, OUTPUT);
	pinMode(PIN_ADDR9, OUTPUT);
	pinMode(PIN_ADDR10, OUTPUT);

#if EEPROMType == EEPROM_32K 
	pinMode(PIN_ADDR11, OUTPUT);
	pinMode(PIN_ADDR12, OUTPUT);
	pinMode(PIN_ADDR13, OUTPUT);
	pinMode(PIN_ADDR14, OUTPUT);
#endif

}


EEPROM::~EEPROM()
{
}


#pragma region EEPROM Chip Methods

void EEPROM::ModeOff() {
	digitalWrite(PIN_WE, HIGH);
	digitalWrite(PIN_OE, HIGH);
	digitalWrite(PIN_CE, HIGH);

	promState = dsOff;
	delayMicroseconds(1);
}

void EEPROM::ModeOutput() {
	if (promState == dsWrite) //if alredy in write mode, skip
	{
		//Serial.println("Already in write mode, skip setting mode");
		return;
	}

	digitalWrite(PIN_OE, HIGH);
	digitalWrite(PIN_WE, HIGH); //do in procedure

	pinMode(PIN_DATA0, OUTPUT);
	pinMode(PIN_DATA1, OUTPUT);
	pinMode(PIN_DATA2, OUTPUT);
	pinMode(PIN_DATA3, OUTPUT);
	pinMode(PIN_DATA4, OUTPUT);
	pinMode(PIN_DATA5, OUTPUT);
	pinMode(PIN_DATA6, OUTPUT);
	pinMode(PIN_DATA7, OUTPUT);
	pinMode(PIN_DATA8, OUTPUT);
	pinMode(PIN_DATA9, OUTPUT);
	pinMode(PIN_DATA10, OUTPUT);
	pinMode(PIN_DATA11, OUTPUT);
	pinMode(PIN_DATA12, OUTPUT);
	pinMode(PIN_DATA13, OUTPUT);
	pinMode(PIN_DATA14, OUTPUT);
	pinMode(PIN_DATA15, OUTPUT);

	if (promState == dsOff) {
		digitalWrite(PIN_CE, LOW); 
	} //make sure chip is enabled
	
	
	promState = dsWrite; //update state

}
void EEPROM::ModeInput() {
	if (promState == dsRead) { //if alredy in read mode, skip
							   //Serial.println("Already in read mode, skip setting mode");
		return;
	}
	
	digitalWrite(PIN_WE, HIGH);
	digitalWrite(PIN_OE, HIGH); //do in procedure
	
	pinMode(PIN_DATA0, INPUT);
	pinMode(PIN_DATA1, INPUT);
	pinMode(PIN_DATA2, INPUT);
	pinMode(PIN_DATA3, INPUT);
	pinMode(PIN_DATA4, INPUT);
	pinMode(PIN_DATA5, INPUT);
	pinMode(PIN_DATA6, INPUT);
	pinMode(PIN_DATA7, INPUT);
	pinMode(PIN_DATA8, INPUT);
	pinMode(PIN_DATA9, INPUT);
	pinMode(PIN_DATA10, INPUT);
	pinMode(PIN_DATA11, INPUT);
	pinMode(PIN_DATA12, INPUT);
	pinMode(PIN_DATA13, INPUT);
	pinMode(PIN_DATA14, INPUT);
	pinMode(PIN_DATA15, INPUT);

	if (promState == dsOff) {
		digitalWrite(PIN_CE, LOW); 		
	} //make sure chip is enabled

	promState = dsRead; //update state
}
//MAX ADDR is 2048 with 11 address lines
void EEPROM::SetAddress(uint16_t addr) {
	//each bit of address is going to addr pin 0 - 10. 
	
	digitalWrite(PIN_ADDR0, addr & 0x1 << 0);
	digitalWrite(PIN_ADDR1, addr & 0x1 << 1);
	digitalWrite(PIN_ADDR2, addr & 0x1 << 2);
	digitalWrite(PIN_ADDR3, addr & 0x1 << 3);
	digitalWrite(PIN_ADDR4, addr & 0x1 << 4);
	digitalWrite(PIN_ADDR5, addr & 0x1 << 5);
	digitalWrite(PIN_ADDR6, addr & 0x1 << 6);
	digitalWrite(PIN_ADDR7, addr & 0x1 << 7);
	digitalWrite(PIN_ADDR8, addr & 0x1 << 8);
	digitalWrite(PIN_ADDR9, addr & 0x1 << 9);
	digitalWrite(PIN_ADDR10, addr & 0x1 << 10);
#if EEPROMType == EEPROM_32K
	digitalWrite(PIN_ADDR11, addr & 0x1 << 11);
	digitalWrite(PIN_ADDR12, addr & 0x1 << 12);
	digitalWrite(PIN_ADDR13, addr & 0x1 << 13);
	digitalWrite(PIN_ADDR14, addr & 0x1 << 14);
#endif

}
void EEPROM::SetDataLines(uint8_t data, uint8_t byteNum) {
	if (byteNum == 1) {
		digitalWrite(PIN_DATA0, data & 0x1 << 0);
		digitalWrite(PIN_DATA1, data & 0x1 << 1);
		digitalWrite(PIN_DATA2, data & 0x1 << 2);
		digitalWrite(PIN_DATA3, data & 0x1 << 3);
		digitalWrite(PIN_DATA4, data & 0x1 << 4);
		digitalWrite(PIN_DATA5, data & 0x1 << 5);
		digitalWrite(PIN_DATA6, data & 0x1 << 6);
		digitalWrite(PIN_DATA7, data & 0x1 << 7);		
	}
	else if (byteNum == 2) {
		digitalWrite(PIN_DATA8, data & 0x1 << 0);
		digitalWrite(PIN_DATA9, data & 0x1 << 1);
		digitalWrite(PIN_DATA10, data & 0x1 << 2);
		digitalWrite(PIN_DATA11, data & 0x1 << 3);
		digitalWrite(PIN_DATA12, data & 0x1 << 4);
		digitalWrite(PIN_DATA13, data & 0x1 << 5);
		digitalWrite(PIN_DATA14, data & 0x1 << 6);
		digitalWrite(PIN_DATA15, data & 0x1 << 7);
	}
}

#pragma endregion

#pragma region Value Read/Write

//construct byte from data bits
uint8_t EEPROM::ReadByte(uint16_t addr, uint8_t chipNumber) {
	ModeInput();
	SetAddress(addr);
	digitalWrite(PIN_OE, LOW);
	delayMicroseconds(1);	
	uint8_t readValue = 0;
	switch (chipNumber)
	{
	case 1:
		readValue |= (digitalRead(PIN_DATA0) << 0);
		readValue |= (digitalRead(PIN_DATA1) << 1);
		readValue |= (digitalRead(PIN_DATA2) << 2);
		readValue |= (digitalRead(PIN_DATA3) << 3);
		readValue |= (digitalRead(PIN_DATA4) << 4);
		readValue |= (digitalRead(PIN_DATA5) << 5);
		readValue |= (digitalRead(PIN_DATA6) << 6);
		readValue |= (digitalRead(PIN_DATA7) << 7);		
		break;

	case 2:
		readValue |= (digitalRead(PIN_DATA8) << 0);
		readValue |= (digitalRead(PIN_DATA9) << 1);
		readValue |= (digitalRead(PIN_DATA10) << 2);
		readValue |= (digitalRead(PIN_DATA11) << 3);
		readValue |= (digitalRead(PIN_DATA12) << 4);
		readValue |= (digitalRead(PIN_DATA13) << 5);
		readValue |= (digitalRead(PIN_DATA14) << 6);
		readValue |= (digitalRead(PIN_DATA15) << 7);
		break;

	default:
		break;
	}

	delay(1);
	ModeOff();
	return readValue;
}

void EEPROM::WriteFirstByte(uint8_t data, uint16_t offsetAddress) {
	counter = offsetAddress;
	WriteByte(counter, data);
}
void EEPROM::WriteNextByte(uint8_t data) {
	counter++;
	WriteByte(counter, data);
}
bool EEPROM::WriteByte(uint16_t addr, uint8_t data, uint8_t retryCount) {
	_retries = 0;
	bool done = false;
	while (_retries <= retryCount && !done) {
		ModeOutput();
		SetAddress(addr);
		SetDataLines(data);
		//toggle WE low for 100ns - 1000ns
		delay(1); //give 1ms for setup time
		digitalWrite(PIN_WE, LOW);
		delayMicroseconds(1);		
		digitalWrite(PIN_WE, HIGH);
		delayMicroseconds(1);
		ModeOff();


#if DEBUG
		
		if (data != 0) {
			uint8_t step = addr >> 8;
			//uint8_t opDest = addr & 0xFF >> 4; //top 3 bits
			uint8_t opCode = addr & 0x1F; //bottom 5 bits
			Serial.print(F("Address: 0x")); Serial.print(addr, HEX);
			Serial.print(F("\tData:[0x")); Serial.print(data & 0xFF, HEX);
			/*if(opDest != 0)
			Serial.print(F("]  with destination [")); Serial.print(opDest, DEC);*/
			Serial.print(F("\t- "));  Serial.print(data & 0xFF, BIN); Serial.println(F("]"));
		}


#endif

		//verify
		ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
		delay(1);
		uint8_t readByte = ReadByte(addr);
		done = (readByte == data);
		_retries++;
		if (!done) {
			delay(50);
			if (RETRY_COUNT == _retries) {
				Serial.print(F("Attempts:")); Serial.print(_retries); Serial.print(F(".  Failed to write at address: 0x")); Serial.print(addr,HEX);
				Serial.print(F(". Expected: ")); Serial.print(data, BIN); Serial.println(F(" but found: ")); Serial.println(readByte, BIN);
				Serial.println("*****************************************");
				Serial.println("Critical Error. Failed to write to chip!");
				Serial.println("*****************************************");
				return true;
			}
		}
	}
	return done;
}
bool EEPROM::WriteShort(uint16_t addr, uint16_t data) {
	_retries = 0;
	uint16_t data_org = data;
	bool done = false;
	while (_retries < RETRY_COUNT && !done) {

		ModeOutput();
		SetAddress(addr);
		SetDataLines(data);
		SetDataLines(data >> 8, 2);
		//toggle WE low for 100ns - 1000ns
		delay(1); //give 1ms for setup time
		digitalWrite(PIN_WE, LOW);
		delayMicroseconds(1);
		digitalWrite(PIN_WE, HIGH);
	

#if DEBUG
		if (data != 0) {
			uint8_t step = addr >> 8;
			//uint8_t opDest = addr & 0xFF >> 4; //top 3 bits
			uint8_t opCode = addr & 0xFF; //bottom 8 bits
			Serial.print(F("Address: 0x")); Serial.print(addr, HEX);
			Serial.print(F("\tOpcode:"));
			bool isCondJump = (addr & (0x5 << 10)) || (addr & (0x7 << 10));
			Serial.print(isCondJump ? "[C]  " : "[NC] ");
			ui.PrintOpCode(opCode, isCondJump); Serial.print(F(" at step ")); Serial.print(step, DEC);
			Serial.print(F("\tSetting Flags: ")); ui.PrintOperations(data >> 8);
			Serial.print(F("\tData:["));
			BinToSerial(data >> 8 & 0xFF); Serial.print("  ");
			BinToSerial(data & 0xFF);
			/*if(opDest != 0)
			Serial.print(F("]  with destination [")); Serial.print(opDest, DEC);*/
			Serial.println(F("]"));
		}

#endif
		//verify
		//ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
		//uint8_t readByte = ReadByte(addr);

		//verify
		ReadByte(addr > 0 ? addr - 1 : addr + 1,1); //force data change
		uint8_t bHi, bLow;
		bHi = ReadByte(addr, 2);
		delay(1);
		bLow = ReadByte(addr);
		uint16_t bOut = bHi << 8 | bLow;

		done = (data_org == bOut);
		_retries++;
		if (!done) {
			delay(50);
			
			if (RETRY_COUNT == _retries) {
				Serial.print(F("Attempts:")); Serial.print(_retries); Serial.print(F(".  Failed to write at address: 0x")); Serial.print(addr, HEX);
				Serial.print(F(". Expected: ")); Serial.print(data_org, BIN); Serial.print(F(" but found: ")); Serial.print(bHi, BIN); Serial.print(F(" "));
				Serial.println(bLow, BIN);
				Serial.println("*****************************************");
				Serial.println("Critical Error. Failed to write to chip!");
				Serial.println("*****************************************");
				return true;//return false;
			}
				
		}
	}
	return done;
}
#pragma endregion

//Prints binary representation of number
void EEPROM::BinToSerial(uint8_t var) {
	for (int i = sizeof(var) * 8 - 1; i >= 0; i--) {
		Serial.print(((var >> i) & 1) == 1 ? "1" : "0");
	}
	
}