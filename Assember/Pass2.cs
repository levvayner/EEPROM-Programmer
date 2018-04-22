using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Assember
{
    class Pass2
    {//INPUT Assembly files and Symbol file
     //OUTPUT object file (for this version, that is machine code)
        SymbolTable _symbolTable;
        Pass2LineParser _parser;
        string _outputFile = "file1.bin";

        public string OutputFile { get { return _outputFile; } }

        //Dictionary<string, int> _counters;
        //string _currentCounter;

        public Pass2(SymbolTable SymbolTable, string OutputFile = "File1.bin")
        {
            _symbolTable = SymbolTable;
            //_counters = new Dictionary<string, int>(20);
            ////set up current counter
            //_counters.Add("$", 0);
            //_currentCounter = "$";
            _outputFile = OutputFile;

        }

        public bool ProcessIntermediary(string intermediaryFile)
        {
            //set up a new parsing session
            _parser = new Pass2LineParser();
            Dictionary<int, byte> lines = new Dictionary<int, byte>();
            Dictionary<int, string> debugLines = new Dictionary<int, string>();
            string dbgFileName = OutputFile.Replace(".bin", ".dbg");


            if (!File.Exists(intermediaryFile))
            {
                Console.WriteLine("Error {0}. File not found!", intermediaryFile);
                return false;
            }
            if (File.Exists(dbgFileName)) File.Delete(dbgFileName);

            _parser.SymbolTable = _symbolTable;

            ParseIntermedieryFile(intermediaryFile, lines, debugLines);
            WriteBinaryFile(lines);

            //debugLines = debugLines.OrderBy(d => d.Key);
            WriteDebugFile(dbgFileName, debugLines);
            return true;
        }
        /// <summary>
        /// Writes binary (machine code) file
        /// </summary>
        /// <param name="lines"></param>
        private void WriteBinaryFile(Dictionary<int, byte> lines)
        {
            using (StreamWriter sWriter = new StreamWriter(_outputFile))
            {
                BinaryWriter writer = new BinaryWriter(sWriter.BaseStream);

                byte b;
                for (int i = 0; i <= lines.Keys.Max(); i++)
                {
                    b = (byte)(lines.Keys.Any(k => k == i) ? lines[i] : 0x0);
                    writer.Write(b);
                }

            }
        }

        private void ParseIntermedieryFile(string intermediaryFile, Dictionary<int, byte> lines, Dictionary<int, string> debugLines)
        {
            string labelVal = "";
            using (StreamReader reader = new StreamReader(intermediaryFile))
            {
                while (!reader.EndOfStream)
                {
                    var line = reader.ReadLine();
                    var rets = _parser.ParseLine(0, line,_symbolTable);

                    if (rets == null) continue; //null check
                    foreach (var ret in rets)
                    {
                        lines.Add(ret.address, ret.opcode);
                    }
                    if (rets.Count > 0 &&  _symbolTable.FindByAddress(rets[0].address))// && )
                    {// symbol exists, so label does as well
                        labelVal= _symbolTable.FindItem(rets[0].address).name.Trim().PadRight(8, ' ');
                       
                    } else if (line[0] == 'L')
                        //label
                        labelVal = line.Split(' ')[2].Replace(":", "").Trim().PadLeft(8, ' ');
                    else
                        labelVal = "".PadLeft(8, ' ');

                    #region Debug File
                    string data="";
                    if (rets.Count > 0)
                    {

                        if (rets[0].opObj.OpCodeInstruction != null) // instruction
                        {
                            //check if label is registered from previous entry line
                            if (debugLines.ContainsKey(rets[0].address))
                            {
                                var words = debugLines[rets[0].address].Split(' ').ToList();
                                words.RemoveAll(s => s.Length == 0);
                                labelVal = words[0].Trim().PadLeft(7, ' ').PadRight(8, ' ');
                                bool isLabel = (debugLines[rets[0].address][35] == 'L');
                                if (isLabel) debugLines.Remove(rets[0].address);
                            }

                            switch (rets.Count)
                            {
                                case 2:
                                    data = rets[1].opcode.ToString("X").PadLeft(5, ' ');
                                    break;
                                case 3:
                                    data = rets[1].opcode.ToString("X").PadLeft(2, ' ') + ", " + rets[2].opcode.ToString("X").PadRight(1, ' ');
                                    break;
                                default:
                                    data = "\t\t";
                                    break;
                            }

                            debugLines.Add(rets[0].address, labelVal + String.Format("{0:X}", rets[0].opcode).PadLeft(2, '0').PadLeft(4, ' ').PadRight(7, ' ') +
                                       data + rets[0].opObj.OpCodeInstruction.PadRight(10, ' ').PadLeft(14, ' ') +
                                      line);
                            continue;
                        }
                        else
                        {
                            //write out data section
                            for (int i = 0; i < rets.Count; i++)
                            {
                                data += rets[i].opcode.ToString() + ", ";
                            }
                            if (rets.Count > 0)
                                data = data.Remove(data.Length - 2);


                            debugLines.Add(Int32.Parse(line.Split(' ')[0].Substring(1), System.Globalization.NumberStyles.HexNumber), labelVal + data.PadRight(26, ' ') +
                                  line);
                        }
                    }else
                    //label
                    if (!string.IsNullOrEmpty(labelVal.Trim()))
                        debugLines.Add(Int32.Parse(line.Split(' ')[0].Substring(1), System.Globalization.NumberStyles.HexNumber), labelVal + "".PadRight(27, ' ') +
                              line);
                    #endregion
                }
            }
        }

        private static void WriteDebugFile(string dbgFileName, Dictionary<int, string> debugLines)
        {
            bool startedData = false;
            using (StreamWriter write = new StreamWriter(dbgFileName, true))
            {
                write.WriteLine(String.Format("{0}\t{1}\t{2}\t{3}\t{4}\t\t\t{5}", "Address", "Label", "OpCode", "Data", "Machine", "Assembly"));
                write.WriteLine("".PadLeft(70, '-'));
                int lastI = 0;
                for (int i = 0; i <= debugLines.Keys.Max(); i++)
                {
                    if (debugLines.Keys.Contains(i))
                    {
                        if (debugLines[i][35] == 'D')
                        {
                            if (!startedData)
                            {
                                startedData = true;
                                write.WriteLine(". DATA SECTION");
                            }
                        }
                        lastI = i;
                        write.WriteLine(String.Format("{0:X}", i).PadLeft(6, '0') + ": " + debugLines[i]);
                    }
                    if (i > 0 && i % 0xF == 0 && i - lastI < 12)
                        write.WriteLine();
                }
            }

            return;
        }

    }
}
