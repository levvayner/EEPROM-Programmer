/******************************************************************
	Name: 28C16_Programmer
	Date: 2017-12-21
	Author:Lev Vayner
	Description: Arduino code (mega or due footprint) for programming
		and inspecting EEPROMs needed for 8 bit computer.
		8 bit computer design was inspired by Ben Eater (You Tube)
*/
//TODO: if time is laying around, write code to detect number of chips, in a non-violatile kind of manner, for now prompt user at start up
#include "EEPROM.h"
#include "ControlRom.h"
#include "CounterRom.h"
#include "ConditionalRom.h"
#include "ProgramRom.h"
#include "Constructs.h"
#include "UI.h"


EEPROM programmer = EEPROM();
ControlRom controlRom = ControlRom();
CounterRom counterRom = CounterRom();
ConditionalRom conditionalRom = ConditionalRom();
ProgramRom programRom = ProgramRom();
UI ui = UI();


#pragma region Arduino Stuff

void setup()
{

	Serial.begin(115200);
	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_OE, OUTPUT);
	pinMode(PIN_CE, OUTPUT);
	pinMode(PIN_WE, OUTPUT);
	//start with chip in neither read nor write.
	programmer.ModeOff();

	digitalWrite(PIN_LED, LOW);
	Serial.println("");
	Serial.println("Starting EEPROM tool");
	ui.GetChipCount();
	//DumpROM();

#if EEPROMType == EEPROM_32K //Software Protection OFF
	programmer.WriteByte(0x5555, 0xAA, 0); delay(1);
	programmer.WriteByte(0x2AAA, 0x55, 0); delay(1);
	programmer.WriteByte(0x5555, 0x80, 0); delay(1);
	programmer.WriteByte(0x5555, 0xAA, 0); delay(1);
	programmer.WriteByte(0x2AAA, 0x55, 0); delay(1);
	programmer.WriteByte(0x5555, 0x20, 0); delay(1);
#endif

}
void loop()
{
	ui.PrintMenu();
	delay(5);
	if (Serial.available()) {
		ui.ProcessInput();
	}

}

#pragma endregion
