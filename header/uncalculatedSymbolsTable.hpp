#ifndef UNCALCULATEDSYMBOLSTABLE_H_
#define UNCALCULATEDSYMBOLSTABLE_H_

#include <string>
#include <cstdint>
#include <vector>
#include "symbolTable.hpp"
using namespace std;

typedef int16_t word;

struct Section;

class UncalculatedSymbolsTable
{
public:
    struct Symbol
    {
        string name;
        string expression;
        Section *section;
        word value;

        Symbol(string name, string expression, Section *section) : name(name), expression(expression), section(section), value(0) {}
        bool calculateValue(SymbolTable *symbTable);
        word parseOperand(string operand, SymbolTable *symbTable, bool *status = nullptr);
    };

    UncalculatedSymbolsTable(SymbolTable *symbTable) : symbTable(symbTable) {}
    void add(string name, string expression, Section *section);
    void write() const;
    bool calculateAll();

private:
    SymbolTable *symbTable;
    vector<Symbol*> symbols;
};

#endif
