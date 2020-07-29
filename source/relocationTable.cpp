#include "relocationTable.hpp"
#include "symbolTable.hpp"
#include "section.hpp"
#include <iostream>
using namespace std;

void RelocationTable::add(string name, Section *section, unsigned offset, RelocationType type, bool plus)
{
    records.push_front(new Record(name, section, offset, type, plus));
}

void RelocationTable::write() const
{
    cout << "Relocation table" << endl;
    cout << "Name\tSection\tOffset\tType\t\tPlus" << endl;
    for (Record *r : records)
        cout << r->name << '\t' << ((long)r->section & 0xffff) << '\t' << r->offset << '\t' << (r->type == R_X86_64_PC16 ? "R_X86_64_PC16" : r->type == R_X86_64_8 ? "R_X86_64_8" : "R_X86_64_16") << '\t' << (r->plus ? '+' : '-') << endl;
}

void RelocationTable::add(unordered_map<string, UncalculatedSymbolsTable::Symbol *> symbols)
{
    forward_list<Record*> tmp;
    
    for (auto sym : symbols) {
        auto *s = sym.second;
        for (Record *r : records)
        {
            if (r->name == s->name)
            {
                for (string s : s->symbols)
                    tmp.push_front(new Record(s.substr(1), r->section, r->offset, R_X86_64_16, s[0] == '+'));
                records.remove(r);
                delete r;
            }
        }
    }

    for (Record *r : tmp)
        records.push_front(r);
}

void RelocationTable::replace()
{
    for (Record *r : records)
    {
        auto symbol = symbTable->getSymbol(r->name);
        if (symbol && symbol->defined)
            r->name = r->section->name;
    }
}
