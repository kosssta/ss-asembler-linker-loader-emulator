#include "symbolTable.hpp"
#include "symbolAlreadyDefinedError.hpp"
#include "section.hpp"

unsigned SymbolTable::insertSymbol(string name, bool defined, word value, Section *section)
{
    Symbol *symb = getSymbol(name);
    if (!symb)
    {
        symb = new Symbol();
        symb->name = name;
        if (defined)
            symb->section = section;
        symb->value = value;
        symb->global = false;
        symb->defined = defined;
        symb->id = nextId++;
        symbols[name] = symb;
        return symb->id;
    }
    else if (symb->defined)
    {
        throw SymbolAlreadyDefinedError(symb->name);
    }
    else
    {
        symb->defined = defined;
        symb->value = value;
        if (defined) {
            symb->section = section;
            symb->clearFLink();
        }
    }
    return 0;
}

SymbolTable::Symbol *SymbolTable::getSymbol(string name)
{
    unordered_map<string, Symbol *>::iterator ret = symbols.find(name);
    return ret == symbols.end() ? nullptr : ret->second;
}

void SymbolTable::setSymbolGlobal(string name)
{
    Symbol *symb = getSymbol(name);
    if (symb)
        symb->global = true;
    else
    {
        symb = new Symbol();
        symb->name = name;
        symb->global = true;
        symb->defined = false;
        symb->id = nextId++;
        symbols[name] = symb;
    }
}

void SymbolTable::Symbol::clearFLink()
{
    while (flink)
    { 
        word tmp = value;
        for (unsigned i = 0; i < flink->size; ++i)
        {
            flink->section->bytes[flink->location + i] = tmp & 0xff;
            tmp >>= 8;
        }
        flink = flink->next;
    }
}

void SymbolTable::write() const
{
    cout << "Symbol table" << endl;
    cout << "Name\t"
         << "Value\t"
         << "Section\t"
         << "Defined\t"
         << "Global\t"
         << "Id\t" << endl;
    for (auto &symb : symbols)
    {
        Symbol *s = symb.second;
        cout << s->name << '\t' << s->value << '\t' << ((long)s->section & 0xffff) << '\t' << (s->defined ? "true" : "false") << '\t'
             << (s->global ? "true" : "false") << '\t' << s->id << endl;
    }
    cout << endl
         << "Extern symbols:\n";
    for (auto &symb : externSymbols)
    {
        cout << symb.second << endl;
    }
    if (externSymbols.empty())
        cout << "No extern symbols" << endl;
}

void SymbolTable::insertExternSymbol(string name)
{
    externSymbols[name] = name;
}
