#ifndef _UI_H
#define _UI_H
#include "Arduino.h"
#include "Constructs.h"
#include "ControlRom.h"

#define DEBUG	1
class UI
{
public:
	UI();
	~UI();


	
	void blinkLED();

	
	void PrintMenu();

	void DumpROM();
	void EraseROM();
	void ProcessInput();
	void GetChipCount();
	void PrintOpCode(uint8_t opCode, bool condJump);
	void PrintOperations(uint8_t op);

private:
	bool ledState = 0;
	unsigned long lastToggle = 0;
	unsigned long toggleDuration = 500;
	bool needPrintMenu = true;
	uint8_t chipCount = 2;

};
#endif
