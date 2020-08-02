#include "../header/symbolTable.hpp"
#include "../header/section.hpp"
#include "../header/syntaxErrors.hpp"
#include <iomanip>
using namespace std;

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
        throw SymbolRedefinitionError(symb->name);
    }
    else
    {
        symb->defined = defined;
        symb->value = value;
        if (defined)
        {
            symb->section = section;
            if (symb->section && symb->section->id == symb->id)
            {
                symb->id = nextId++;
                symb->section->id = symb->id;
            }
            else
                symb->id = nextId++;
            symb->clearFLink();
        }
    }
    return 0;
}

SymbolTable::Symbol *SymbolTable::getSymbol(string name) const
{
    auto ret = symbols.find(name);
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

    if (getExternSymbol(name))
        throw SyntaxError("Symbol " + name + " must be either global or extern, not both");
}

void SymbolTable::Symbol::clearFLink()
{
    if (global)
    {
        flink.clear();
        return;
    }

    while (!flink.empty())
    {
        FLink f = flink.front();
        word val = 0;
        for (unsigned i = 0; i < f.size; ++i)
        {
            val |= f.section->bytes[f.location + i] << i * 8;
        }
        val += value;
        for (unsigned i = 0; i < f.size; ++i)
        {
            f.section->bytes[f.location + i] = val & 0xff;
            val >>= 8;
        }
        flink.pop_front();
    }
}

void SymbolTable::write(ofstream &output)
{
    output << "=== Symbol table ===" << endl;
    list<Symbol> symbols = sort();

    if (symbols.empty())
    {
        output << "No symbols" << endl;
        return;
    }

    output << left << setw(10) << setfill(' ') << "Name";
    output << left << setw(8) << setfill(' ') << "Value";
    output << left << setw(8) << setfill(' ') << "Section";
    output << left << setw(9) << setfill(' ') << "Defined";
    output << left << setw(9) << setfill(' ') << "Global";
    output << left << setw(8) << setfill(' ') << "Id";
    output << endl;

    for (auto &s : symbols)
    {
        output << left << setw(10) << setfill(' ') << s.name;
        output << left << setw(8) << setfill(' ') << s.value;
        output << left << setw(8) << setfill(' ') << (s.section ? s.section->id : 0);
        output << left << setw(9) << setfill(' ') << (s.defined ? "true" : "false");
        output << left << setw(9) << setfill(' ') << (s.global ? "true" : "false");
        output << left << setw(8) << setfill(' ') << s.id;
        output << endl;
    }
}

void SymbolTable::insertExternSymbol(string name)
{
    Symbol *s = getSymbol(name);
    if (s && s->global)
        throw SyntaxError("Symbol " + name + " must be either global or extern, not both");
    externSymbols[name] = name;
}

void SymbolTable::removeAllLocalSymbols()
{
    forward_list<string> tmp;
    for (auto s : symbols)
    {
        Symbol *symb = s.second;
        if (!symb->defined)
        {
            if (externSymbols.find(symb->name) == externSymbols.end())
                throw UnrecognizedSymbol(symb->name);
        }
        else
        {
            if (externSymbols.find(symb->name) != externSymbols.end())
                throw SyntaxError("Symbol " + symb->name + " is extern, but local definition found");
            else if (!symb->global && symb->name[0] != '.')
                tmp.push_front(symb->name);
        }
    }
    for (string name : tmp)
    {
        delete symbols[name];
        symbols.erase(name);
    }
}

SymbolTable::Symbol *SymbolTable::getExternSymbol(string name) const
{
    auto ret = externSymbols.find(name);
    return ret == externSymbols.end() ? nullptr : getSymbol(ret->second);
}

SymbolTable::~SymbolTable()
{
    for (auto s : symbols)
        delete s.second;
}

list<SymbolTable::Symbol> SymbolTable::sort()
{
    unsigned nextId = 1;
    list<Symbol> sortedSymbols;

    for (auto s : symbols)
        sortedSymbols.push_back(*s.second);

    sortedSymbols.sort();
    for (Symbol &s : sortedSymbols)
    {
        s.id = nextId++;
        symbols[s.name]->id = s.id;
        if (s.name[0] == '.')
            s.section->id = s.id;
    }

    return sortedSymbols;
}

bool operator<(const SymbolTable::Symbol &s1, const SymbolTable::Symbol &s2)
{
    return s1.name[0] == '.' && s2.name[0] != '.' || (s1.name[0] == '.' && s2.name[0] == '.' || s1.name[0] != '.' && s2.name[0] != '.') && s1.id < s2.id;
}

unsigned SymbolTable::writeBinary(ofstream &output)
{
    list<Symbol> symbols = sort();
    struct BinarySymbol
    {
        unsigned nameLength;
        word value;
        unsigned section;
        bool global;
        unsigned id;
    };

    BinarySymbol bs;
    for (Symbol &s : symbols)
    {
        bs.nameLength = s.name.size();
        bs.value = s.value;
        bs.section = s.section ? s.section->id : 0;
        bs.global = s.global;
        bs.id = s.id;
        output.write((char *)&bs, sizeof(bs));
        output << s.name;
    }

    return symbols.size();
}
