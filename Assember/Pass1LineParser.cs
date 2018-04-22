using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;


namespace Assember
{
    struct Pass1LineResult
    {
        public bool hasLabel;
        public bool generatesSymbol;
        public int length;
        public bool isDirective;
        public bool isOperation;
        public short resultingAddress;
        public string symbolName;
        public bool isAbsolute;
        public string intermediaryLine;
        public bool isFar;
    }

    class Pass1LineParser : LineParser<Pass1LineResult>
    {
        //a hack to keep track of being in a far procedure call (so we can add a byte for RET)
        private string _procType;
        public int codeSegmentStart; // where .CODE can be found
        public int dataSegmentStart; // where .DATA can be found

        public override List<Pass1LineResult> ParseLine(short currAddress,string line, SymbolTable symbols)
        {
            
            
            //ok, now we are ready to process
            var results = new List<Pass1LineResult>();
            var result = new Pass1LineResult();
            result.intermediaryLine = line.Trim();
            //if there isnt anything to do, leave it be
            if (string.IsNullOrEmpty(line)) return results;
             //result.resultingAddress = addr_in_source; //if we have nothing to add, it will be the same as before
             string[] words = line.Trim().Split(' ');
            //check if we have a label
            result.hasLabel = words[0].Trim().EndsWith(":");// if first word ends with colon it is a label

            if (words.Length > 0) // no known directives with 1 word yet
            {
                var directiveWord = String.Empty;
                if (DirectiveList.IsDirective(words[0]))
                    directiveWord = words[0].Trim();
                else if (words.Length > 1 && DirectiveList.IsDirective(words[1]))
                    directiveWord = words[1].Trim();
                //check if the line is a directive
                result.isDirective = DirectiveList.IsDirective(directiveWord);
                if (result.isDirective)
                {
                    //need to figure out if it has impact on first pass
                    var d = DirectiveList.GetDirective(directiveWord);
                    result.generatesSymbol = d.hasSymbol;
                    MarkIfFar(result, words, d);

                    if (words.Length < 2)
                    {
                        //one word directive or something bogus
                        if (words[0].ToLower() == ".code")
                        {
                            //add instructions to set code segment to this address
                            codeSegmentStart = currAddress;
                        }
                        else if (words[0].ToLower() == ".data")
                        {
                            //add instructions to set data segment to this address.
                            dataSegmentStart = currAddress;
                        }
                    }else if (words.Length > 2 && words[2].Any(c => c == ','))
                    {
                        //if comma seperated list
                        byte num;
                        string[] vars = words[2].Split(',');
                        byte numOfBytes = 0;
                        foreach (string var in vars)
                        {
                            if (var.Contains("'"))
                            {
                                string parsed = var.Trim('\'');
                                if (parsed.Length > 1)
                                { //string
                                    for (int i = 0; i < parsed.Length; i++)
                                        numOfBytes++;

                                }
                                else
                                    numOfBytes++;
                            }
                            else if (byte.TryParse(var, out num))
                                numOfBytes++;
                        }
                        d.ByteLength = numOfBytes;
                        //lineAddr++;
                    }

                    result.isAbsolute = d.isAbsolute;
                    if (d.isAbsolute)
                    {//directive calls for absoulte address adjustment                    
                        result = GetAbsoluteAddress(line, result, words);

                    }
                    else if (d.ByteLength != 0)
                    {
                        //has impact on LC
                        //check for DUP statements
                        if (line.ToUpper().Contains(" DUP"))
                        {
                            int indxDup = words.ToList().IndexOf(words.ToList().Last(s => s.Contains("DUP")));
                            byte multiplier;
                            if (byte.TryParse(words[indxDup - 1], out multiplier))
                            {
                                d.ByteLength *= multiplier;
                            }
                        }
                        result.resultingAddress = d.ByteLength;
                    }
                    else if (words.Length > 2 && words[2].ToLower() == "far")
                        _procType = "far";
                }
            }
            bool hasPopCS = false;
            if (!result.isDirective)
            { //if it wasn't a directive line, it might be a code line
                hasPopCS = ParseCodeLine(currAddress, symbols, results, ref result, words);

            }

            if (result.hasLabel)
                result.symbolName = words[0].Replace(":", "");
            else if (result.generatesSymbol && string.IsNullOrEmpty(result.symbolName))
                if (words[0].ToLower() == "proc" || words[0].ToLower() == "end")
                    result.symbolName = codeSegmentStart.ToString("X");
                else
                result.symbolName = words[0];

                results.Add(result); //for now, just one per line on pass 1

            if (hasPopCS) {
                results.Add(new Pass1LineResult() { isOperation = true, length = 1, intermediaryLine = String.Format("POP CS",currAddress), resultingAddress = 1 });
            }

            return results;
        }

        private bool ParseCodeLine(short currAddress, SymbolTable symbols, List<Pass1LineResult> results, ref Pass1LineResult result, string[] words)
        {
            bool hasPopCS = false;
            //get length of instruction
            int instWord = result.hasLabel ? 1 : 0;
            if (words[0] == "") instWord = 1;
            if (instWord < words.Length)
            {
                if (OpcodeList.HasOpcode(words[instWord]))
                { //if this is an op code, lets increment the counter accordingly
                    var opCode = OpcodeList.GetOpcodes(words[instWord]).First(); //don't care which we select, since we just want length
                    if (opCode.OpCodeName == "CALL")
                    {
                        var symbol = symbols.FindItem(words[instWord + 1]);
                        if (!string.IsNullOrEmpty(symbol.name))
                            if (symbol.type == SymbolType.far)
                            //if (Math.Abs((symbol.value >> 8) - (currAddress >> 8)) > 0)
                            {
                                results.Add(new Pass1LineResult() { isOperation = true, length = 1, intermediaryLine = String.Format("PUSH CS", currAddress), resultingAddress = 1 });
                                opCode = OpcodeList.GetOpcodes(words[instWord]).Last();
                                _procType = "far";
                            }
                            else
                                _procType = "";
                        else
                        {
                            //it hasn't been declared yet possibly. Mark that it needs to be treated as a far jump. later we will make is near and add nop if necessary??!!
                        }
                            result.intermediaryLine = "??";
                    }
                    if (opCode.OpCodeName == "RET")
                    {
                        //need to figure out if PROC is far
                        if (_procType == "far")
                        {
                            hasPopCS = true;
                        }
                    }
                    result.length = opCode.Length;
                    result.resultingAddress = opCode.Length;
                }
            }
            return hasPopCS;
        }

        private static void MarkIfFar(Pass1LineResult result, string[] words, Directive d)
        {
            if (d.name == "PROC")
            {
                result.symbolName = words[2];
                if (words.Length > 3 && words[3].Trim().ToUpper() == "FAR")
                    result.isFar = true;
            }
        }

        private static Pass1LineResult GetAbsoluteAddress(string line, Pass1LineResult result, string[] words)
        {
            try
            {
                short val = 0;
                if (Int16.TryParse(words[words.Length - 1], out val))
                    result.resultingAddress = val;
                else
                {
                    if (words[words.Length - 1].ToLower().Contains('h'))
                    {
                        result.resultingAddress = Int16.Parse(words[words.Length - 1].Remove(words[words.Length - 1].Length - 1), System.Globalization.NumberStyles.AllowHexSpecifier);
                    }
                }

                //result.resultingAddress = Convert.ToInt16(words[2], 16);
            }
            catch (Exception ex) { Console.WriteLine("Error {2} while parsing line {0}. Failed to load absolute address {1}", line, words[words.Length - 1], ex.Message); }

            return result;
        }
    }
}
