#ifndef UNCALCULATEDSYMBOLSTABLE_H_
#define UNCALCULATEDSYMBOLSTABLE_H_

#include <string>
#include <cstdint>
#include <unordered_map>
#include <forward_list>
#include <fstream>
#include <iostream>
using namespace std;

typedef int16_t word;

struct Section;
class SymbolTable;
class RelocationTable;

class UncalculatedSymbolsTable
{
public:
    struct Symbol
    {
        string name;
        string expression;
        word value;
        forward_list<string> symbols;

        Symbol(string name, string expression);
        bool calculateValue(SymbolTable *symbTable, RelocationTable *relTable);
        word parseOperand(string operand, SymbolTable *symbTable, RelocationTable *relTable, bool minusSign, bool *status = nullptr);
    };

    UncalculatedSymbolsTable(SymbolTable *symbTable, RelocationTable *relTable) : symbTable(symbTable), relTable(relTable) {}
    ~UncalculatedSymbolsTable();
    void add(string name, string expression, Section *section = nullptr);
    Symbol *get(string name);
    void write(ofstream& output) const;
    bool calculateAll();

private:
    SymbolTable *symbTable;
    RelocationTable *relTable;
    unordered_map<string, UncalculatedSymbolsTable::Symbol*> symbols;
};

#endif
