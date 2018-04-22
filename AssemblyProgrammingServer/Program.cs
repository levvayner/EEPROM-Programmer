using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AssemblyProgrammingServer
{
    class Program
    {
        private static PortReader _reader;
        static void Main(string[] args)
        {
            string filename = @"Output1.bin";
            for(int i=0;i< args.Length; i++)
                if(args[i] == "/O" && i + 1 < args.Length)
                {
                    filename = args[++i];
                }

            _reader = new PortReader(filename);
            _reader.WriteBinary();
           
        }
        
    }
}
