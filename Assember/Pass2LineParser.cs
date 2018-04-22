using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Assember
{
    public struct Pass2LineResult
    {
        public int address;
        public byte opcode;
        public Opcode opObj; //string represenation of byte code
        public Nullable<byte> operand; //optional extra byte
        public Nullable<byte> operand2; //only used on FAR calls. might go away; might expand.
        

        public Pass2LineResult(int Address, byte Opcode, Opcode OpObj, Nullable<byte> Operand = null, Nullable<byte> Operand2 = null)
        {
            address = Address;
            opcode = Opcode;
            operand = Operand;
            operand2 = Operand2;
            opObj = OpObj;
        }
    }
    public struct Pass2Line
    {
        public string Label; //may be empty;
        public string OpCode; //must be present if not just a label
        public Nullable<Operand> Operand1; //optional
        public Nullable<Operand> Operand2; //optional
        public Opcode Op;

    }
    class Pass2LineParser : LineParser<Pass2LineResult>
    {
        public SymbolTable SymbolTable;
        private bool farProc = false;
        private string _line;
        public int codeSegmentStart; // where .CODE can be found
        public int dataSegmentStart; // where .DATA can be found

        public override List<Pass2LineResult> ParseLine(short addr, string line, SymbolTable symbols)
        {
            if (line.Length < 27) return null;
            //in pass2 we are parsing an intermediate line.
            //it is streuctured as {0} {1} {2} where 0 is the type, 1 is the address, and 2 is the line
            string cmdLine = line.Substring(26);
            line = line.Trim();
            line = Regex.Replace(line.Trim(), @"\s+", " ").Trim();
            cmdLine = Regex.Replace(cmdLine.Trim(), @"\s+", " ").Trim();
            _line = line;
            //peel off comments
            string comment = "";
            if (cmdLine.IndexOf(';') >= 0)
            {
                comment = cmdLine.Substring(cmdLine.IndexOf(';'));
                cmdLine = cmdLine.Remove(cmdLine.IndexOf(';'));
            }

            string[] words = line.Split(' ');
            if (String.IsNullOrEmpty(cmdLine)) return new List<Pass2LineResult>();
            char lType = line[0];
            int lineAddr = Int32.Parse(words[0].Substring(1),System.Globalization.NumberStyles.AllowHexSpecifier);

            //and now juss the command
            
            words = cmdLine.Split(' ');

            //var results = new List<Pass2LineResult>();
            
            //var result = new Pass2LineResult();
            if(lType == 'D') //directive)
            {
                
                try
                {
                    return ParseDirective(cmdLine, symbols, words, lineAddr);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Failed to parse directive {0} : {1} -- {2}", line, addr, ex.Message);
                }
            }
            else {
                try
                {
                    return ParseInstruction(cmdLine, symbols, words, lineAddr);
                } catch (Exception ex)
                {
                    Console.WriteLine("Failed to parse instruction {0} : {1} -- {2}", line, addr, ex.Message);
                }
            }
            
            return null;
        }


        private List<Pass2LineResult> ParseInstruction(string line, SymbolTable symbols, string[] words, int lineAddr) {
            //hooray, it is an instruction. Let's get the Label, if its there, op code, first operand if there, second operand if there;
            Pass2Line p2l = new Pass2Line();
            int opcodeEndIdx;
            var results = new List<Pass2LineResult>();

            var rawLine = String.Join(" ", words).Trim();
            words = rawLine.Split(' ');
            if (rawLine.Length == 0) return results; //no need to do anything
            if (SymbolTable.FindByAddress(lineAddr))// && )
            {// symbol exists, so label does as well
                if (words[0].Contains(':')) //regular label
                {
                    p2l.Label = words[0].Trim().Replace(":", "");
                    if (words.Length == 1) return results; //no need to do anything
                    p2l.OpCode = words[1].Trim();
                    opcodeEndIdx = words[0].Length + words[1].Length + (words[0].Length > 0 ? 2 : 1);
                }
                else //proc or otherwise labeled
                {
                    p2l.Label = SymbolTable.FindItem(lineAddr).name;
                    p2l.OpCode = words[0].Trim();
                    opcodeEndIdx = words[0].Length + 1;
                }
            }
            else
            { //get the opcode
                p2l.OpCode = words[0].Trim();
                opcodeEndIdx = words[0].Length + 1;
            }
            //check or existence of params. I more than one, we should see a command.
            bool has2Params = rawLine.Contains(',');
            if (has2Params)
            {
                string operand1Text, operand2Text;

                operand1Text = rawLine.Substring(opcodeEndIdx, rawLine.IndexOf(',') - opcodeEndIdx).Trim();
                operand2Text = rawLine.Substring(rawLine.IndexOf(',') + 1).Trim();

                Nullable<Opcode> opcode = OpcodeList.GetOpcodes(p2l.OpCode).Where(o => o.HasOperand1 || o.HasOperand2).FirstOrDefault();
                if (opcode.HasValue)
                {
                    if (opcode.Value.HasOperand1)
                    {
                        p2l.Operand1 = GetOperand(operand1Text,OperandType.Input);
                    }
                    if (opcode.Value.HasOperand2)
                    {
                        p2l.Operand2 = GetOperand(operand2Text, OperandType.Output);
                    }
                    p2l.Op = OpcodeList.GetOpcode(p2l.OpCode, p2l.Operand1, p2l.Operand2);
                }
            }
            else
            {
                //check if we have one parameter
                string operandText = rawLine.Substring(opcodeEndIdx - 1).Trim();

                if (!string.IsNullOrEmpty(operandText))
                {
                    //we should have one parameter. Need to determine if its input or output
                    //look up opcode that has at least one operand
                    Nullable<Opcode> opcode = OpcodeList.GetOpcodes(p2l.OpCode).Where(o => o.HasOperand1 || o.HasOperand2).FirstOrDefault();
                    if (opcode.HasValue)
                    {
                        if (opcode.Value.HasOperand1)
                        {
                            var operand = GetOperand(operandText, OperandType.Input);
                            p2l.Op = OpcodeList.GetOpcode(p2l.OpCode, operand, null);
                            p2l.Operand1 = operand;
                        }
                        else if (opcode.Value.HasOperand2)
                        {
                            p2l.Operand2 = GetOperand(operandText, opcode.Value.Operand2.Value.OpType); ;


                            p2l.Op = OpcodeList.GetOpcode(p2l.OpCode, p2l.Operand1, p2l.Operand2);
                            var opCode = OpcodeList.GetOpcodes(p2l.OpCode).First();
                            if (opCode.OpCodeName == "CALL")
                            {
                                var symbol = symbols.FindItem(operandText);
                                if (!string.IsNullOrEmpty(symbol.name))
                                    if (symbol.type == SymbolType.far)
                                    {
                                        p2l.Op = OpcodeList.GetOpcodes(p2l.OpCode).Last();
                                        farProc = true;
                                    }

                            }
                        }
                    }

                }
                else if (p2l.OpCode == "RET")
                {
                    //need to figure out if PROC is far
                    if (farProc) //if we are in the middle of a PROC that has the FAR qualifier, at return select far proc
                    {
                        p2l.Op = OpcodeList.GetOpcodes(p2l.OpCode).Last();
                        farProc = false; //clear the flag that we are in a far proc
                    }
                    else p2l.Op = OpcodeList.GetOpcodes(p2l.OpCode).First();
                }
                else if (p2l.OpCode == "END") //end of program, next byte should mark where the first byte goes, todo list stuff
                {
                    
                }
                else
                    p2l.Op = OpcodeList.GetOpcode(p2l.OpCode, null, null); //opcode does not have any parameters
            }

            //if nothing was generated, leave empty-handed
            if (p2l.Op.Length == 0)
            {
                Console.WriteLine("Error writing binary. Opcode not properly formed in source [{0}] at address {1}", rawLine, lineAddr);
                return results;
            }
            
            //add the op code byte
            results.Add(new Pass2LineResult(lineAddr, (byte)p2l.Op.ByteCode, p2l.Op));

            if (p2l.Op.Length == 2)
            {
                bool param1MAR = p2l.Op.HasOperand1 && p2l.Operand1.HasValue && p2l.Operand1.Value.OpCode == OperandCode.MAR;
                bool param2MAR = p2l.Op.HasOperand2 && p2l.Operand2.HasValue && p2l.Operand2.Value.OpCode == OperandCode.MAR;

                if (param1MAR)
                    results.Add(new Pass2LineResult(lineAddr + 1, (byte)p2l.Operand1.Value.Value, p2l.Op));
                else if (param2MAR)
                    results.Add(new Pass2LineResult(lineAddr + 1, (byte)p2l.Operand2.Value.Value, p2l.Op));
                else
                {
                    //constant
                    if (p2l.Operand2.HasValue) //if there is a second parameter, it could be const
                        results.Add(new Pass2LineResult(lineAddr + 1, (byte)p2l.Operand2.Value.Value, p2l.Op));
                    else if (p2l.Operand1.HasValue) //if there is a second parameter, it could be const
                        results.Add(new Pass2LineResult(lineAddr + 1, (byte)p2l.Operand1.Value.Value, p2l.Op));
                }

            }
            else if (p2l.Op.Length == 3)
            {
                string operandText = rawLine.Substring(opcodeEndIdx - 1).Trim();
                var symbol = symbols.FindItem(operandText);
                //far jump is the only one today
                results.Add(new Pass2LineResult(lineAddr + 1, (byte)(symbol.value >> 8), p2l.Op)); //add top 8 bits that will be written to CS
                results.Add(new Pass2LineResult(lineAddr + 2, (byte)(symbol.value & 0xFF), p2l.Op)); //add bottom 8 bits for PC
            }

            return results;
        }

        private Operand GetOperand(string operandText, OperandType opType)
        {
            var operand = OpcodeList.GetOperand(operandText, opType);
            //its not an operand by name.
            //check if its a reference to a known symbol
            var symbol = SymbolTable.FindItem(operandText);
            if (!string.IsNullOrEmpty(symbol.name))
            {
                //it is a referece to a symbol. replace the content's with symbol value
                operand.Name = "MAR";
                operand.Value = (byte)symbol.value;
                operand.OpCode = OperandCode.MAR;
                operand.OpType = opType;
            }//its not an operand by name.
             //check if its a reference to a known symbol            
            else
            {
                short test;
                if (operandText.ParseValue(out test))
                {
                    operand.Name = "MAR";
                    operand.Value = (byte)test;
                    operand.OpCode = OperandCode.MAR;
                    operand.OpType = opType;
                }
            }

            return operand;
        }

        private List<Pass2LineResult> ParseDirective(string line, SymbolTable symbols, string[] words, int lineAddr)
        {
            var results = new List<Pass2LineResult>();
            if (words.Length < 2)
            {
                //one word directive or something bogus
                if(words[0].ToLower() == ".code")
                {
                    //add instructions to set code segment to this address
                    codeSegmentStart = lineAddr;
                } else if (words[0].ToLower() == ".data")
                {
                    //add instructions to set data segment to this address.
                    dataSegmentStart = lineAddr;
                }
            }
            else
            {
                var d = DirectiveList.GetDirective(words[1]);
                if (d.hasSymbol)// if it doesnt have a symbol, it shouldn't impact us
                {
                    short num;
                    if (d.dType == DirectiveType.PROC)
                    {
                        if (symbols.FindItem(words[2]).type == SymbolType.far)
                            farProc = true;
                    }
                    else if (words.Length > 2 && (words[1] == "DB" || words[1] == "EQU"))
                    {
                        //comma delimited list
                        if (words[2].Any(c => c == ','))
                        {
                            //if comma seperated list

                            string[] vars = words[2].Split(',');
                            foreach (string var in vars)
                            {
                                if (var.Contains("'"))
                                {
                                    string parsed = var.Trim('\'');
                                    if (parsed.Length > 1)
                                    { //string
                                        for (int i = 0; i < parsed.Length; i++)
                                        {
                                            num = (byte)parsed[i];
                                            results.Add(new Pass2LineResult(lineAddr++, (byte)num, new Opcode()));
                                        }
                                    }
                                    else
                                    { //char
                                        num = (byte)parsed[0];
                                        results.Add(new Pass2LineResult(lineAddr++, (byte)num, new Opcode()));
                                    }
                                }
                                else if (var.ParseValue(out num))
                                    results.Add(new Pass2LineResult(lineAddr++, (byte)num, new Opcode()));
                                else
                                {
                                    //not just a regular number, may include reference to Symbol and offset
                                    throw new NotImplementedException("Meaninfgul comma seperated declarations not yet supported!");
                                }
                            }
                            //lineAddr++;
                        }
                        else //single variable
                        {

                            //fix up number
                            num = 0xFF;

                            if (line.Contains('*'))
                            {//reference to LC
                                if (line.Contains("**")) //get upper byte
                                    results.Add(new Pass2LineResult(lineAddr++, (byte)(lineAddr >> 8 & 0xFF), new Opcode()));
                                else //get lower byte
                                    results.Add(new Pass2LineResult(lineAddr++, (byte)(lineAddr & 0xFF), new Opcode()));
                            }
                            else if (line.ToUpper().Contains("DUP"))
                            {
                                int indxDup = words.ToList().IndexOf(words.ToList().Last(s => s.Contains("DUP")));
                                int multiplier;
                                if (Int32.TryParse(words[indxDup - 1], out multiplier))
                                {
                                    string dupStr = words[indxDup];
                                    int strIdx = dupStr.IndexOf("(") + 1;
                                    int endIdx = dupStr.IndexOf(")");
                                    string dupNum = dupStr.Substring(strIdx, endIdx - strIdx);
                                    byte v = 0;
                                    if (!String.IsNullOrEmpty(dupNum))
                                        byte.TryParse(dupNum, out v);

                                    for (int i = 0; i < multiplier; i++)
                                    {
                                        results.Add(new Pass2LineResult(lineAddr++, v, new Opcode()));

                                    }

                                }
                            }
                            else if (words.Length > 2 && words[2].ParseValue(out num))
                                results.Add(new Pass2LineResult(lineAddr, (byte)num, new Opcode()));
                            else
                            {
                                //not just a regular number, may include reference to Symbol and offset

                            }
                        }
                    }
                }
            }
            return results;
        }

        
    }

}
