#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <string>
#include <unordered_map>
#include <forward_list>
#include <list>
#include <cstdint>
#include <fstream>
#include <iostream>
using namespace std;

struct Section;

typedef int16_t word;

class SymbolTable
{
public:
    struct FLink
    {
        Section *section;
        unsigned location;
        char size;

        FLink(Section *section, unsigned location, char size) : section(section), location(location), size(size) {}
    };

    struct Symbol
    {
        string name;
        word value;
        Section *section = nullptr;
        bool defined;
        bool global;
        unsigned id;
        forward_list<FLink> flink;

        void addFLink(Section *section, unsigned location, char size) {
            flink.push_front(FLink(section, location, size));
        }
        void clearFLink();
        friend bool operator<(const Symbol &s1, const Symbol &s2);
    };

    ~SymbolTable();
    unsigned insertSymbol(string name, bool defined, word value = 0, Section *section = nullptr);
    void insertExternSymbol(string name);
    void setSymbolGlobal(string name);
    void removeAllLocalSymbols();
    
    Symbol* getSymbol(string name) const;
    Symbol *getExternSymbol(string name) const;
    list<Symbol> sort() const;
    void write(ofstream& output) const;

private:
    unordered_map<string, Symbol*> symbols;
    unordered_map<string, string> externSymbols;
    unsigned nextId = 1;
};

#endif
