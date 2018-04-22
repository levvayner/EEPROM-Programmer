using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using static Assember.DirectiveList;

namespace Assember
{
    class Pass1aLineParser
    {
        public SourceLine ParseLine(string line, bool isDataLine = false)
        {
            SourceLine ret = new SourceLine();
            ret.IsData = isDataLine;
            //remove repeating tabs or spaces
            line = Regex.Replace(line.Trim(), @"\s+", " ").Trim();
            //if empty line, ignore it
            if (String.IsNullOrEmpty(line)) return ret;

            ret.IsValidLine = true;
            

            //comment          
            if (line.IndexOf(';') >= 0)
            {
                ret.Comment = line.Substring(line.IndexOf(';'));
                line = line.Remove(line.IndexOf(';'));
            }
            
            string[] words = line.Trim().Split(' ');

            ret.SourceText = line;
            ret.WordLength = words.Length;

            //label
            if (words[0].Trim().EndsWith(":"))
            {
                int test;
                ret.Label = new Label() { isLocal = Int32.TryParse(words[0], out test), label = words[0].Replace(":", "") };
            }
            //directive
            ret.IsDirective = CheckIfDirective(words);
            if (ret.IsDirective)
            {
                ret.Directive = ParseDirectiveLength(words);
                ret.ByteLength = ret.Directive.Value.ByteLength;
                if (ret.ByteLength > 0) ret.Label = new Label() { label = words[0] };
            }
            else
            // code
            {
                int codeLength = ParseCodeLength(words);
                ret.ByteLength = codeLength;
                //if (codeLength > 0)
                //{
                //    ret.SourceText = line;
                //}               
            }
            return ret;
        }

        private int ParseCodeLength(string[] words)
        {
            int length = 0;


            //get length of instruction
            int instWord = 0;// words[0].Contains(":") ? 0 : 1;
            //if (words[0] == "") instWord = 1;
            if (instWord < words.Length)
            {
                if (OpcodeList.HasOpcode(words[instWord]))
                { //if this is an op code, lets increment the counter accordingly
                    var opCode = OpcodeList.GetOpcodes(words[instWord]).First(); //don't care which we select, since we just want length
                    //assume call is 3 bytes and ret is 2 (to push and pop cs). IF we don't need it, we will issue a nop instead.
                    length = opCode.Length;
                   
                }
            }
            //if(codeLine.ByteLength > 0)
            //{
            //    codeLine.sourceCode = String.Join(" ", words);
            //}
            return length;
        }

        private bool CheckIfDirective(string[] words)
        {
            if (words.Length > 0) // no known directives with 1 word yet
            {
                var directiveWord = String.Empty;
                if (DirectiveList.IsDirective(words[0]))
                    directiveWord = words[0].Trim();
                else if (words.Length > 1 && DirectiveList.IsDirective(words[1]))
                    directiveWord = words[1].Trim();
                //check if the line is a directive
                return DirectiveList.IsDirective(directiveWord);
            }
            return false;
        }
        private Directive? ParseDirectiveLength(string[] words)
        {
            var directiveWord = String.Empty;
            if (words.Length == 0) return null; // no known directives with 1 word yet
            
            if (DirectiveList.IsDirective(words[0]))
                directiveWord = words[0].Trim();
            else if (words.Length > 1 && DirectiveList.IsDirective(words[1]))
                directiveWord = words[1].Trim();
            

            //need to figure out if it has impact on first pass
            var d = DirectiveList.GetDirective(directiveWord);
            switch (d.dType)
            {
                case DirectiveType.DB:
                case DirectiveType.DW:
                case DirectiveType.DD:
                case DirectiveType.EQU:
                    d.ByteLength = GetDataVariableLength(words, d.ByteLength);
                    break;
            }
            return d;
        }     

        private byte GetDataVariableLength(string[] words, byte byteOffset)
        {
            byte numOfBytes = 0;
            //comma delimited list?
            if (words.Length > 2 && words[2].Any(c => c == ','))
            {
                //if comma seperated list
                byte num;
                string[] vars = words[2].Split(',');

                foreach (string var in vars)
                {
                    if (var.Contains("'"))
                    {
                        string parsed = var.Trim('\'');
                        if (parsed.Length > 1)
                        { //string
                            for (int i = 0; i < parsed.Length; i++)
                                numOfBytes += (byte)( byteOffset * numOfBytes);

                        }
                        else
                            numOfBytes++;
                    }
                    else if (byte.TryParse(var, out num))
                        numOfBytes++;
                }

            }else if (words.Length == 3)
            {
                numOfBytes = byteOffset;
            }
            else if (words.Length > 3)
            {
                //has impact on LC
                //check for DUP statements
                if (words.Any(w => w.ToUpper().Contains("DUP")))
                {
                    int indxDup = words.ToList().IndexOf(words.ToList().Last(s => s.Contains("DUP")));
                    byte multiplier;
                    if (byte.TryParse(words[indxDup - 1], out multiplier))
                    {
                        numOfBytes = (byte)(multiplier * byteOffset);
                    }
                }
            }
            return numOfBytes;
        }
        
    }
}
