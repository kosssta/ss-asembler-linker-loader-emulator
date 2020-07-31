#include "relocationTable.hpp"
#include "symbolTable.hpp"
#include "section.hpp"
#include "sectionTable.hpp"
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

void RelocationTable::write(ofstream &output, SectionTable *sections)
{
    sort();

    forward_list<string> names = sections->getAllNames();
    for (string name : names)
    {
        Section *section = sections->findSection(name);
        if (!section)
            continue;

        list<const Record *> rec;
        for (const Record &r : records)
            if (r.section == section)
                rec.push_back(&r);

        if (rec.size() == 0)
            continue;

        output << ".rel" + name << endl;
        output << "Length = " << rec.size() << endl;
        output << left << setw(10) << setfill(' ') << "Id";
        output << left << setw(8) << setfill(' ') << "Offset";
        output << left << setw(15) << setfill(' ') << "Type";
        output << left << setw(5) << setfill(' ') << "Plus";
        output << endl;

        for (const Record *r : rec)
        {
            SymbolTable::Symbol *symb = symbTable->getSymbol(r->name);
            output << left << setw(10) << setfill(' ') << (symb && symb->section ? (symb->global ? symb->id : symb->section->id) : 0);
            output << left << setw(8) << setfill(' ') << r->offset;
            output << left << setw(15) << setfill(' ') << (r->type == R_X86_64_PC16 ? "R_X86_64_PC16" : r->type == R_X86_64_8 ? "R_X86_64_8" : "R_X86_64_16");
            output << left << setw(5) << setfill(' ') << (r->plus ? '+' : '-');
            output << endl;
        }
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
    records.remove_if(*this);

    for (Record &r : records)
    {
        auto symbol = symbTable->getSymbol(r.name);
        if (symbol && symbol->defined && symbol->section)
            if (!symbol->global)
                r.name = r.section->name;
            else
            {
                r.section->bytes[r.offset] = 0;
                if (r.type != R_X86_64_8)
                    r.section->bytes[r.offset + 1] = 0;
            }
    }
}

bool RelocationTable::operator()(const RelocationTable::Record &r)
{
    SymbolTable::Symbol *symb = symbTable->getSymbol(r.name);
    return !symb || !symb->defined || !symb->section || r.type == R_X86_64_PC16 && !symb->global && symb->section == r.section;
}

void RelocationTable::sort()
{
    records.sort();
}

bool operator<(const RelocationTable::Record &r1, const RelocationTable::Record &r2)
{
    return r1.section->id < r2.section->id || r1.section->id == r2.section->id && r1.offset < r2.offset;
}

unsigned RelocationTable::writeBinary(ofstream &output)
{
    struct RecordBinary {
        unsigned symbol;
        unsigned offset;
        unsigned section;
        RelocationType type;
        bool plus;
    };

    RecordBinary rb;
    unsigned cnt = 0;
    for (Record &r : records)
    {
        ++cnt;
        SymbolTable::Symbol *symb = symbTable->getSymbol(r.name);
        rb.symbol = symb && symb->section ? (symb->global ? symb->id : symb->section->id) : 0;
        rb.offset = r.offset;
        rb.section = r.section ? r.section->id : 0;
        rb.type = r.type;
        rb.plus = r.plus;
        output.write((char *) &rb, sizeof(rb));
    }

    return cnt;
}
