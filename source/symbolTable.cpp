#include "symbolTable.hpp"
#include "symbolAlreadyDefinedError.hpp"
#include "section.hpp"

unsigned SymbolTable::insertSymbol(string name, long value, Section *section)
{
    Symbol *symb = getSymbol(name);
    if (!symb)
    {
        symb = new Symbol();
        symb->name = name;
        symb->section = section;
        symb->value = value;
        symb->global = false;
        symb->defined = true;
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
        symb->defined = true;
        symb->value = value;
        symb->section = section;
        symb->clearFLink();
    }
    return 0;
}

SymbolTable::Symbol *SymbolTable::getSymbol(string name)
{
    unordered_map<string, Symbol*>::iterator ret = symbols.find(name);
    if(ret == symbols.end()) return nullptr;
    return ret->second;
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
    }
}

void SymbolTable::Symbol::clearFLink() {
    while(flink) {
        // neispravno
        section->bytes.add(value);
        flink = flink->next;
    }
}
