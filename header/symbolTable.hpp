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
        FLink *next;

        FLink(unsigned location, FLink *next = nullptr) : location(location), next(next) {}
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

        void addFLink(unsigned location) {
            flink = new FLink(location, flink);
        }
        void clearFLink();
    };

    unsigned insertSymbol(string name, long value, Section *section = nullptr);
    Symbol* getSymbol(string name);
    void setSymbolGlobal(string name);
private:
    unordered_map<string, Symbol*> symbols;
    unsigned nextId = 1;
};

#endif
