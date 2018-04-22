using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Assember
{
    
    class Procedure
    {
        private int lineIdx = 0;

        public List<SourceLine> CodeLines { get; private set; }
        public string Name { get; private set; }
        //public int LineStartNum { get; private set; }
        //public int LineEndNum { get; private set; }
        public int Length { get; private set; }
        public bool IsFar { get; private set; }
        public int ParamCount { get; private set; } //byte only for now

        public Procedure(string procedureLine)
        {
            string[] words = procedureLine.Trim().Split(' ');
            this.Name = words[1];
            this.CodeLines = new List<SourceLine>();
            lineIdx = 0;
            if(words.Length > 2)
            {
                IsFar = (words[2].ToLower() == "far");
                if(words.Length > 3)
                {
                    //have arguments to pass
                    //TODO: implement later, use stack for now
                    //var args = String.Join(" ", words.Skip(3));
                    //args = args.Replace("args:", "").Trim();
                }

            }
        }

        public void AddLine(SourceLine line)
        {
            //if(!line.IsCode)
            //{
                CodeLines.Add(line);
            //}
            lineIdx++;
        }
    }
}
