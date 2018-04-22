using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Assember
{

    enum SymbolType
    {
        rel = 0,
        abs = 1,
        ent = 2,
        ext = 3,
        mtdf = 4,
        far = 5
    }
    struct SymbolRef
    {
        public string name;
        public short value;
        public SymbolType type;
        public SymbolRef(string Name, short Value, SymbolType Type)
        {
            name = Name;
            value = Value;
            type = Type;
        }

    }
    class SymbolTable
    {
        List<SymbolRef> _symbols;
        

        public SymbolTable()
        {
            _symbols = new List<SymbolRef>(500);
            
        }
        
        public bool AddItem(string Name, short Value, SymbolType Type)
        {
            if (string.IsNullOrEmpty(Name)) return false;
            //first check that item is not present
            if (_symbols.Any(s => s.name == Name))
            {
                _symbols.Add(new SymbolRef(Name.Trim(), Value, SymbolType.mtdf));
                return false;
            }
            else
                _symbols.Add(new SymbolRef(Name.Trim(), Value, Type));
            return true;
        }

        public SymbolRef FindItem(string Name)
        {
            return _symbols.FirstOrDefault(s => s.name.ToLower() == Name.ToLower().Trim());
        }
        public SymbolRef FindItem(int Addr)
        {
            return _symbols.FirstOrDefault(s => s.value == Addr && !string.IsNullOrEmpty(s.name));
        }
        public bool FindByAddress(int Addr)
        {
            return _symbols.Any(s => s.value == Addr && !string.IsNullOrEmpty(s.name));
        }
    }
}
