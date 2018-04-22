using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Assember
{
    public static class Extentions
    {
        public static bool ParseValue(this string valueString, out short test)
        {
            test = 0x0;
            bool sucess = false;
            if (valueString.ToUpper().EndsWith("H")) //hex
                try
                {
                    test = Convert.ToInt16(valueString.Remove(valueString.Length - 1), 16);
                    sucess = true;
                }
                catch (Exception ex) { Console.WriteLine("Failed to parse HEX value {0}: {1}", valueString, ex.Message); }
            else if (valueString.ToUpper().EndsWith("O"))
            {
                try
                {
                    test = Convert.ToInt16(valueString, 8);
                    sucess = true;
                }
                catch (Exception ex) { Console.WriteLine("Failed to parse OCT value {0}: {1}", valueString, ex.Message); }
            }
            else
                sucess = Int16.TryParse(valueString, out test);

            return sucess;
        }
    }
}
