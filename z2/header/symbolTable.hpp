#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <string>
#include <unordered_map>
#include <cstdint>
using namespace std;

struct Section;
class Linker;

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
    Symbol* getSymbol(unsigned id) const;
    bool isSection(string name) const;
    void checkUndefinedSymbols() const;
    bool isDefined(Symbol *symbol) const;

    friend class Linker;
private:
    unordered_map<string, Symbol*> symbols;
    unsigned nextId = 1;
};

#endif
