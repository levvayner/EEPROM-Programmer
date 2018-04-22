#ifndef __CONSTRUCTS_H
#define __CONSTRUCTS_H
#include "Arduino.h"
#include "Pins.h"
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
	riMAR = 0xB,
	riSP = 0xC,
	riSI = 0xE,
	riDI = 0xF
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
	woSR = 0x9,
	woIP = 0xA,
	woMAR = 0xB,
	woSP = 0xC,
	woSI = 0xE,
	woDI = 0xF
};

#pragma endregion
#endif