using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Assember
{
    class Pass1
    {

        //INPUT Assembly files
        //OUTPUT Symbol Map for each file
        /* Structure
         *  NAME      VALUE     TYPE   */
        SymbolTable _symbolTable;
        Pass1LineParser _parser;
        private short _ds = 0x7;
        private short _cs = 0x0;
        private short _ss = 0x20;
        private int dataSegLineNum = -1, codeSegLineNum = -1;
        public SymbolTable Symbols { get { return _symbolTable; } }
        Dictionary<string, short> _counters;
        string _currentCounter;
        public string IntermediateFile { get; private set; }

        public Pass1(string intermediateFileName = "intermediate.obj")
        {
            _symbolTable = new SymbolTable();
            _counters = new Dictionary<string, short>(20);
            //set up current counter
            _counters.Add("$", 0);
            _counters.Add("data", 0);
            _currentCounter = "$";
            IntermediateFile = intermediateFileName;

        }
        //TODO: make use of multiple counters
        public bool ProcessFiles(string[] sourceFiles)
        {
            //set up a new parsing session
            _parser = new Pass1LineParser();
            List<string> allLines = new List<string>();
            List<KeyValuePair<int, string>> codeLines = new List<KeyValuePair<int, string>>();
            List<KeyValuePair<int, string>> dataLines = new List<KeyValuePair<int, string>>();
            List<Procedure> procedures = new List<Procedure>();
            foreach (string file in sourceFiles)
            {
                //read source file
                allLines = ReadSourceFile(file);

                //put data into seperate list
                ReadSourceDataLines(allLines, dataLines);
                //now for the code
                ReadSourceCodeLines(allLines, codeLines);
                

                List<SourceLine> sourceLines = ExtractSourceLines(codeLines, dataLines);
                
                //at this point, we should have all items cataloged with addresses relative to their segment
                //build procs
                ExtractProcLines(procedures, sourceLines);
                BuildAddressSpace(sourceLines);

                //we have all source lines, with assigned addresses, now we can finally parse out the symbols with their addresses
                foreach (var line in sourceLines.Where(s => s.IsDirective && s.Directive.HasValue && s.Directive.Value.hasSymbol))
                {
                    SymbolType st;
                    if (line.Directive.Value.isAbsolute)
                        st = SymbolType.abs;
                    else if (line.Directive.Value.dType == DirectiveType.PROC)
                    {
                        var proc = procedures.FirstOrDefault(p => p.CodeLines[0] == line);
                        if (proc.IsFar)
                            st = SymbolType.far;
                        else
                            st = SymbolType.rel;
                       
                    }
                    else
                        st = SymbolType.rel;

                    _symbolTable.AddItem(line.Label.Value.label, line.Address, st);
                }
                //add labels to symbols table
                foreach( var line in sourceLines.Where(s => s.IsLabel))
                {
                    _symbolTable.AddItem(line.Label.Value.label, line.Address, SymbolType.rel);
                }

                //new we have all of our symbols built. write out intermediate file
                using (StreamWriter writer = new StreamWriter(IntermediateFile))
                {
                    //first write out header
                    string header = String.Format("; DS:0x{0} \tCS:0x{1}",
                        dataLines.Count > 0 ? (_ds << 8 & 0xFF00).ToString("X") : " ",
                        codeLines.Count > 0 ? (_cs << 8 & 0xFF00).ToString("X") : " "
                        );
                    writer.WriteLine(header);
                    foreach (var line in sourceLines.Where(l => l.WordLength > 0))
                    {
                        if (String.IsNullOrEmpty(line.SourceText)) continue;
                        writer.WriteLine("{0}{1}{2}{3}{4}{5}", 
                            line.IsDirective ? 'D' : line.IsLabel ? 'L' : 'C', 
                            line.Address.ToString("X").PadLeft(6, '0').PadRight(8, ' '), 
                            line.Label.HasValue ? line.Label.Value.label.PadRight(10, ' ') : "".PadRight(10, ' '), 
                            line.ByteLength.ToString().PadRight(7, ' '), 
                            line.SourceText.PadRight(40, ' '),
                            "".PadLeft(5,' ') + (String.IsNullOrEmpty(line.Comment) ? "" :  line.Comment.PadRight(40, ' '))
                        );
                    }

                }
            }

            return true;
        }

        private List<string> CreateInitLines()
        {
            List<string> codeLines = new List<string>();
            codeLines.Insert(0, String.Format("LDI A, 0{0:X}h    ;  initialize A for DS offset for 0x{0:X}00", _ds));
            codeLines.Insert(1, String.Format("MOV DS, A     ;  load data segment with value in A"));
            codeLines.Insert(2, String.Format("LD A, 0{0:X}h     ;  get variable for segment register", _ss));
            codeLines.Insert(3, String.Format("MOV SS, A     ;  load segment register with value"));
            codeLines.Insert(4, String.Format("LD A, 0FFh    ;  set up stack pointer"));
            codeLines.Insert(5, String.Format("MOV SP, A     ;  set up stack pointer"));
            return codeLines;
        }

        private void BuildAddressSpace(List<SourceLine> sourceLines)
        {
            //let's build data from DS
            _currentCounter = "data";
            _counters[_currentCounter] = (short)(_ds << 8);
            foreach (var line in sourceLines.Where(s => s.IsData))
            {
                //assign addresses to data lines
                sourceLines[sourceLines.IndexOf(line)].AssignAddress(_counters[_currentCounter]);
                _counters[_currentCounter] += (short)line.ByteLength;
            }

            //AdjustForFarCalls(sourceLines);
            _currentCounter = "$";
            _counters[_currentCounter] = (short)(_cs << 8);
            foreach (var line in sourceLines.Where(s => !s.IsAddressAssigned))
            {
                //assign addresses to data lines
                if (line.IsDirective && line.Directive.HasValue && line.Directive.Value.isAbsolute)
                {
                    short test;
                    if (line.SourceText.Split(' ')[1].ParseValue(out test)) //get ORG value
                        _counters[_currentCounter] = test;
                }
                sourceLines[sourceLines.IndexOf(line)].AssignAddress(_counters[_currentCounter]);
                _counters[_currentCounter] += (short)line.ByteLength;
            }
        }

        //private void AdjustForFarCalls(List<SourceLine> sourceLines)
        //{
        //    var procLineHeaders = sourceLines.Where(s => s.IsDirective && s.Directive.HasValue && s.Directive.Value.hasSymbol && s.Directive.Value.dType == DirectiveType.PROC).ToList();

        //    foreach (var procHeader in procLineHeaders)
        //    {
        //        //find lines that have a reference
        //        sourceLines.Where(s => !s.IsDirective && s.SourceText.Contains(procHeader.Label.Value.label)).ToList().ForEach(call => call.ByteLength++);
        //    }
        //}

        private static void ExtractProcLines(List<Procedure> procedures, List<SourceLine> sourceLines)
        {
            var procLineHeaders = sourceLines.Where(s => s.IsDirective && s.Directive.HasValue && s.Directive.Value.hasSymbol && s.Directive.Value.dType == DirectiveType.PROC).ToList();

            foreach (var procHeader in procLineHeaders)
            {
                Procedure p = new Procedure(procHeader.SourceText);
                int startProcIdx = sourceLines.IndexOf(procHeader);
                var procEnd = sourceLines.Skip(startProcIdx).FirstOrDefault(l => l.IsDirective && l.Directive.HasValue && l.Directive.Value.dType == DirectiveType.ENDP);
                if (procEnd.WordLength == 0) throw new NotImplementedException(
                     String.Format("The procedure {0} in line {1} does not have a closing ENDP statement", procHeader.Directive.Value.name, procHeader.SourceText));
                int nextEndPIdx = sourceLines.IndexOf(procEnd);
                for (int i = startProcIdx; i <= nextEndPIdx; i++)
                {
                    p.AddLine(sourceLines[i]);
                }
                //set the label on the line
                procHeader.Label = new Label() { label = p.Name };
                procedures.Add(p);
                //modify any calls to this procedure to take 3 bytes
                if (p.IsFar) sourceLines.Where(sl => sl != procHeader && sl.SourceText.Contains(p.Name)).ToList().ForEach(sl => sl.ByteLength += 1);

            }
        }

        private int ReadSourceCodeLines(List<string> allLines, List<KeyValuePair<int, string>> codeLines)
        {
            int lineAddedCount = 0;
            //add init sequence
            var autoLines = CreateInitLines();
            autoLines.ForEach(l => codeLines.Add(new KeyValuePair<int, string>(lineAddedCount++, l)));

            for (int i = 0; i < allLines.Count; i++)
            {
                codeLines.Add(new KeyValuePair<int, string>(lineAddedCount, allLines[i]));
                lineAddedCount++;
            }
            Console.WriteLine("Found {0} code lines", lineAddedCount.ToString());
            return lineAddedCount;
        }

        private int ReadSourceDataLines(List<string> allLines, List<KeyValuePair<int, string>> dataLines)
        {
            int lineAddedCount = 0;
            //check if we got ds
            if (dataSegLineNum >= 0)
            {
                int numLines = 0;
                if (dataSegLineNum < codeSegLineNum) //if .data is written first in source file
                    numLines = codeSegLineNum - dataSegLineNum;
                else //code is first
                    numLines = allLines.Count - dataSegLineNum + 1; //TODO: verify the +1


                for (int i = 0; i < numLines; i++)
                {
                    dataLines.Add(new KeyValuePair<int, string>(lineAddedCount, allLines[dataSegLineNum]));
                    allLines.RemoveAt(dataSegLineNum);
                    lineAddedCount++;
                }

            }
            Console.WriteLine("Found {0} data lines", lineAddedCount.ToString());
            return lineAddedCount;
        }

        private List<string> ReadSourceFile(string file)
        {
            var allLines = new List<string>();
            if (!File.Exists(file))
            {
                Console.WriteLine("Skipping {0}. File not found!", file);
                return allLines;
            }
            using (StreamReader reader = new StreamReader(file))
            {
                int lineNum = 0;
                while (!reader.EndOfStream)
                {
                    var line = reader.ReadLine();
                    if (line.Trim().ToLower() == ".code")
                    {
                        codeSegLineNum = lineNum;
                        //add instructions to set code segment to this address                            
                    }
                    else if (line.Trim().ToLower() == ".data")
                    {
                        //add instructions to set data segment to this address.
                        dataSegLineNum = lineNum;
                    }
                    allLines.Add(line);
                    lineNum++;
                }
            }
            return allLines;
        }

        private static List<SourceLine> ExtractSourceLines(List<KeyValuePair<int, string>> codeLines, List<KeyValuePair<int, string>> dataLines)
        {
            var parser = new Pass1aLineParser();
            List<SourceLine> sourceLines = new List<SourceLine>();
            foreach (var line in dataLines)
            {
                if (string.IsNullOrEmpty(line.Value)) continue;
                var result = parser.ParseLine(line.Value, true);
                if (String.IsNullOrEmpty(result.SourceText)) continue;
                sourceLines.Add(result);
            }

            foreach (var line in codeLines)
            {
                if (string.IsNullOrEmpty(line.Value)) continue;
                var result = parser.ParseLine(line.Value);
                if (String.IsNullOrEmpty(result.SourceText)) continue;
                sourceLines.Add(result);
            }

            return sourceLines;
        }
    }
}
