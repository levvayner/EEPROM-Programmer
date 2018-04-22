using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
namespace Assember
{
    class Program
    {
        private static Pass1 pass1;
        private static Pass2 pass2;
        //private Pass2 pass2;
        static void Main(string[] args)
        {
            List<string> pars = new List<string>(args);
            List<string> AssemblySources = new List<string>();
            pass1 = new Pass1();
            string desitnationFileName = @"..\..\..\AssemblyProgrammingServer\bin\debug\output1.bin";
            pass2 = new Assember.Pass2(pass1.Symbols, desitnationFileName);
            
            if (pars.Count == 0)
                pars.AddRange(new[] { "/A","test1.asm" });
                
            for (int i = 0; i < pars.Count; i++)
            {
                string sw, cmd;
                if (pars[i].StartsWith("/"))
                {
                    //switch    
                    sw = pars[i];
                    cmd = pars[++i];
                    
                    //check for open quote
                    while (cmd.Count<char>(c => c == '"') % 2 != 0 && i + 1 < pars.Count)
                    {
                        cmd += " " + pars[++i];
                    }

                    char s = sw.Substring(1).Trim().ToUpper()[0];
                    
                    switch (s)
                    { 
                        case 'A': //assemble
                            if (cmd.Any(c => c == ','))
                            {
                                //if comma seperated list, must be ordered with main source first, I think
                                AssemblySources.AddRange(cmd.Split(','));
                                AssemblySources.ForEach(a => a.Trim()); //does this trim the result or returns the trim value and discards it
                            }

                            else
                            {
                                AssemblySources.Add(cmd.Trim());
                            }
                        break;
                        case 'D':
                            desitnationFileName = cmd;
                            break;
                        default:
                            break;
                    }
                }else
                {
                    //free parameter
                    cmd = pars[i];
                }
            }

            //start assembly process

            if (pass1.ProcessFiles(AssemblySources.ToArray()))
            {
                if(pass2.ProcessIntermediary(pass1.IntermediateFile))
                {
                    Console.WriteLine("Sucess! Created {0}.",pass2.OutputFile);
                }
                else Console.WriteLine("Failed Pass 2");
            }
            else Console.WriteLine("Failed Pass 1");
        }

        private static bool Pass2()
        {
            return false;
        }
    }
}
