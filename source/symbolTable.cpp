#include "symbolTable.hpp"
#include "symbolAlreadyDefinedError.hpp"
#include "section.hpp"

unsigned SymbolTable::insertSymbol(string name, bool defined, long value, Section *section)
{
    Symbol *symb = getSymbol(name);
    if (!symb)
    {
        symb = new Symbol();
        symb->name = name;
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
        symb->section = section;
        symb->clearFLink();
    }
    return 0;
}

SymbolTable::Symbol *SymbolTable::getSymbol(string name)
{
    unordered_map<string, Symbol *>::iterator ret = symbols.find(name);
    if (ret == symbols.end())
        return nullptr;
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

void SymbolTable::Symbol::clearFLink()
{
    while (flink)
    {
        if (flink->size == 1)
            section->bytes.set(flink->location, (byte)value);
        else
        {
            byte *array = new byte[flink->size];
            long tmp = value;
            for (unsigned i = 0; i < flink->size; ++i)
            {
                array[i] = tmp & 0xff;
                tmp >>= 8;
            }
            section->bytes.set(flink->location, array, flink->size);
            delete array;
        }
        flink = flink->next;
    }
}
