using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace AssemblyProgrammingServer
{
    class PortReader

        {
            private static SerialPort port = new SerialPort("COM3");
            private static byte[] buffer = new byte[256];
            private static int runNumber = 0;
            string workingDir;
        private static string FileName;
            public PortReader(string fileName)
            {
                port.BaudRate = 115200;// 1843200;
                FileName = fileName;
                port.ReadBufferSize = 256;
                port.Open();
                runNumber = new Random(DateTime.Now.Hour).Next(100000, 9000000);
                workingDir = Path.Combine(Environment.CurrentDirectory, runNumber.ToString());
            }
        public void WriteBinary()
        {
            try
            {
                //port.Write(new[] { (byte)'c' },0,1);
                EstablishProgrammingMode();
                List<byte> data = new List<byte>();
                using (BinaryReader reader = new BinaryReader(new StreamReader(FileName).BaseStream))
                {
                    while (reader.BaseStream.Position != reader.BaseStream.Length)
                        data.Add(reader.ReadByte());
                }
                data.Add(0x0); //null terminator for file
                var outStream = port.BaseStream;                
                string linecount = data.Count.ToString();
                Array.Copy(Encoding.ASCII.GetBytes(linecount), buffer, 2);
                Console.WriteLine("Passing line count as {0}",linecount);
                buffer[0] = (byte)((byte)(data.Count >> 8) & 0xFF);
                buffer[1] = (byte)(data.Count & 0xFF);
                outStream.Write(buffer, 0, 2);
                Thread.Sleep(100);
                
                Console.WriteLine("Established line count.. writting program");
                //Thread.Sleep(100);
                string retVal;
                for (int lineNum = 0; lineNum < data.Count; lineNum++)
                {
                    if (data[lineNum] == 0) continue;
                    Console.WriteLine("Writing 0x{0} at address 0x{1}", Convert.ToInt32(data[lineNum]), Convert.ToInt32(lineNum));
                    //Array.Copy(Encoding.ASCII.GetBytes(lineNum.ToString().PadLeft(4, '0')), buffer, 4);
                    //Array.Copy(Encoding.ASCII.GetBytes(lines[lineNum].PadLeft(3, '0')),0, buffer, 4,3);
                    buffer[0] = ((byte)((byte)(lineNum >> 8) & 0xFF));
                    buffer[1] = ((byte)(lineNum & 0xFF));
                    buffer[2] = ((byte)((byte)(Convert.ToInt32(data[lineNum])) & 0xFF));
                    outStream.Write(buffer, 0, 3);
                    retVal = ReadBytes(1, outStream)[0];
                    //if (!retVal.StartsWith("<<")) lineNum--; // retry
                    Thread.Sleep(15);
                }
                Thread.Sleep(3000); // give it a sec to flush out
                port.DiscardOutBuffer();
                Console.WriteLine("Completed upload of binary to EEPROM. Data has not been validated!");
                Console.ReadKey();
            }
            catch (Exception e)
            {
                throw e;

            }
        }

        void EstablishProgrammingMode()
        {
            var outStream = port.BaseStream;
            ReadBytes(3, outStream); //get first two into lines
            Console.WriteLine("---------------------");
            Console.Write(">> ");
            Array.Copy(Encoding.ASCII.GetBytes("1"), buffer, 1);
            outStream.Write(buffer, 0, 1);
            ReadBytes(11, outStream);
            Console.WriteLine("Established chip count.. setting Mode");

            Console.WriteLine("---------------------");
            Console.Write(">> ");
            Array.Copy(Encoding.ASCII.GetBytes("S"), buffer, 1);
            outStream.Write(buffer, 0, 1);
            ReadBytes(5, outStream);
            Console.WriteLine("Established mode as write.. setting type of programming");

            Console.WriteLine("---------------------");
            Console.Write(">> ");
            Array.Copy(Encoding.ASCII.GetBytes("3"), buffer, 1);
            outStream.Write(buffer, 0, 1);
            ReadBytes(2,outStream);
            Console.WriteLine("Established mode as write program.. setting type of program rom");

            Console.WriteLine("---------------------");
            Console.Write(">> ");
            Array.Copy(Encoding.ASCII.GetBytes("3"), buffer, 1);
            outStream.Write(buffer, 0, 1);
            ReadBytes(1, outStream);
            Console.WriteLine("Established mode as write program from remote server.. ready to write handshake");

            Console.WriteLine("---------------------");
            Console.Write(">> ");
            Array.Copy(Encoding.ASCII.GetBytes("START_PROGRAM"), buffer, 13);
            outStream.Write(buffer, 0, 13);
            ReadBytes(1, outStream);            
            Console.WriteLine("Established handshake from remote server.. ready to write program ");            
        }

        private string[] ReadBytes(int linesToRead,Stream outStream)
        {
            //retVal = Read(outStream);
            System.Threading.Thread.Sleep(10);
            int retLineCount = 0;
            List<string> it = new List<string>();
            DateTime starTime = DateTime.Now;
            while (retLineCount < linesToRead && DateTime.Now.Subtract(starTime).TotalSeconds < 5) // max 2 sec for timeout
            {
                if (port.BytesToRead > 0)
                {
                    it.Add(port.ReadLine());
                    Console.WriteLine(it[it.Count - 1]);
                    Thread.Sleep(10);
                    retLineCount++;
                }
            }

            return it.ToArray();
            
        }

        private byte Read(Stream inputStream)
        {
            byte temp = (byte)inputStream.ReadByte();
            
            return temp;
        }

            //private bool isImageStart(Stream inputStream, int index)
            //{

            //    if (index < COMMAND.Length)
            //    {
            //        try
            //        {
            //            if (COMMAND[index] == Read(inputStream))
            //            {
            //                return isImageStart(inputStream, ++index);
            //            }
            //            else
            //            {
            //                return false;
            //            }
            //        }
            //        catch (Exception ex)
            //        {
            //            Console.WriteLine("Error occured {0}", ex.Message);
            //        }
            //    }
            //    return true;
            //}
        }
    }
