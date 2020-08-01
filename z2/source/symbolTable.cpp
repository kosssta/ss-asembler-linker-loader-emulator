#include "../header/symbolTable.hpp"
#include "../header/syntaxErrors.hpp"

unsigned SymbolTable::addSymbol(string name, word value, Section *section, bool global)
{
    Symbol *s = getSymbol(name);
    if (!s)
    {
        symbols[name] = new Symbol(name, value, section, global, nextId++);
        s = getSymbol(name);
    }
    else if (s->global)
        throw MultipleDefinitionError(name);

    s->global |= global;
    return s->id;
}

SymbolTable::Symbol *SymbolTable::getSymbol(string name) const
{
    auto s = symbols.find(name);
    return s == symbols.end() ? nullptr : s->second;
}
