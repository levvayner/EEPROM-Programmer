#include "CounterRom.h"
#include "EEPROM.h"
extern EEPROM programmer;

CounterRom::CounterRom()
{
}


CounterRom::~CounterRom()
{
}

void CounterRom::StoreCounterData() {
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
			if (!programmer.WriteByte(addr, data)) {
				Serial.print(F("Failed to write at address: ")); Serial.print(addr);
				Serial.print(F("  data: ")); Serial.println(data, BIN);
			}
		}
	}
}
