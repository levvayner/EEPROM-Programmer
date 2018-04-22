#ifndef _PINS_H
#define _PINS_H
#define EEPROM_2K	0		//2K chip (AT28C16)
#define EEPROM_32K	1		//32K chip (AT28C256)
#define EEPROMType EEPROM_2K

#if EEPROMType == EEPROM_2K

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

#else

#pragma region Pins

//address pins
#define PIN_ADDR0	33
#define PIN_ADDR1	35
#define PIN_ADDR2	37
#define PIN_ADDR3	39
#define PIN_ADDR4	41
#define PIN_ADDR5	43
#define PIN_ADDR6	45
#define PIN_ADDR7	47
#define PIN_ADDR8	29
#define PIN_ADDR9	27
#define PIN_ADDR10	23
#define PIN_ADDR11	25
#define PIN_ADDR12	49
#define PIN_ADDR13	31
#define PIN_ADDR14	51

//data pins
#define PIN_DATA0	52
#define PIN_DATA1	50
#define PIN_DATA2	48
#define PIN_DATA3	30
#define PIN_DATA4	28
#define PIN_DATA5	26
#define PIN_DATA6	24
#define PIN_DATA7	22
#define PIN_DATA8	46
#define PIN_DATA9	44
#define PIN_DATA10	42
#define PIN_DATA11	40
#define PIN_DATA12	38
#define PIN_DATA13	36
#define PIN_DATA14	34
#define PIN_DATA15	32


//control pins
#define PIN_CE		19	//ACTIVE LOW	- pull down to use chip.
#define PIN_WE		21	//ACTIVE LOW	- pull down for 100ns - 1000ns *AFTER* setting address and data. OE should be HIGH. CE should be HIGH.
#define PIN_OE		20	//ACTIVE LOW	- pull down *AFTER* setting address and data. WE should be HIGH. CE should be HIGH.

#pragma endregion

#endif


#endif // !28C16_PINS_H