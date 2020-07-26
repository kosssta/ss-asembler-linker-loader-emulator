#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

#include <string>
#include <unordered_map>
using namespace std;

struct Section;

class SymbolTable
{
public:
    struct FLink
    {
        Section *section;
        unsigned location;
        char size;
        FLink *next;

        FLink(Section *section, unsigned location, char size, FLink *next = nullptr) : section(section), location(location), size(size), next(next) {}
    };

    struct Symbol
    {
        string name;
        long value;
        Section *section = nullptr;
        bool defined;
        bool global;
        unsigned id;
        FLink *flink = nullptr;
        Symbol *next = nullptr;

        void addFLink(Section *section, unsigned location, char size) {
            flink = new FLink(section, location, size, flink);
        }
        void clearFLink();
    };

    unsigned insertSymbol(string name, bool defined, long value = 0, Section *section = nullptr);
    Symbol* getSymbol(string name);
    void setSymbolGlobal(string name);
    void write() const;
    void insertExternSymbol(string name);
private:
    unordered_map<string, Symbol*> symbols;
    unordered_map<string, string> externSymbols;
    unsigned nextId = 1;
};

#endif
