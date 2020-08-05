#include "../header/symbolTable.hpp"
#include "../header/syntaxErrors.hpp"
#include "../header/section.hpp"

unsigned SymbolTable::addSymbol(string name, word value, Section *section, bool global)
{
    Symbol *s = getSymbol(name);
    if (!s)
    {
        symbols[name] = new Symbol(name, value, section, global, nextId++);
        s = getSymbol(name);
    }
    else if (s->global && global)
        throw MultipleDefinitionError(name);

    s->global |= global;
    if (global)
    {
        s->section = section;
        s->value = value;
    }
    return s->id;
}

SymbolTable::Symbol *SymbolTable::getSymbol(string name) const
{
    auto s = symbols.find(name);
    return s == symbols.end() ? nullptr : s->second;
}

SymbolTable::Symbol *SymbolTable::getSymbol(unsigned id) const
{
    for (auto s : symbols)
        if (s.second->id == id)
            return s.second;
    return nullptr;
}

bool SymbolTable::isSection(string name) const
{
    Symbol *symb = getSymbol(name);
    return symb && symb->section && symb->section->id == symb->id;
}

void SymbolTable::checkUndefinedSymbols() const
{
    string undefinedSymbols;

    for (auto s : symbols)
        if (!isDefined(s.second))
            undefinedSymbols += (undefinedSymbols.empty() ? "" : ", ") + s.second->name;

    if (!undefinedSymbols.empty())
        throw SyntaxError("Undefined symbols: " + undefinedSymbols);
}

bool SymbolTable::isDefined(SymbolTable::Symbol *symbol) const {
    return symbol && (symbol->name == "data_in" || symbol->name == "data_out" || symbol->name == "timer_cfg" || symbol->section);
}
