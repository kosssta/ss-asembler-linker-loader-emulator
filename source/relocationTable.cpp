#include "relocationTable.hpp"
#include "symbolTable.hpp"
#include "section.hpp"
#include <iomanip>
using namespace std;

void RelocationTable::add(string name, Section *section, unsigned offset, RelocationType type, bool plus)
{
    records.push_front(Record(name, section, offset, type, plus));
}

bool operator==(const RelocationTable::Record &r1, const RelocationTable::Record &r2)
{
    return r1.name == r2.name && r1.offset == r2.offset && r1.section == r2.section;
}

void RelocationTable::write(ofstream &output) const
{
    output << "=== Relocation table ===" << endl;
    if (records.empty())
    {
        output << "No records" << endl;
        return;
    }

    output << left << setw(10) << setfill(' ') << "Name";
    output << left << setw(8) << setfill(' ') << "Section";
    output << left << setw(8) << setfill(' ') << "Offset";
    output << left << setw(15) << setfill(' ') << "Type";
    output << left << setw(5) << setfill(' ') << "Plus";
    output << endl;

    for (Record r : records)
    {
        output << left << setw(10) << setfill(' ') << r.name;
        output << left << setw(8) << setfill(' ') << (r.section ? r.section->id : 0);
        output << left << setw(8) << setfill(' ') << r.offset;
        output << left << setw(15) << setfill(' ') << (r.type == R_X86_64_PC16 ? "R_X86_64_PC16" : r.type == R_X86_64_8 ? "R_X86_64_8" : "R_X86_64_16");
        output << left << setw(5) << setfill(' ') << (r.plus ? '+' : '-');
        output << endl;
    }
}

void RelocationTable::add(unordered_map<string, UncalculatedSymbolsTable::Symbol *> symbols)
{
    forward_list<Record> tmp;
    forward_list<Record> to_remove;

    for (auto sym : symbols)
    {
        auto *s = sym.second;
        for (Record r : records)
        {
            if (r.name == s->name)
            {
                for (string s : s->symbols)
                    tmp.push_front(Record(s.substr(1), r.section, r.offset, R_X86_64_16, s[0] == '+'));
                to_remove.push_front(r);
            }
        }
    }

    for (Record r : to_remove)
        records.remove(r);
    for (Record r : tmp)
        records.push_front(r);
}

void RelocationTable::replace()
{
    for (Record &r : records)
    {
        auto symbol = symbTable->getSymbol(r.name);
        if (symbol && symbol->defined && symbol->section)
            r.name = r.section->name;
    }
}
