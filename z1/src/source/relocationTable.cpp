#include "../header/relocationTable.hpp"
#include "../header/symbolTable.hpp"
#include "../header/section.hpp"
#include "../header/sectionTable.hpp"
#include <iomanip>
using namespace std;

void RelocationTable::add(string name, Section *section, unsigned offset, RelocationType type, bool plus)
{
    records.push_front(Record(name, section, offset, type, plus));
}

bool operator==(const RelocationTable::Record &r1, const RelocationTable::Record &r2)
{
    return r1.name == r2.name && r1.offset == r2.offset && r1.section == r2.section && r1.type == r2.type && r1.plus == r2.plus;
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
            output << left << setw(10) << setfill(' ') << (symb ? (symb->global || !symb->defined ? symb->id : symb->section->id) : 0);
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
    forward_list<Record> to_remove;
    
    for (Record r : records)
    {
        auto s = symbols.find(r.name);
        if (s == symbols.end())
            continue;

        UncalculatedSymbolsTable::Symbol *usymbol = s->second;
        SymbolTable::Symbol *symbol = symbTable->getSymbol(usymbol->name);

        if (symbol->defined && symbol->global && symbol->section)
        {
            word val = 0;
            val |= symbol->section->bytes[r.offset] & 0xff;
            if (r.type != R_X86_64_8)
                val |= symbol->section->bytes[r.offset + 1] << 8;

            val -= symbol->value;

            symbol->section->bytes[r.offset] = val & 0xff;
            if (r.type != R_X86_64_8)
                symbol->section->bytes[r.offset + 1] = val >> 8 & 0xff;
        }

        to_remove.push_front(r);

        for (auto s : usymbol->symbols)
            records.push_front(Record(s.second, r.section, r.offset, r.type, !(s.first ^ r.plus)));
    }
    for (Record r : to_remove)
        records.remove(r);
    to_remove.clear();
}

void RelocationTable::replace()
{
    records.remove_if(*this);

    for (Record &r : records)
    {
        auto symbol = symbTable->getSymbol(r.name);
        if (symbol && symbol->defined && symbol->section)
            if (!symbol->global)
                r.name = symbol->section->name;
            else if (r.type != R_X86_64_PC16)
            {
                r.section->bytes[r.offset] = 0;
                if (r.type == R_X86_64_16)
                    r.section->bytes[r.offset + 1] = 0;
            }
    }
}

bool RelocationTable::operator()(const RelocationTable::Record &r)
{
    SymbolTable::Symbol *symb = symbTable->getSymbol(r.name);
    if (symb && symb->defined)
    {
        switch (r.type)
        {
        case R_X86_64_PC16:
            if (symb->section == r.section)
            {
                word number = r.section->bytes[r.offset] & 0xff | r.section->bytes[r.offset + 1] << 8;
                number -= r.offset;
                r.section->bytes[r.offset] = number & 0xff;
                r.section->bytes[r.offset + 1] = number >> 8 & 0xff;
                return true;
            }
            break;
        case R_X86_64_16:
            if (!symb->section)
                r.section->bytes[r.offset + 1] = symb->value >> 8 & 0xff;
        case R_X86_64_8:
            if (!symb->section)
                r.section->bytes[r.offset] = symb->value & 0xff;
            return !symb->section;
        }
    }
    return !symb;
}

void RelocationTable::sort()
{
    records.sort();
}

bool operator<(const RelocationTable::Record &r1, const RelocationTable::Record &r2)
{
    return r1.section->id < r2.section->id || r1.section->id == r2.section->id && r1.offset < r2.offset;
}

unsigned RelocationTable::writeBinary(ofstream &output, Section *section)
{
    struct RecordBinary
    {
        unsigned symbol;
        unsigned offset;
        RelocationType type;
        bool plus;
    };

    RecordBinary rb;
    unsigned cnt = 0;
    for (Record &r : records)
    {
        if (r.section->id == section->id)
        {
            ++cnt;
            SymbolTable::Symbol *symb = symbTable->getSymbol(r.name);
            rb.symbol = symb ? (symb->global || !symb->defined ? symb->id : symb->section->id) : 0;
            rb.offset = r.offset;
            rb.type = r.type;
            rb.plus = r.plus;
            output.write((char *)&rb, sizeof(rb));
        }
    }

    return cnt;
}
