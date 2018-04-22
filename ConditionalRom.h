#ifndef _CONDITIONALROM_H
#define _CONDITIONALROM_H
#include "Arduino.h"
#include "Constructs.h"

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
class ConditionalRom
{
public:
	ConditionalRom();
	~ConditionalRom();


	//stores data to conditional logic rom
	void StoreConditionalLogicData();
};

#endif