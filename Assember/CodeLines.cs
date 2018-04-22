using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Assember.DirectiveList;

namespace Assember
{
    public class CodeLines
    {
        private List<SourceLine> _codeLines;
        Pass1aLineParser _parser;

        public CodeLines(int capacity = 255)
        {
            _codeLines = new List<SourceLine>(capacity);
        }
        /// <summary>
        /// Adds code line. returns number of bytes required by line of code
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        //public void Add(string line)
        //{
        //    _codeLines.Add(_parser.ParseLine(line));

        //}
        //parse out proc

    }
}
