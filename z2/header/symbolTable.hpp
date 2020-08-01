#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <string>
#include <unordered_map>
#include <cstdint>
using namespace std;

struct Section;

typedef int16_t word;

class SymbolTable
{
public:
    struct Symbol
    {
        string name;
        word value;
        Section *section = nullptr;
        bool global;
        unsigned id;

        Symbol(string name, word value, Section *section, bool global, unsigned id) : name(name), value(value), section(section), global(global), id(id) {}
    };

    unsigned addSymbol(string name, word value, Section *section, bool global);
    Symbol* getSymbol(string name) const;

private:
    unordered_map<string, Symbol*> symbols;
    unsigned nextId = 1;
};

#endif
