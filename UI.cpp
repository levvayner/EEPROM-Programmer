#include "EEPROM.h"
#include "CounterRom.h"
#include "ConditionalRom.h"
#include "ProgramRom.h"
#include "UI.h"

extern EEPROM programmer;
extern ControlRom controlRom;
extern CounterRom counterRom;
extern ConditionalRom conditionalRom;
extern ProgramRom programRom;

UI::UI()
{
}


UI::~UI()
{
}


void UI::blinkLED() {
	//has been long enough since last toggle
	if (millis() - lastToggle > toggleDuration)
	{
		ledState = !ledState; //togle state
		digitalWrite(PIN_LED, ledState); //update LED
		lastToggle = millis(); //update time
	}
}

void UI::PrintMenu() {
	if (!needPrintMenu) return;
	Serial.println(F("EE PROM TOOL   -   v 0.11134a.6"));
	Serial.println(F("--------------------------------"));
	Serial.println(F("Press r to read"));
	Serial.println(F("Press w to write"));
	Serial.println(F("Press p to print data"));
	Serial.println(F("Press s to store data"));
	Serial.println(F("Press e to erase ROM"));
	Serial.println(F("Press n to specify chip count"));
	Serial.println(F("--------------------------------"));

	needPrintMenu = false;
}

void UI::DumpROM() {
	Serial.print(F("        **************      LOW        ******************"));
	if (chipCount == 2) {
		Serial.println(F("        **************      HIGH       ******************"));
	}
	else Serial.println();

	for (int sector = 0; sector < 8; sector++) {
		for (int base = sector * 256; base <= sector * 256 + 255; base += 16) {
			byte data[16];
			for (int offset = 0; offset <= 15; offset += 1) {
				data[offset] = programmer.ReadByte(base + offset);
			}

			char buf[80];
			sprintf(buf, "%05x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
				base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
				data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
			Serial.print(buf);

			if (chipCount == 2) {
				byte data2[16];
				for (int offset = 0; offset <= 15; offset += 1) {
					data2[offset] = programmer.ReadByte(base + offset, 2);
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

void UI::EraseROM()
{
	Serial.print(F("Erasing ROM.."));
	for (int i = 0; i < 2048; i++) {
		if(chipCount == 1) programmer.WriteByte(i, 0x00);
		else programmer.WriteShort(i, 0x00);
		if (i % 128 == 0) Serial.print(F("."));
	}
	Serial.println(F(" : Done."));
}

void UI::ProcessInput() {
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

		byte data = programmer.ReadByte(addr);
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

		if (programmer.WriteByte(addr, data)) {
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
	else if (c == 'e' || c == 'E') {
		EraseROM();

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
			counterRom.StoreCounterData();
			break;
		case '2':
			if (chipCount < 2) {
				Serial.println(F("This function requires a minimum of two chips.\r\n Come back when you get more."));
				break;
			}
			controlRom.StoreControlROMData();
			break;
		case '3':
			programRom.StoreProgramData();
			break;
		case '4':
			conditionalRom.StoreConditionalLogicData();
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

void UI::GetChipCount() {
	Serial.println("How many chips will be dining with us today?");
	delay(50);
	while (!Serial.available()) {
		blinkLED();
	}
	String chipC = Serial.readString();
	chipCount = (uint8_t)chipC.toInt();
	Serial.print("Ok, assuming "); Serial.print(chipCount); Serial.println(" chips.. but only because you seem trustworthy"); //and programmer does not devote time to develop detect
}

void UI::PrintOpCode(uint8_t opCode, bool condJump) {
	switch (opCode) {
	case 0:
		Serial.print(condJump ? " JLT    " : " NOP    ");
		break;
	case 1:
		Serial.print(condJump ? " JLTE   " : " LDA    ");
		break;
	case 2:
		Serial.print(condJump ? " JE     " : " LDB    ");
		break;
	case 3:
		Serial.print(condJump ? " JGT    " : " LDC    ");
		break;
	case 4:
		Serial.print(condJump ? " JGTE   " : " LDD    ");
		break;
	case 5:
		Serial.print(condJump ? " JC     " : " LDF    ");
		break;
	case 6:
		Serial.print(condJump ? " JZ     " : " LDU    ");
		break;
	case 7:
		Serial.print(condJump ? " JNZ    " : " NOTUSE ");
		break;
	case 8:
		Serial.print(" NOTUSE ");
		break;
	case 9:
		Serial.print(" NOTUSE ");
		break;
	case 0xA:
		Serial.print(" NOTUSE ");
		break;

	case 0xB:
		Serial.print(" M_RIMM ");
		break;
	case 0xC:
		Serial.print(" M_M_MM ");
		break;
	case 0xD:
		Serial.print(" ADD    ");
		break;
	case 0xE:
		Serial.print(" ADD_IP ");
		break;
	case 0xF:
		Serial.print(" SUB    ");
		break;
	case 0x10:
		Serial.print(" SUB_IP ");
		break;
	case 0x11:
		Serial.print(" JMP    ");
		break;
	case 0x12:
		Serial.print(" JLT    ");
		break;
	case 0x13:
		Serial.print(" JLE    ");
		break;
	case 0x14:
		Serial.print(" JE     ");
		break;
	case 0x15:
		Serial.print(" JG     ");
		break;
	case 0x16:
		Serial.print(" JGTE   ");
		break;
	case 0x17:
		Serial.print(" JC     ");
		break;
	case 0x18:
		Serial.print(" JZ     ");
		break;
	case 0x19:
		Serial.print(" JNZ    ");
	case 0x40:
		Serial.print(" OUT    ");
		break;
	case 0x1B:
		Serial.print(" HLT    ");
		break;
	case 0x1C:
		Serial.print(" SCF    ");
		break;
	case 0x1D:
		Serial.print(" CCF    ");
		break;
	case 0x1E:
		Serial.print(" SZF    ");
		break;
	case 0x1F:
		Serial.print(" CZF    ");
		break;
	case 0x80:
		Serial.print(" LDI    ");
		break;
	default:
		Serial.print("        ");
		break;
	}
}
void UI::PrintOperations(uint8_t op) {
	if (op & 0x1) Serial.print(" WR "); else Serial.print("    ");
	if (op &(0x1 << 1)) Serial.print(" RD "); else Serial.print("    ");
	uint8_t subOp = op >> 2;
	switch (subOp)
	{
	case 0x0:
		Serial.print(" [-]   ");
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
		Serial.print(" [CJ]  ");
		break;
	default:
		Serial.print("       ");
		break;
	}

}
