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
        unsigned location;
        char size;
        FLink *next;

        FLink(unsigned location, char size, FLink *next = nullptr) : location(location), size(size), next(next) {}
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

        void addFLink(unsigned location, char size) {
            flink = new FLink(location, size, flink);
        }
        void clearFLink();
    };

    unsigned insertSymbol(string name, bool defined, long value = 0, Section *section = nullptr);
    Symbol* getSymbol(string name);
    void setSymbolGlobal(string name);
private:
    unordered_map<string, Symbol*> symbols;
    unsigned nextId = 1;
};

#endif
