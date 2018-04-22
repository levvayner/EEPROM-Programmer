# EEPROM-Programmer

v0.2 (2018-04-22)
--------------------------------------------------------------------------------------
Added support for AT28C256 (32K) EEPROM for CONTROL ROM
Assember (C#, developed using VS2017 and .NET 4.6) 
  used to assemble "assembly" code to machine code.
  supports several directives
  DB, DW, DD, EQU, .data, .code, ORG, PROC, ENDP, END
  supports all instructions on physical machine (example included in Assember/bin/Debug folder named test1.asm
  Assember auto-injects setup code to initialze data segment, stack and code segment.
  Supports far and near calls.
  Other stuff too I am sure
Hardware
  Added segment registers
  Added stack pointer
  Added C register (or was it there before, not sure)
  Expanded memory space from the 2k AT28C16 chip to a total of 64K, using EEPROM for lowest 2K, SRAM for upper 62k.
  


v0.1 (2018-02-??)
--------------------------------------------------------------------------------------
Working for 8 bit machine. 
Current configuration allows for:
 - 1 x 2kB (11 addr lines) ROM to be programmed for machine code. Current HW configuration uses the top 3 address lines to select which program to execute, bottom 8 address lines are fed from program counter.
 - 1 x 2KB ROM to be programmed for conditional logic circuit. Allows for conditional jump intructions (LS, LSE, EQ, GT, GTE, Zero, not Zero,  carry, not carry)
 - 2 x 2KB ROM to be programmed for Control ROM. Current configuration has a 7 stage ring counter, 8 bit ip.
 
 
 
