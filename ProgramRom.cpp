#include "ProgramRom.h"
#include "EEPROM.h"
#include "UI.h"
extern EEPROM programmer;
extern UI ui;
#define TIMEOUT_WAIT_FOR_HOST 1000 * 60


ProgramRom::ProgramRom()
{
}


ProgramRom::~ProgramRom()
{
}



void ProgramRom::StoreProgramData() {

	Serial.println("Select program. (1) Single Chip + DIP selector  (2) 16 bit address (8 bit segment + 8 bit data bus)");
	Serial.println("                (3) Aseembler Slave Mode");
	delay(50);
	while (!Serial.available()) {
		ui.blinkLED();
	}
	const uint16_t MAX_ERASE = 0xFF;// 2048;
	uint16_t addr;
	uint16_t data = 0xFF;
	//Serial.print("Storing Program Code .. first erase..");
	////unsigned
	//for (uint16_t i = 0; i < MAX_ERASE; i++) {
	//	//clear
	//	programmer.WriteByte(i, 0);
	//	if (i % 128 == 0)
	//		Serial.print(".");
	//}
	//Serial.println();

	int PROGRAM_COUNT = 0x1F;
	String programC = Serial.readString();
	uint8_t program = (uint8_t)programC.toInt();
	String lineIn;
	if (program == 1) {
		for (uint16_t i = 0; i <= PROGRAM_COUNT; i++) {
			switch (i)
			{
			case 0: //get data and store into a, get some more and store into b.
					//add, output, repeat
					//store 0 into a, 0 into b.

				programmer.WriteFirstByte(LDI | dmA, i << 8); //Load 
				programmer.WriteNextByte(0x1); // value to load into A
				programmer.WriteNextByte(LDI | dmB);
				programmer.WriteNextByte(0x2); // value to load into B
				programmer.WriteNextByte(ADD_A);
				programmer.WriteNextByte(OUT);
				programmer.WriteNextByte(LDI | dmB);
				programmer.WriteNextByte(0x5); // value to load into B
				programmer.WriteNextByte(ADD_A);
				programmer.WriteNextByte(OUT);
				programmer.WriteNextByte(JMP);
				programmer.WriteNextByte(0x2);
				programmer.WriteNextByte(HLT);
				break;
			case 1: //output FF / 255 to output port and halt. if fails to halt, it should loop. if fails to loop, got 2 problems :)
				programmer.WriteFirstByte(LDI | dmA, i << 8);	//b0
				programmer.WriteNextByte(0xFF);				//b1
				programmer.WriteNextByte(OUT);					//b2			
				programmer.WriteNextByte(HLT);					//b3
				programmer.WriteNextByte(NOP);					//b4
				programmer.WriteNextByte(JMP);					//b5
				programmer.WriteNextByte(0x0);					//b6
				break;
			case 2: // read data from mem, add until zero
				programmer.WriteFirstByte(LDA, i << 8);
				programmer.WriteNextByte(0x10); //data at 0x10
				programmer.WriteNextByte(LDB);
				programmer.WriteNextByte(0x11); //data at 0x11
				programmer.WriteNextByte(ADD_A);
				programmer.WriteNextByte(OUT);
				programmer.WriteNextByte(JNZ);
				programmer.WriteNextByte(0x4);
				programmer.WriteNextByte(HLT);
				programmer.WriteNextByte(NOP);
				programmer.WriteNextByte(NOP);
				programmer.WriteNextByte(NOP);
				programmer.WriteNextByte(NOP);
				programmer.WriteNextByte(NOP);
				programmer.WriteNextByte(NOP);
				programmer.WriteNextByte(NOP);
				programmer.WriteNextByte(50);
				programmer.WriteNextByte(70);
				programmer.WriteNextByte(HLT);
				break;
			case 3: //increment from 10 up
				programmer.WriteFirstByte(LDI | dmA, i << 8);
				programmer.WriteNextByte(0xA);
				programmer.WriteNextByte(OUT);
				programmer.WriteNextByte(LDB);
				programmer.WriteNextByte(0xA);
				programmer.WriteNextByte(ADD_A);
				programmer.WriteNextByte(OUT);
				programmer.WriteNextByte(JMP);
				programmer.WriteNextByte(0x2);
				programmer.WriteNextByte(HLT);
				break;
			case 4: //up and down
				programmer.WriteFirstByte(LDI | dmA, i << 8);	//b0
				programmer.WriteNextByte(0x00);				//b1
				programmer.WriteNextByte(OUT);					//b2
				programmer.WriteNextByte(LDB);					//b3
				programmer.WriteNextByte(0x18);				//b4
				programmer.WriteNextByte(ADD_A);					//b5
				programmer.WriteNextByte(OUT);					//b6
				programmer.WriteNextByte(JNZ);					//b7
				programmer.WriteNextByte(0x5);					//b8
				programmer.WriteNextByte(LDI | dmA);			//b9
				programmer.WriteNextByte(0xF0);				//bA
				programmer.WriteNextByte(SUB);					//bB
				programmer.WriteNextByte(OUT);					//bC
				programmer.WriteNextByte(JNZ);					//bD
				programmer.WriteNextByte(0xB);					//bE
				programmer.WriteNextByte(JMP);					//bF
				programmer.WriteNextByte(0x5);					//b10
				programmer.WriteNextByte(HLT);					//b11
				programmer.WriteNextByte(NOP);					//b12
				programmer.WriteNextByte(NOP);					//b13
				programmer.WriteNextByte(NOP);					//b14
				programmer.WriteNextByte(NOP);					//b15
				programmer.WriteNextByte(NOP);					//b16
				programmer.WriteNextByte(NOP);					//b17
				programmer.WriteNextByte(0x01);				//b18
				break;
			case 5: //fibanacci
				programmer.WriteFirstByte(LDI | dmA, i << 8);
				programmer.WriteNextByte(0x0);
				programmer.WriteNextByte(LDI | dmB);
				programmer.WriteNextByte(0x1);
				programmer.WriteNextByte(OUT | dmB);
				programmer.WriteNextByte(ADD_A);
				programmer.WriteNextByte(JC);//if carry flag, reset
				programmer.WriteNextByte(0x0);
				programmer.WriteNextByte(OUT);							//0x08
				programmer.WriteNextByte(ADD_B);
				programmer.WriteNextByte(JC);//if carry flag, reset
				programmer.WriteNextByte(0x0);
				programmer.WriteNextByte(OUT | dmB);
				/*programmer.WriteNextByte(JGTE);
				programmer.WriteNextByte(0x5);*/
				programmer.WriteNextByte(JMP); //jump back to instruction Add A
				programmer.WriteNextByte(0x5);
				programmer.WriteNextByte(HLT); //0x15
				break;
			case 6: //up and down
				programmer.WriteFirstByte(LDI | dmA, i << 8);	//b0
				programmer.WriteNextByte(0xF0);				//b1
				programmer.WriteNextByte(OUT);					//b2
				programmer.WriteNextByte(LDB);					//b3
				programmer.WriteNextByte(0x4);					//b4
				programmer.WriteNextByte(ADD_A);					//b5
				programmer.WriteNextByte(OUT);					//b6
				programmer.WriteNextByte(JNZ);					//b7
				programmer.WriteNextByte(0x5);					//b8
				programmer.WriteNextByte(LDI | dmA);			//b9
				programmer.WriteNextByte(0x20);				//bA
				programmer.WriteNextByte(SUB);					//bB
				programmer.WriteNextByte(OUT);					//bC
				programmer.WriteNextByte(JNZ);					//bD
				programmer.WriteNextByte(0xB);					//bE
				programmer.WriteNextByte(JMP);					//bF
				programmer.WriteNextByte(0x0);					//b10
				programmer.WriteNextByte(HLT);					//b11

				break;
			case 7: //use register C as well
				programmer.WriteFirstByte(LDI | dmC, i << 8);	//b0
				programmer.WriteNextByte(0x10);				//b1
				programmer.WriteNextByte(OUT | dmC);			//b2	
				programmer.WriteFirstByte(LDI | dmB, i << 8);	//b3
				programmer.WriteNextByte(0x08);				//b4
				programmer.WriteNextByte(OUT | dmA);			//b5
				programmer.WriteNextByte(HLT);					//b6
				programmer.WriteNextByte(NOP);					//b7
				programmer.WriteNextByte(JMP);					//b8
				programmer.WriteNextByte(0x0);					//b9

				break;
			default:
				//Serial.println("  *Unknown selection * ");
				return;
			}
		}

	}
	else if(program == 2)
	{
		//set up segment registers
		//<<<<<<<<<<<     byte 0x00       >>>>>>>>>>>>>>>
		programmer.WriteFirstByte(LDI); //load a reg
		programmer.WriteNextByte(0x90); //with value 
		programmer.WriteNextByte(MOV_SS_A); //set stack segment

		programmer.WriteNextByte(LDI); //load a reg
		programmer.WriteNextByte(0x40); //with value 
		programmer.WriteNextByte(MOV_DS_A); //set data segment

		programmer.WriteNextByte(LDI); //load a reg
		programmer.WriteNextByte(0xF0); //with value 
		programmer.WriteNextByte(MOV_ES_A); //set extra segment



		//set up stack
		programmer.WriteNextByte(LDI | dmC);
		programmer.WriteNextByte(0xFE);
		programmer.WriteNextByte(MOV_SP_C);


#pragma region Memory test with stack (ver 1)
		////try calling store check program at known address 0x40
		//programmer.WriteNextByte(CALL_S);
		//programmer.WriteNextByte(0x40);

		///*
		////try calling count program at known address 0xE0
		//programmer.WriteNextByte(CALL_S);
		//programmer.WriteNextByte(0xE0);

		//*/

		////done with loop, sum at zero
		//// demo program to store data in RAM (first we do a scan)
		//// load value at DS + offset, outout.
		//programmer.WriteNextByte(LDI); //load A with value 0
		//programmer.WriteNextByte(0x0);
		//programmer.WriteNextByte(OUT); //otput A
		////<<<<<<<<<<<     byte 0x10       >>>>>>>>>>>>>>>
		//programmer.WriteNextByte(LDI | dmB); //load B with value 1
		//programmer.WriteNextByte(0x1);

		//programmer.WriteNextByte(LDCI_A); //load data from address + A offset into register C.
		//programmer.WriteNextByte(OUT | dmC); //otput C

		//programmer.WriteNextByte(ADD_A);

		//programmer.WriteNextByte(JNZ);
		//programmer.WriteNextByte(0x23); //if not zero, go to memory test for this segment

		////if we are here, increase data segment by one and repeat
		//programmer.WriteNextByte(MOV_A_DS); //compare data segment to 255
		//programmer.WriteNextByte(LDI | dmB);

		//programmer.WriteNextByte(0xFF);
		//programmer.WriteNextByte(SUB);

		////if zero, register is fully looped, can stop, otherwise increment segment by 1 and loop through it
		//programmer.WriteNextByte(JNZ);
		//programmer.WriteNextByte(0x1E);
		////otherwise, halt the program
		//programmer.WriteNextByte(HLT);

		////get DS, increment DS, store back to DS.
		//programmer.WriteNextByte(MOV_A_DS);
		//programmer.WriteNextByte(LDI | dmB);
		////<<<<<<<<<<<     byte 0x20       >>>>>>>>>>>>>>>
		//programmer.WriteNextByte(0x1);
		//programmer.WriteNextByte(ADD_A);
		//programmer.WriteNextByte(MOV_DS_A);
		////go to begining of reading data from address + A offset into register C

		//programmer.WriteNextByte(CALL_S);
		//programmer.WriteNextByte(0x40);

		//programmer.WriteNextByte(JMP);
		//programmer.WriteNextByte(0x14);


		///******* PROGRAM STORED AT 0x40  ***************/
		///******* CHECK ZE MEMORY (just ze segment) *****/
		//programmer.WriteFirstByte(LDI, 0x40); //load A with value 0
		//programmer.WriteNextByte(0x0);
		//programmer.WriteNextByte(MOV_C_A);
		//programmer.WriteNextByte(OUT); //otput A
		//programmer.WriteNextByte(LDI | dmB); //load B with value 1
		//programmer.WriteNextByte(0x1);

		////save value of A to memory
		//programmer.WriteNextByte(SDAI_A);
		////programmer.WriteNextByte(NOP);

		//programmer.WriteNextByte(LDCI_A); //load data from address + C offset into register A.
		//programmer.WriteNextByte(OUT | dmC); //otput C

		//programmer.WriteNextByte(ADD_A);

		//programmer.WriteNextByte(JNZ);
		//programmer.WriteNextByte(0x46); //if not zero, loop to get next byte -- LDCI_A

		////if we are here, increase data segment by one and repeat
		//programmer.WriteNextByte(MOV_A_DS); //compare data segment to 255
		//programmer.WriteNextByte(LDI | dmB);

		//programmer.WriteNextByte(0xFF);
		//programmer.WriteNextByte(SUB);

		////<<<<<<<<<<<     byte 0x50       >>>>>>>>>>>>>>>
		////if zero, register is fully looped, can stop, otherwise increment segment by 1 and loop through it
		//programmer.WriteNextByte(JNZ);
		//programmer.WriteNextByte(0x53);

		////otherwise, halt the program
		//programmer.WriteNextByte(HLT);

		////get DS, increment DS, store back to DS.
		//programmer.WriteNextByte(MOV_A_DS);
		//programmer.WriteNextByte(LDI | dmB);
		//programmer.WriteNextByte(0x1);
		//programmer.WriteNextByte(ADD_A);
		//programmer.WriteNextByte(MOV_DS_A);
		////go to begining of reading data from address + A offset into register C
		//programmer.WriteNextByte(RET);

		///****************END OF PROGRAM*/
#pragma endregion

#pragma region Stack Test
		programmer.WriteNextByte(LDI | dmB);
		programmer.WriteNextByte(0x0);
		programmer.WriteNextByte(CALL_S);
		programmer.WriteNextByte(0x30);
		//done with stack test (full wind and unwind)
		//programmer.WriteNextByte(HLT); 
		programmer.WriteNextByte(JMP);
		programmer.WriteNextByte(0x50);


		/* Loop of stack*/
		programmer.WriteFirstByte(MOV_A_SP,0x30);
		programmer.WriteNextByte(OUT);
		programmer.WriteNextByte(SUB); //subtract 0x0 from A, which should have value of stack pointer.
		programmer.WriteNextByte(JZ);//if zero, we have reached top of stack.
		programmer.WriteNextByte(0x40);
		programmer.WriteNextByte(RET_S); //return back to calling instruction at beggining of stack test call

		/* Stack top is reached .. unwind*/
		programmer.WriteFirstByte(MOV_A_SP, 0x40);
		programmer.WriteNextByte(OUT);
		programmer.WriteNextByte(RET_S);

#pragma endregion

#pragma region Far Jump Test		
		programmer.WriteFirstByte(CALL_F, 0x50);		
		programmer.WriteNextByte(0x1);
		programmer.WriteNextByte(0x30);
		//after return to function call
		programmer.WriteNextByte(POP_C); //caller of far call must get code segment back
		programmer.WriteNextByte(MOV_CS_C);
		programmer.WriteNextByte(LDI);
		programmer.WriteNextByte(0x77);
		programmer.WriteNextByte(OUT);


		//if here, we returned from the far world.
		programmer.WriteNextByte(LDI | dmB);
		programmer.WriteNextByte(100);
		programmer.WriteNextByte(OUT | dmB);
		programmer.WriteNextByte(HLT);
		programmer.WriteNextByte(HLT);
		programmer.WriteNextByte(HLT);


		//bytes at far location
		programmer.WriteFirstByte(LDA,0x130);
		programmer.WriteNextByte(0x10); //ds should still be code segment, so this will return instruction at address 0x10
		programmer.WriteNextByte(OUT);
		programmer.WriteNextByte(RET_S);

#pragma endregion

		/******PROGRAM STORED AT ADDRESS E0     ****************/
		//demo program to count (test store to reg, alu, output)
		programmer.WriteFirstByte(LDI | dmA,0xE0);
		programmer.WriteNextByte(0x0); // value to load into A
		programmer.WriteNextByte(LDI | dmB);
		programmer.WriteNextByte(0x4); // value to load into B

									   //byte 16
		programmer.WriteNextByte(ADD_A);
		programmer.WriteNextByte(OUT);

		/*programmer.WriteNextByte(JNZ);
		programmer.WriteNextByte(0xE4);*/
		// 0xE8
		programmer.WriteNextByte(RET_S);
	}
	else {
		Serial.println("Standby mode.. waiting for connection from server.");
		unsigned long startTime = millis();
		bool startComms = false;
		uint16_t lineCount = 0;
		while (millis() - startTime < TIMEOUT_WAIT_FOR_HOST && !startComms) {
			if (TIMEOUT_WAIT_FOR_HOST / 10 % millis() - startTime == 0)
			{
				Serial.print(".");
			}

			if (Serial.available()) {
				lineIn = Serial.readString();
				if (lineIn.startsWith("START_PROGRAM")) {
					Serial.println("READY");
					startComms = true;
				}
			}
		}
		while (Serial.available());
		delay(10);
		if (!startComms) {
			Serial.println("Failed to establish connection with programming server.");
			//first line after START_PROGRAM is line count
		}
		bool getLines = false;
		while (millis() - startTime < TIMEOUT_WAIT_FOR_HOST && !getLines) {
			if (Serial.available()) {
				//lineIn = Serial.readString();
				//lineCount = lineIn.toInt();
				lineCount = Serial.read() << 8;
				delay(20);
				lineCount |= Serial.read();
				if (lineCount > 0)
					getLines = true;
			}
		}
		Serial.print("Established line count: "); Serial.println(lineCount);
		uint8_t counter = 0;
		uint16_t addr;
		uint8_t data;
		byte b = 0;
		for (int i = 0; i < lineCount * 3; i++) {
			//wait to get line
			startTime = millis();
			while (millis() - startTime < TIMEOUT_WAIT_FOR_HOST && !Serial.available());
			if (Serial.available()) {
				b = Serial.read();
				//get next data byte
				//lineIn = Serial.readString();
				//Serial.print("Processing input: "); Serial.print(lineIn);
				//uint16_t length = lineIn.length();
				//uint16_t addr = lineIn.substring(0, 4).toInt();
				//uint8_t data = lineIn.substring(4,7).toInt();
				switch (counter) {
				case 0:
					
					addr =  b << 8; counter++;
					Serial.print("<< "); 
					break;
				case 1:
					addr |= (b); counter++;					
					break;
				case 2:
					data = b; counter++;
					break;
				default:
					counter = 0;
				}
				Serial.print(b, HEX);
				if (counter > 2)
					//addr = Serial.read() << 8 | Serial.read();
					//data = Serial.read();
				{
					//Serial.print("Writing to programmer: at adr: 0x"); Serial.print(addr, HEX); Serial.print(" data:0x"); Serial.println(data, HEX);
					programmer.WriteByte(addr, data);
					counter = 0;
				}
			}
		}
		
	}
}


