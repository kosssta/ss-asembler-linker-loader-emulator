#ifndef RELOCATION_TABLE_H_
#define RELOCATION_TABLE_H_

#include <string>
#include <forward_list>
#include <unordered_map>
#include "uncalculatedSymbolsTable.hpp"
using namespace std;

struct Section;
class SymbolTable;

class RelocationTable
{
public:
    enum RelocationType
    {
        R_X86_64_PC16,
        R_X86_64_16,
        R_X86_64_8
    };
    struct Record
    {
        string name;
        unsigned offset;
        RelocationType type;
        Section *section;
        bool plus;

        Record(string name, Section *section, unsigned offset, RelocationType type, bool plus) : name(name), offset(offset), type(type), section(section), plus(plus) {}
    };

    RelocationTable(SymbolTable *symb) : symbTable(symb) {}
    void add(string name, Section *section, unsigned offset, RelocationType type, bool plus = true);
    void write() const;
    void add(unordered_map<string, UncalculatedSymbolsTable::Symbol*> symbols);
    void replace();
private:
    forward_list<RelocationTable::Record*> records;
    SymbolTable *symbTable;
};

#endif
