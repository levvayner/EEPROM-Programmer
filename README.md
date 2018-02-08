# EEPROM-Programmer
Working for 8 bit machine. 
Current configuration allows for:
 - 1 x 2kB (11 addr lines) ROM to be programmed for machine code. Current HW configuration uses the top 3 address lines to select which program to execute, bottom 8 address lines are fed from instruction pointer.
 - 1 x 2KB ROM to be programmed for conditional logic circuit. Allows for conditional jump intructions (LS, LSE, EQ, GT, GTE, Zero, not Zero,  carry, not carry)
 - 2 x 2KB ROM to be programmed for Control ROM. Current configuration has a 7 stage ring counter, 8 bit ip.
