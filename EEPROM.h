#ifndef _EEPROM_H
#define _EEPROM_H
#include "Arduino.h"
#include "Pins.h"
#include "Constructs.h"

#define RETRY_COUNT 25

enum DeviceState {
	dsOff = 0,
	dsRead = 1,
	dsWrite = 2
};
class EEPROM
{
public:
	EEPROM();
	~EEPROM();


	void ModeOff();
	void ModeOutput();
	void ModeInput();
	//MAX ADDR is 2048 with 11 address lines

	//construct byte from data bits
	uint8_t ReadByte(uint16_t addr, uint8_t chipNumber = 1);
	
	void WriteFirstByte(uint8_t data, uint16_t offsetAddress = 0);
	void WriteNextByte(uint8_t data);
	bool WriteByte(uint16_t addr, uint8_t data, uint8_t retryCount = RETRY_COUNT);
	bool WriteShort(uint16_t addr, uint16_t data);

private:
	uint16_t counter = 0;
	uint16_t _retries = 0;

	void SetAddress(uint16_t addr);
	void SetDataLines(uint8_t data, uint8_t byteNum = 1);

	void BinToSerial(uint8_t var);
	
	DeviceState promState;
};


#endif
