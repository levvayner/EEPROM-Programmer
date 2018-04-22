using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Assember
{

    class SourceLine
    {
        public bool IsDirective; //is it needed if directive is nullable?
        public bool IsCode { get { return Code.HasValue; } }
        public bool IsLabel { get { return Label.HasValue; } }
        public bool IsData;
        public bool IsValidLine;

        public bool IsAddressAssigned { get; private set; }
        public short Address;
        public int WordLength; //number of words in line
        public int ByteLength { get; set; } // => IsDirective ? Directive.HasValue ? Directive.Value.ByteLength : 0 : Code.HasValue ? Code.Value.ByteLength : 0;
        //number of bytes this line will require in machine code
        public Nullable<Directive> Directive;
        public Nullable<Label> Label;
        public Nullable<CodeLine> Code;
        public string SourceText;
        public string Comment; //if comment is present

        public void AssignAddress(short Address)
        {
            this.Address = Address;
            IsAddressAssigned = true;
        }
        public static bool operator ==(SourceLine sourceLine1, SourceLine sourceLine2)
        {
            if (object.ReferenceEquals(sourceLine1, null))
            {
                return object.ReferenceEquals(sourceLine2, null);
            }

            return sourceLine1.Equals(sourceLine2);
        }
        public static bool operator !=(SourceLine sourceLine1, SourceLine sourceLine2)
        {
            if (object.ReferenceEquals(sourceLine1, null))
            {
                return object.ReferenceEquals(sourceLine2, null);
            }

            return !sourceLine1.Equals(sourceLine2);
        }
    }
    struct Label
    {
        public bool isLocal;
        public string label;
    }
    public enum DirectiveType
    {
        ORG = 0,
        DB =  1,
        DW = 2,
        DD = 3,
        EQU = 4,
        PROC = 5,
        ENDP = 6,
        _code = 7,
        _data = 8,
        _stack = 9,
        _model = 10,
        _title = 11,
        END = 12
            
    }
    public struct Directive
    {
        public string name;
        public string description; //for later use, to help developer
        public byte ByteLength;
        public bool isAbsolute;
        public bool hasSymbol;
        public DirectiveType dType;

        public Directive(string Name, DirectiveType directiveType, byte byteLength, bool IsAbsolute, bool HasSymbol, string Description = "")
        {
            if (String.IsNullOrEmpty(Description)) description = Name;
            else description = Description;
            name = Name;
            ByteLength = byteLength;
            isAbsolute = IsAbsolute;
            hasSymbol = HasSymbol;
            dType = directiveType;
            //dType = DirectiveList.GetDirectiveType(name);
        }
    }

    public struct CodeLine
    {
        public int ByteLength; //number of bytes this line will require in machine code
        //public string sourceCode; //code of line
        
    }

    public static class DirectiveList
    {
        
        private static List<Directive> _directives = new List<Directive>(new[] {
            new Directive( "DB", DirectiveType.DB, 1, false, true ,"Declare space for byte"),
            new Directive( "DW", DirectiveType.DW, 2, false, true, "Declare space for word"),
            new Directive( "DD", DirectiveType.DD, 4, false, true, "Declare space for double word"),
            new Directive( ".DATA", DirectiveType._data,  0, false, false, "Defines the start of a data segment"),
            new Directive( ".CODE", DirectiveType._code,  0, false, false, "Defines the start of a code segment"),
            new Directive( "EQU", DirectiveType.EQU, 1, false, true, "Sets variable (memory location/symbol) equal to a constant"),
            new Directive( "ORG", DirectiveType.ORG, 0, true, false, "Define absolute memory location"),
           // new Directive( "ASSUME", DirectiveType.ASSUME, 0, false, false, "Assume segment register locations (variable names)"),
           // new Directive( "SEGMENT",0, false,true, "Defines segment space (to be loaded in a segment register)"),
            new Directive( "END",  DirectiveType.END, 1, false, true, "Defines the end of a program"),
            new Directive( "PROC", DirectiveType.PROC,  0, false, true, "Defines the start of a procedure"),
            new Directive( "ENDP", DirectiveType.ENDP,  0, false, false, "Defines the end of a procedure")});
        public static bool IsDirective(string word)
        {
            return _directives.Any(d => d.name.ToLower() == word.Trim().ToLower());
        }
        public static Directive GetDirective(string word)
        {
            return _directives.FirstOrDefault(d => d.name.ToLower() == word.Trim().ToLower());
        }
        //public static DirectiveType GetDirectiveType(string word)
    }

    abstract class LineParser<TResultType> where TResultType : struct
    {
        public abstract List<TResultType> ParseLine(short addr, string line, SymbolTable symbols);
    }
   
}
