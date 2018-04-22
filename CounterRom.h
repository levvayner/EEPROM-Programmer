#ifndef _COUNTERROM_H
#define _COUNTERROM_H
#include "Arduino.h"
#include "Constructs.h"
#include "ControlRom.h"

class CounterRom
{
public:
	CounterRom();
	~CounterRom();

	void StoreCounterData();

private:
	const byte digits[11] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00 };

};

#endif;