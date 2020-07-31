#ifndef RELOCATION_TABLE_H_
#define RELOCATION_TABLE_H_

#include "uncalculatedSymbolsTable.hpp"
#include <string>
#include <forward_list>
#include <unordered_map>
#include <fstream>
#include <iostream>
using namespace std;

struct Section;
class SymbolTable;
class SectionTable;

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
    void add(unordered_map<string, UncalculatedSymbolsTable::Symbol*> symbols);
    void replace();
    void write(ofstream& output, SectionTable *sections);
    unsigned writeBinary(ofstream& output);
    void sort();

    friend bool operator==(const Record &r1, const Record &r2);
    friend bool operator<(const Record &r1, const Record &r2);
    bool operator()(const Record &r);
    
private:
    forward_list<RelocationTable::Record> records;
    SymbolTable *symbTable;
};

#endif
