#ifndef UNCALCULATED_SYMBOLS_TABLE_H_
#define UNCALCULATED_SYMBOLS_TABLE_H_

#include <string>
#include <cstdint>
#include <unordered_map>
#include <list>
#include <iostream>
#include <utility>
using namespace std;

typedef int16_t word;

class SymbolTable;
struct Section;
class RelocationTable;

class UncalculatedSymbolsTable
{
public:
    struct RelocationIndex
    {
        unordered_map<Section *, int> index;
        void add(Section *symbol, bool plus);
    };

    struct Symbol
    {
        string name;
        RelocationIndex index;
        list<pair<bool, string>> symbols;

        Symbol(string name, string expression, SymbolTable *symbTable);
        void calculateValue(UncalculatedSymbolsTable *uTable, SymbolTable *symbTable, RelocationTable *relTable);
        bool parseOperand(string operand, SymbolTable *symbTable, bool minusSign);
        void checkIndex(SymbolTable *symbTable);
    };

    UncalculatedSymbolsTable(SymbolTable *symbTable, RelocationTable *relTable) : symbTable(symbTable), relTable(relTable) {}
    ~UncalculatedSymbolsTable();
    void add(string name, string expression);
    Symbol *get(string name);
    void calculateAll();

private:
    SymbolTable *symbTable;
    RelocationTable *relTable;
    unordered_map<string, UncalculatedSymbolsTable::Symbol *> symbols;
};

#endif
