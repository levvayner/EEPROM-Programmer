using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Assember
{
    public struct Opcode
    {
        public int ByteCode;
        public string OpCodeName;
        public string OpCodeInstruction;
        public byte Length; /* ** MAYBE THIS IS A BAD IDEA, IN CASE VARIABLE LENGTH INSTRUCTIONS?!? */
        public bool HasOperand1;
        public bool HasOperand2;
        public Nullable<Operand> Operand1;// InputOperand;
        public Nullable<Operand> Operand2;//OutputOperand;
        public Opcode(string opCodeName,bool hasOperand1,bool hasOperand2, string opCodeInstruction, int byteCode, byte length = 1, Nullable<Operand> input = null, Nullable<Operand> output = null )
        {
            ByteCode = byteCode;
            OpCodeName = opCodeName;
            Length = length;
            OpCodeInstruction = opCodeInstruction;
            this.HasOperand1 = hasOperand1;
            this.HasOperand2 = hasOperand2;

            //if (hasOperand1 && input.HasValue)
                Operand1 = input;

            //if (hasOperand2 && output.HasValue)
                Operand2 = output;

        }
    }
    public enum OperandType
    {
        Input = 0, //reading from the bus
        Output = 1 //writing to the bus
    }
    public enum OperandCode
    {//ARBITRARY NUMBERS TO NOT CONFUSE WITH BINARY REPRESENTATIONS OF REGISTERS AND OTHER I/O 
        A = 30,
        B = 31,
        C = 32,
        D = 33,
        F = 34,
        IN1 = 35,
        OUT = 36,
        MAR = 37,
        IR = 38,
        PC = 39,
        SR = 40,
        SP = 41,
        SI = 42,
        DI = 43,
        CS = 44,
        SS = 45,
        DS = 46,
        ES = 47,
        BP = 48


    }
    public struct Operand
    {
        public string Name;
        public OperandCode OpCode;
        public byte Value;
        public OperandType OpType;
        public Operand(string Name, OperandCode OpCode, byte Value, OperandType OpType)
        {
            this.Name = Name;
            this.OpCode = OpCode;
            this.Value = Value;
            this.OpType = OpType;
        }
    }
    public static class OpcodeList {
        public static List<Operand> Operands = new List<Operand>(new Operand[] {
                new Operand("A",OperandCode.A, 0 ,OperandType.Input),
                new Operand("B",OperandCode.B, 1 << 4, OperandType.Input),
                new Operand("C",OperandCode.C, 2 << 4, OperandType.Input),
                new Operand("D",OperandCode.D, 3 << 4, OperandType.Input),
                new Operand("F",OperandCode.F, 4 << 4, OperandType.Input),
                new Operand("OUT",OperandCode.OUT, 5 << 4, OperandType.Input), //is it needed here?
                //skip one on index
                new Operand("MAR",OperandCode.MAR, 7 << 4,OperandType.Input),
                new Operand("PC", OperandCode.PC, 8 << 4,OperandType.Input),
                new Operand("SR",OperandCode.SR, 9 << 4,OperandType.Input),//is it needed here?
                new Operand("CS",OperandCode.CS, 9 << 4,OperandType.Input),
                new Operand("SS",OperandCode.SS, 9 << 4,OperandType.Input),
                new Operand("DS",OperandCode.DS, 9 << 4,OperandType.Input),
                new Operand("ES",OperandCode.ES, 9 << 4,OperandType.Input),
                new Operand("IR",OperandCode.IR, 10 << 4,OperandType.Input),
                new Operand("SP", OperandCode.SP, 12 << 4,OperandType.Input),
                //new Operand("SP_INC", OperandCode.SP,13 << 4, OperandType.Input),//is it needed here?
                new Operand("SI", OperandCode.SI, 14 << 4, OperandType.Input),
                new Operand("DI", OperandCode.DI, 15 << 4, OperandType.Input),
                
                /*   outputs */
                new Operand("A"     ,OperandCode.A, 0 ,OperandType.Output),
                new Operand("B"     ,OperandCode.B, 1 , OperandType.Output),
                new Operand("C"     ,OperandCode.C, 2 , OperandType.Output),
                new Operand("D"     ,OperandCode.D, 3 , OperandType.Output),
                new Operand("F"     ,OperandCode.F, 4 , OperandType.Output),
                new Operand("IN1"   ,OperandCode.IN1, 5, OperandType.Output), //is it needed here?
                //skip one on index
                //new Operand("ALU"   ,OperandCode.ALU, 6,OperandType.Output),
                new Operand("PC"    ,OperandCode.PC, 8 ,OperandType.Output),
                //new Operand("SR"    ,OperandCode.SR, 9 ,OperandType.Output),//is it needed here?
                new Operand("CS"    ,OperandCode.CS, 9 ,OperandType.Output),
                new Operand("SS"    ,OperandCode.SS, 9 ,OperandType.Output),
                new Operand("DS"    ,OperandCode.DS, 9 ,OperandType.Output),
                new Operand("ES"    ,OperandCode.ES, 9 ,OperandType.Output),
                new Operand("IR"    ,OperandCode.IR, 10,OperandType.Output),
                new Operand("MAR"   ,OperandCode.MAR,  11 ,OperandType.Output),
                new Operand("SP"    ,OperandCode.SP, 12 ,OperandType.Output),
                //new Operand("SP_DEC",OperandCode.SP, 13 , OperandType.Output),//is it needed here?
                new Operand("SI"    ,OperandCode.SI, 14 , OperandType.Output),
                new Operand("DI"    ,OperandCode.DI,  15 , OperandType.Output),
        });
        public static List<Opcode> OpCodes = new List<Opcode>(new Opcode[]
        {

            new Opcode( "NOP", false, false, "NOP" ,0x01), //nothing to do, just passing time
            new Opcode( "LDI", true, true, "LDI"  ,0x80, 2,GetOperand("A",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)
            new Opcode( "LDI", true, true, "LDI"  ,0x81, 2,GetOperand("B",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)
            new Opcode( "LDI", true, true, "LDI"  ,0x82, 2,GetOperand("C",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)
            new Opcode( "LDI", true, true, "LDI"  ,0x83, 2,GetOperand("D",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)
            new Opcode( "LDI", true, true, "LDI"  ,0x84, 2,GetOperand("F",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)
            new Opcode( "LDI", true, true, "LDI"  ,0x85, 2,GetOperand("PC",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)
            new Opcode( "LDI", true, true, "LDI"  ,0x86, 2,GetOperand("IR",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)
            new Opcode( "LDI", true, true,  "LDI"  ,0x87, 3,GetOperand("MAR",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load immediate	  (value in next byte)

            new Opcode( "LD", true, true, "LDA"   ,0x01, 2,GetOperand("A",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load from address (address in next byte) 
            new Opcode( "LD", true, true, "LDB"   ,0x02, 2,GetOperand("B",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load from base register + offset (register in next byte)
            new Opcode( "LD", true, true, "LDC"   ,0x03, 2,GetOperand("C",OperandType.Input),GetOperand("MAR",OperandType.Output)), // load from base register + offset (register in next byte)
            new Opcode( "LD", true, true, "LDD"   ,0x04, 2,GetOperand("D",OperandType.Input),GetOperand("MAR",OperandType.Output)), // Load from D
            new Opcode( "LD", true, true, "LDF"   ,0x05, 2,GetOperand("F",OperandType.Input),GetOperand("MAR",OperandType.Output)), // Load from F
            new Opcode( "SD", true, true,"SDA"    ,0x06, 2,GetOperand("MAR",OperandType.Input),GetOperand("A",OperandType.Output)), // write a (to address in next byte)
            new Opcode( "SD", true, true,"SDB"    ,0x50, 2,GetOperand("MAR",OperandType.Input),GetOperand("B",OperandType.Output)), // write a (to address in next byte)
            new Opcode( "SD", true, true, "SDC"   ,0x51, 2,GetOperand("MAR",OperandType.Input),GetOperand("C",OperandType.Output)), // write a (to address in next byte)
            new Opcode( "SD", true, true, "SDD"   ,0x52, 2,GetOperand("MAR",OperandType.Input),GetOperand("D",OperandType.Output)), // write a (to address in next byte)
            new Opcode( "SD", true, true, "SDF"   ,0x53, 2,GetOperand("MAR",OperandType.Input),GetOperand("F",OperandType.Output)), // write a (to address in next byte)
            new Opcode( "ADD", true, false, "ADD_A",0x0D, 1,GetOperand("A",OperandType.Input)), //Add B to A, Output to A
            new Opcode( "ADD", true, false, "ADD_B",0x20, 1, GetOperand("B",OperandType.Input)), //Add B to A, Output to B
            new Opcode( "ADD", true, false, "ADD_C",0x21, 1, GetOperand("C",OperandType.Input)), //Add B to A, Output to C
            new Opcode( "ADD", true, false, "ADD_IP",0x0E,2, GetOperand("MAR",OperandType.Input)), //add contents of memory to A (address in next byte) * pointed by IP
            new Opcode( "SUB", false, false, "SUB"  ,0x0F,1), //Subtract B from A, store to A, register to subtract from A in next byte
            new Opcode( "SUB", true, false, "SUB_IP",0x10,2,GetOperand("MAR",OperandType.Input)), //Subtract value pointed to by IP in MAR from A, write to A
            new Opcode( "JMP",true,false,   "JMP"   ,0x11,2,GetOperand("MAR",OperandType.Input)), //Jump to address pointed by lower 4 bits of IP Register
            new Opcode( "JLT",true,false,"JLT"      ,0x12,2,GetOperand("MAR",OperandType.Input)), //Jump to address pointed by lower 4 bits of IP Register
            new Opcode( "JLE",true,false,"JLE"      ,0x13,2,GetOperand("MAR",OperandType.Input)), //Jump to address pointed by lower 4 bits of IP Register
            new Opcode( "JE",true,false,"JE"        ,0x14,2,GetOperand("MAR",OperandType.Input)), //Jump to address pointed by lower 4 bits of IP Register
            new Opcode( "JG",true,false,"JG"        ,0x15,2,GetOperand("MAR",OperandType.Input)), //Jump to address pointed by lower 4 bits of IP Register
            new Opcode( "JGTE",true,false,"JGTE"    ,0x16,2,GetOperand("MAR",OperandType.Input)), //Jump if carry bit is active. TODO: Implement hardware
            new Opcode( "JC",true,false,"JC"        ,0x17,2,GetOperand("MAR",OperandType.Input)),//Jump if carry bit is active. TODO: Implement hardware
            new Opcode( "JZ",true,false,"JZ"        ,0x18,2,GetOperand("MAR",OperandType.Input)), //Jump if carry bit is active. TODO: Implement hardware
            new Opcode( "JNZ",true,false,"JNZ"      ,0x19,2,GetOperand("MAR",OperandType.Input)),

            new Opcode( "OUT",false,true,"OUT"      ,0x40,1,null,GetOperand("A",OperandType.Output)), //Output content of A to decimal display (Output register 1) (default to A reg a source)
            new Opcode( "OUT",false,true,"OUT"      ,0x41,1,null,GetOperand("B",OperandType.Output)), //Output content of B to decimal display (Output register 1) (default to A reg a source)
            new Opcode( "OUT",false,true,"OUT"      ,0x42,1,null,GetOperand("C",OperandType.Output)), //Output content of C to decimal display (Output register 1) (default to A reg a source)
            new Opcode( "OUT",false,true,"OUT"      ,0x43,1,null,GetOperand("D",OperandType.Output)), //Output content of D to decimal display (Output register 1) (default to A reg a source)
            new Opcode( "OUT",false,true,"OUT"      ,0x44,1,null,GetOperand("F",OperandType.Output)), //Output content of F to decimal display (Output register 1) (default to A reg a source)
            new Opcode( "OUT",false,true,"OUT"      ,0x45,1,null,GetOperand("PC",OperandType.Output)), //Output content of PC to decimal display (Output register 1) (default to A reg a source)
            new Opcode( "OUT",false,true,"OUT"      ,0x46,1,null,GetOperand("IR",OperandType.Output)), //Output content of IR o decimal display (Output register 1) (default to A reg a source)
            new Opcode( "OUT",false,true,"OUT"      ,0x47,1,null,GetOperand("MAR",OperandType.Output)), //Output content of MAR to decimal display (Output register 1) (default to A reg a source)
            
            new Opcode( "HLT",false,true,"HLT"      ,0x1B,1), //Terminate the program


            new Opcode("SCF", false, false, "SCF"   ,0x1C), //Set Carry Flag
            new Opcode("CCF", false, false, "CCF"   ,0x1D), //Clear Carry Flag
            new Opcode("SZF", false, false, "SZF"   ,0x1E), // Set Zero Flag
            new Opcode("CZF", false, false, "CZF"   ,0x1F), // Clear Zero Flag

            new Opcode("PUSH", false, true, "PUSH_A",0x22,1,null,GetOperand(OperandCode.A, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_B" ,0x23,1,null,GetOperand(OperandCode.B, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_C" ,0x24,1,null,GetOperand(OperandCode.C, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_D" ,0x25,1,null,GetOperand(OperandCode.D, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_F" ,0x26,1,null,GetOperand(OperandCode.F, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_SI",0x27,1,null,GetOperand(OperandCode.SI, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_DI",0x28,1,null,GetOperand(OperandCode.DI, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_SP",0x29,1,null,GetOperand(OperandCode.SP, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_BP",0x2A,1,null,GetOperand(OperandCode.BP, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_CS",0x2B,1,null,GetOperand(OperandCode.CS, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_DS",0x2C,1,null,GetOperand(OperandCode.DS, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_ES",0x2D,1,null,GetOperand(OperandCode.ES, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("PUSH", false, true,"PUSH_SS",0x2E,1,null,GetOperand(OperandCode.SS, OperandType.Output)), // Push value to stack (responsible for decrementing stack)
            new Opcode("POP", true, false, "POP_A"  ,0x30,1,GetOperand(OperandCode.A, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_B"   ,0x31,1,GetOperand(OperandCode.B, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_C"   ,0x32,1,GetOperand(OperandCode.C, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_D"   ,0x33,1,GetOperand(OperandCode.D, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_F"   ,0x34,1,GetOperand(OperandCode.F, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_SI"  ,0x35,1,GetOperand(OperandCode.SI, OperandType.Input)),// POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_DI"  ,0x36,1,GetOperand(OperandCode.DI, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            //new Opcode("POP", true, false,"POP_BP"  ,0x37,1,GetOperand(OperandCode.BP, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_SP"  ,0x38,1,GetOperand(OperandCode.SP, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_ES"  ,0x39,1,GetOperand(OperandCode.ES, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_SS"  ,0x3A,1,GetOperand(OperandCode.SS, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_DS"  ,0x3B,1,GetOperand(OperandCode.DS, OperandType.Input)), // POP value from stack (responsible for incrementing stack)
            new Opcode("POP", true, false,"POP_CS"  ,0x37,1,GetOperand(OperandCode.CS, OperandType.Input)), // POP CS
            //for short jump user should push A, B, C, D, F registers onto stack before CALL_S. **For long jump, push CS register first.
            //TODO: How does the caller know which is called, so he can pop the code segment when needed?!?
            new Opcode("CALL",false, true,"CALL_S" ,0X3C,2,null,GetOperand(OperandCode.MAR,OperandType.Output)), // Pushes IP to stack.  (responsible for decrementing stack). Points IP to address in next byte from MAR
            new Opcode("CALL",false, true,"CALL_F" ,0X3D,3,null,GetOperand(OperandCode.MAR,OperandType.Output)), // Loads CS regsiter from next byte in MAR, points IP to byte after from MAR
                                                //for short jump user should pop F, D, C, B, A registers from stack after RET. for long jump, pop CS next.
            new Opcode("RET", false, false, "RET_S"  ,0x3E,1), // Pops value from register, loads into IP.  (responsible for incrementing stack)
            new Opcode("RET", false, false, "RET_F"  ,0x3F,1), // Pops value from register, loads into IP, pops next into cs.  (responsible for incrementing stack)


            /* Mov REG to REG  - to A */
            new Opcode("MOV", true, true, "MOV_A_B"        ,0x90,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to A
            new Opcode("MOV", true, true, "MOV_A_C"        ,0x91,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to A
            new Opcode("MOV", true, true, "MOV_A_D"        ,0x92,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to A
            new Opcode("MOV", true, true, "MOV_A_F"        ,0x93,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.F , OperandType.Output)), //Move from F to A
            new Opcode("MOV", true, true, "MOV_A_SI"       ,0x94,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.SI , OperandType.Output)), //Move from SI to A
            new Opcode("MOV", true, true, "MOV_A_DI"       ,0x95,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.DI , OperandType.Output)), //Move from DI to A
            new Opcode("MOV", true, true, "MOV_A_BP"       ,0x96,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.BP , OperandType.Output)), //Move from BP to A
            new Opcode("MOV", true, true, "MOV_A_SP"       ,0x97,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.SP , OperandType.Output)), //Move from SP to A
            new Opcode("MOV", true, true, "MOV_A_CS"       ,0x98,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.CS , OperandType.Output)), //Move from CS to A
            new Opcode("MOV", true, true, "MOV_A_SS"       ,0x99,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.SS , OperandType.Output)), //Move from SS to A
            new Opcode("MOV", true, true, "MOV_A_DS"       ,0x9A,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.DS , OperandType.Output)), //Move from DS to A
            new Opcode("MOV", true, true, "MOV_A_ES"       ,0x9B,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.ES , OperandType.Output)), //Move from ES to A

            /* Mov REG to REG  - to B */
            new Opcode("MOV", true, true, "MOV_B_A"        ,0x9C,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to B
            new Opcode("MOV", true, true, "MOV_B_C"        ,0x9D,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to B
            new Opcode("MOV", true, true, "MOV_B_D"        ,0x9E,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to B
            new Opcode("MOV", true, true, "MOV_B_F"        ,0x9F,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.F , OperandType.Output)), //Move from F to B
            new Opcode("MOV", true, true, "MOV_B_SI"       ,0xA0,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.SI , OperandType.Output)), //Move from SI to B
            new Opcode("MOV", true, true, "MOV_B_DI"       ,0xA1,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.DI , OperandType.Output)), //Move from DI to B
            new Opcode("MOV", true, true, "MOV_B_BP"       ,0xA2,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.BP , OperandType.Output)), //Move from BP to B
            new Opcode("MOV", true, true, "MOV_B_SP"       ,0xA3,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.SP , OperandType.Output)), //Move from SP to B
            new Opcode("MOV", true, true, "MOV_B_CS"       ,0xA4,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.CS , OperandType.Output)), //Move from CS to B
            new Opcode("MOV", true, true, "MOV_B_SS"       ,0xA5,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.SS , OperandType.Output)), //Move from SS to B
            new Opcode("MOV", true, true, "MOV_B_DS"       ,0xA6,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.DS , OperandType.Output)), //Move from DS to B
            new Opcode("MOV", true, true, "MOV_B_ES"       ,0xA7,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.ES , OperandType.Output)), //Move from ES to B

            /* Mov REG to REG  - to C */
            new Opcode("MOV", true, true, "MOV_C_A"        ,0xA8,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to C
            new Opcode("MOV", true, true, "MOV_C_B"        ,0xA9,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to C
            new Opcode("MOV", true, true, "MOV_C_D"        ,0xAA,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to C
            new Opcode("MOV", true, true, "MOV_C_F"        ,0xAB,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.F , OperandType.Output)), //Move from F to C
            new Opcode("MOV", true, true, "MOV_C_SI"       ,0xAC,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.SI , OperandType.Output)), //Move from SI to C
            new Opcode("MOV", true, true, "MOV_C_DI"       ,0xAD,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.DI , OperandType.Output)), //Move from DI to C
            new Opcode("MOV", true, true, "MOV_C_BP"       ,0xAE,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.BP , OperandType.Output)), //Move from BP to C
            new Opcode("MOV", true, true, "MOV_C_SP"       ,0xAF,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.SP , OperandType.Output)), //Move from SP to C
            new Opcode("MOV", true, true, "MOV_C_CS"       ,0xB0,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.CS , OperandType.Output)), //Move from CS to C
            new Opcode("MOV", true, true, "MOV_C_SS"       ,0xB1,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.SS , OperandType.Output)), //Move from SS to C
            new Opcode("MOV", true, true, "MOV_C_DS"       ,0xB2,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.DS , OperandType.Output)), //Move from DS to C
            new Opcode("MOV", true, true, "MOV_C_ES"       ,0xB3,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.ES , OperandType.Output)), //Move from ES to C

            /* Mov REG to REG  - to D */
            new Opcode("MOV", true, true, "MOV_D_A"        ,0xB4,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to D
            new Opcode("MOV", true, true, "MOV_D_B"        ,0xB5,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from C to D
            new Opcode("MOV", true, true, "MOV_D_C"        ,0xB6,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from D to D
            new Opcode("MOV", true, true, "MOV_D_F"        ,0xB7,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.F , OperandType.Output)), //Move from F to D
            new Opcode("MOV", true, true, "MOV_D_SI"       ,0xB8,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.SI , OperandType.Output)), //Move from SI to D
            new Opcode("MOV", true, true, "MOV_D_DI"       ,0xB9,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.DI , OperandType.Output)), //Move from DI to D
            new Opcode("MOV", true, true, "MOV_D_BP"       ,0xBA,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.BP , OperandType.Output)), //Move from BP to D
            new Opcode("MOV", true, true, "MOV_D_SP"       ,0xBB,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.SP , OperandType.Output)), //Move from SP to D
            new Opcode("MOV", true, true, "MOV_D_CS"       ,0xBC,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.CS , OperandType.Output)), //Move from CS to D
            new Opcode("MOV", true, true, "MOV_D_SS"       ,0xBD,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.SS , OperandType.Output)), //Move from SS to D
            new Opcode("MOV", true, true, "MOV_D_DS"       ,0xBE,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.DS , OperandType.Output)), //Move from DS to D
            new Opcode("MOV", true, true, "MOV_D_ES"       ,0xBF,1,GetOperand(OperandCode.D , OperandType.Input),GetOperand(OperandCode.ES , OperandType.Output)), //Move from ES to D

                    /* Mov REG to REG  - to F */
            new Opcode("MOV", true, true, "MOV_F_A"        ,0xC0,1,GetOperand(OperandCode.F , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to F
            new Opcode("MOV", true, true, "MOV_F_B"        ,0xC1,1,GetOperand(OperandCode.F , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to F
            new Opcode("MOV", true, true, "MOV_F_C"        ,0xC2,1,GetOperand(OperandCode.F , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to F
            new Opcode("MOV", true, true, "MOV_F_D"        ,0xC3,1,GetOperand(OperandCode.F , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to F

                    /* Mov REG to REG  - to SI */
            new Opcode("MOV", true, true, "MOV_SI_A"       ,0xC4,1,GetOperand(OperandCode.SI , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to SI
            new Opcode("MOV", true, true, "MOV_SI_B"       ,0xC5,1,GetOperand(OperandCode.SI , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to SI
            new Opcode("MOV", true, true, "MOV_SI_C"       ,0xC6,1,GetOperand(OperandCode.SI , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to SI
            new Opcode("MOV", true, true, "MOV_SI_D"       ,0xC7,1,GetOperand(OperandCode.SI , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to SI


                    /* Mov REG to REG  - to DI */
            new Opcode("MOV", true, true, "MOV_DI_A"       ,0xC8,1,GetOperand(OperandCode.DI , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to DI
            new Opcode("MOV", true, true, "MOV_DI_B"       ,0xC9,1,GetOperand(OperandCode.DI , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to DI
            new Opcode("MOV", true, true, "MOV_DI_C"       ,0xCA,1,GetOperand(OperandCode.DI , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to DI
            new Opcode("MOV", true, true, "MOV_DI_D"       ,0xCB,1,GetOperand(OperandCode.DI , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to DI


                    /* Mov REG to REG  - to BP */
            new Opcode("MOV", true, true, "MOV_BP_A"       ,0xCC,1,GetOperand(OperandCode.BP , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to BP
            new Opcode("MOV", true, true, "MOV_BP_B"       ,0xCD,1,GetOperand(OperandCode.BP , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to BP
            new Opcode("MOV", true, true, "MOV_BP_C"       ,0xCE,1,GetOperand(OperandCode.BP , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to BP
            new Opcode("MOV", true, true, "MOV_BP_D"       ,0xCF,1,GetOperand(OperandCode.BP , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to BP


                    /* Mov REG to REG  - to SP */ /* Yeah, we can do it. But do we need to ?!? */
            new Opcode("MOV", true, true, "MOV_SP_A"       ,0xD0,1,GetOperand(OperandCode.SP , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to SP
            new Opcode("MOV", true, true, "MOV_SP_B"       ,0xD1,1,GetOperand(OperandCode.SP , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to SP
            new Opcode("MOV", true, true, "MOV_SP_C"       ,0xD2,1,GetOperand(OperandCode.SP , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to SP
            new Opcode("MOV", true, true, "MOV_SP_D"       ,0xD3,1,GetOperand(OperandCode.SP , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to SP


                    /* Mov REG to REG  - to CS */
            new Opcode("MOV", true, true, "MOV_CS_A"       ,0xD4,1,GetOperand(OperandCode.CS , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to CS
            new Opcode("MOV", true, true, "MOV_CS_B"       ,0xD5,1,GetOperand(OperandCode.CS , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to CS
            new Opcode("MOV", true, true, "MOV_CS_C"       ,0xD6,1,GetOperand(OperandCode.CS , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to CS
            new Opcode("MOV", true, true, "MOV_CS_D"       ,0xD7,1,GetOperand(OperandCode.CS , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to CS


                    /* Mov REG to REG  - to SS */
            new Opcode("MOV", true, true, "MOV_SS_A"       ,0xD8,1,GetOperand(OperandCode.SS , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to SS
            new Opcode("MOV", true, true, "MOV_SS_B"       ,0xD9,1,GetOperand(OperandCode.SS , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to SS
            new Opcode("MOV", true, true, "MOV_SS_C"       ,0xDA,1,GetOperand(OperandCode.SS , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to SS
            new Opcode("MOV", true, true, "MOV_SS_D"       ,0xDB,1,GetOperand(OperandCode.SS , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to SS


                    /* Mov REG to REG  - to DS */
            new Opcode("MOV", true, true, "MOV_DS_A"       ,0xDC,1,GetOperand(OperandCode.DS , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to DS
            new Opcode("MOV", true, true, "MOV_DS_B"       ,0xDD,1,GetOperand(OperandCode.DS , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to DS
            new Opcode("MOV", true, true, "MOV_DS_C"       ,0xDE,1,GetOperand(OperandCode.DS , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to DS
            new Opcode("MOV", true, true, "MOV_DS_D"       ,0xDF,1,GetOperand(OperandCode.DS , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to DS


                    /* Mov REG to REG  - to ES */
            new Opcode("MOV", true, true, "MOV_ES_A"       ,0xE0,1,GetOperand(OperandCode.ES , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Move from A to ES
            new Opcode("MOV", true, true, "MOV_ES_B"       ,0xE1,1,GetOperand(OperandCode.ES , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Move from B to ES
            new Opcode("MOV", true, true, "MOV_ES_C"       ,0xE2,1,GetOperand(OperandCode.ES , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Move from C to ES
            new Opcode("MOV", true, true, "MOV_ES_D"       ,0xE3,1,GetOperand(OperandCode.ES , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Move from D to ES


                    /* Load from Memory using offset in register  - to A */
            new Opcode("LDA", false, true, "LDAI_A"         ,0xE4,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Load from DS + A in MAR to A
            new Opcode("LDA", false, true, "LDAI_B"         ,0xE5,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Load from DS + B in MAR to A
            new Opcode("LDA", false, true, "LDAI_C"         ,0xE6,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Load from DS + C in MAR to A
            new Opcode("LDA", false, true, "LDAI_D"         ,0xE7,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Load from DS + D in MAR to A

                    /* Load from Memory using offset in register  - to B */
            new Opcode("LDB", false, true, "LDBI_A"         ,0xE8,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Load from DS + A in MAR to B
            new Opcode("LDB", false, true, "LDBI_B"         ,0xE9,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Load from DS + B in MAR to B
            new Opcode("LDB", false, true, "LDBI_C"         ,0xEA,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Load from DS + C in MAR to B
            new Opcode("LDB", false, true, "LDBI_D"         ,0xEB,1,GetOperand(OperandCode.B , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Load from DS + D in MAR to B

                    /* Load from Memory using offset in register  - to C */
            new Opcode("LDC", false, true, "LDCI_A"         ,0xEC,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Load from DS + A in MAR to C
            new Opcode("LDC", false, true, "LDCI_B"         ,0xED,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Load from DS + B in MAR to C
            new Opcode("LDC", false, true, "LDCI_C"         ,0xEE,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Load from DS + C in MAR to C
            new Opcode("LDC", false, true, "LDCI_D"         ,0xEF,1,GetOperand(OperandCode.C , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)), //Load from DS + D in MAR to C

                    /* Save to Memory using offset in register  - value in A */
            new Opcode("SDA", false, true, "SDAI_A"         ,0xF0,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.A , OperandType.Output)), //Save to DS + A in MAR value in A
            new Opcode("SDA", false, true, "SDAI_B"         ,0xF1,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.B , OperandType.Output)), //Save to DS + B in MAR value in A
            new Opcode("SDA", false, true, "SDAI_C"         ,0xF2,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.C , OperandType.Output)), //Save to DS + C in MAR value in A
            new Opcode("SDA", false, true, "SDAI_D"         ,0xF3,1,GetOperand(OperandCode.A , OperandType.Input),GetOperand(OperandCode.D , OperandType.Output)) //Save to DS + D in MAR value in A

        });

        public static bool HasOpcode(string opcode)
        {
            return OpCodes.Any(op => op.OpCodeName.ToLower() == opcode.ToLower());
        }
        public static List<Opcode> GetOpcodes(string opcode)
        {
            return OpCodes.Where(op => op.OpCodeName.ToLower() == opcode.ToLower()).ToList();
        }
        public static Opcode GetOpcode(string opcode, Nullable<Operand> op1, Nullable<Operand> op2)
        {
            if(!op1.HasValue && !op2.HasValue)
                return OpCodes.Where(op => op.OpCodeName.ToLower() == opcode.ToLower()).First();
            var opTest = OpCodes.Where(op => op.OpCodeName == opcode).FirstOrDefault();
                Nullable<Operand> r1, r2;

                //operand might be implied as in LDA, where only one operand is passed. need to check for this case
                if (!opTest.HasOperand1)
                {
                    if (op1 != null)
                    {
                        r1 = null; r2 = op1;
                    }
                    else
                    {
                        r1 = null; r2 = op2;
                    }
                }
                else { r1 = op1; r2 = op2; }


            var opCodeOut = OpCodes.Where(op => op.OpCodeName.ToLower() == opcode.ToLower() &&
                ((!r1.HasValue && !op.Operand1.HasValue)
                    || (op.Operand1 != null && op.Operand1.HasValue && r1 != null && r1.Value.OpCode == op.Operand1.Value.OpCode && r1.Value.OpType == op.Operand1.Value.OpType)
                ) && ((!r2.HasValue && !op.Operand2.HasValue)
                    || (op.Operand2 != null && op.Operand2.HasValue && r2 != null && r2.Value.OpCode == op.Operand2.Value.OpCode && r2.Value.OpType == op.Operand2.Value.OpType)
                )
            ).FirstOrDefault();
            return opCodeOut;
        }

        public static bool HasOperand(string operandName)
        {
            return Operands.Any(op => op.Name.ToLower() == operandName.ToLower());
        }
        public static Operand GetOperand(string operandName, OperandType type)
        {
            return Operands.FirstOrDefault(op => op.Name.ToLower() == operandName.ToLower() && op.OpType == type);
        }
        public static Operand GetOperand(OperandCode operandCode, OperandType opType)
        {
            return Operands.FirstOrDefault(op => op.OpCode == operandCode && op.OpType == opType);
        }
    }
}
