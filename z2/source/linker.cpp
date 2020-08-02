#include "../header/linker.hpp"
#include "../header/syntaxErrors.hpp"
#include "../header/section.hpp"
#include "../header/relocationTable.hpp"
#include "../header/emulator.hpp"
#include <iostream>
#include <fstream>
using namespace std;

void Linker::link(list<string> input_files)
{
    struct BinarySymbol
    {
        unsigned nameLength;
        word value;
        unsigned section;
        bool global;
        unsigned id;
    };
    struct SectionBinary
    {
        unsigned nameLength;
        unsigned id;
        unsigned codeLength;
        char access_rights;
        unsigned num_relocations;
    };
    struct RecordBinary
    {
        unsigned symbol;
        unsigned offset;
        RelocationTable::RelocationType type;
        bool plus;
    };

    SectionBinary sb;
    BinarySymbol symb;
    RecordBinary rb;
    byte b;
    for (string input_file : input_files)
    {
        ifstream input("../test/" + input_file, ios::binary | ios::in);

        if (!input.is_open())
            throw SyntaxError("File " + input_file + " does not exist");

        unsigned num_symbols;
        unsigned num_sections;

        input.read((char *)&num_sections, sizeof(num_sections));
        input.read((char *)&num_symbols, sizeof(num_symbols));

        unordered_map<unsigned, string> sectionId;
        unordered_map<unsigned, unsigned> changedIds;
        forward_list<RelocationTable::Record *> relocations;
        unsigned offset;
        while (num_sections-- > 0)
        {
            input.read((char *)&sb, sizeof(sb));
            char *name = new char[sb.nameLength + 1];
            name[sb.nameLength] = '\0';
            input.read(name, sb.nameLength * sizeof(name[0]));
            Section *s = new Section(name, sb.id, sb.access_rights);
            delete name;

            s = sections.addSection(s);
            sectionId[s->id] = s->name;
            changedIds[sb.id] = s->id;
            offset = s->bytes.size();
            for (unsigned i = 0; i < sb.codeLength; ++i)
                s->bytes.push_back(b);
            input.read((char *)&s->bytes[s->bytes.size() - sb.codeLength], sb.codeLength * sizeof(s->bytes[0]));

            while (sb.num_relocations-- > 0)
            {
                input.read((char *)&rb, sizeof(rb));
                relocations.push_front(s->relocationTable.add(rb.symbol, rb.offset, rb.type, rb.plus));
            }
        }
        while (num_symbols-- > 0)
        {
            input.read((char *)&symb, sizeof(symb));
            char *name = new char[symb.nameLength + 1];
            name[symb.nameLength] = '\0';
            input.read(name, symb.nameLength * sizeof(name[0]));
            auto secId = sectionId.find(changedIds[symb.section]);
            unsigned id = symbols.addSymbol(name, symb.value, secId == sectionId.end() ? nullptr : sections.findSection(secId->second), symb.global);
            changedIds[symb.id] = id;
            delete name;
        }

        for (RelocationTable::Record *r : relocations)
        {
            r->symbol = changedIds[r->symbol];
            r->offset += offset;
            if (r->type == RelocationTable::R_X86_64_PC16 && offset)
            {
                word val = 0;
                Section *sec = symbols.getSymbol(r->symbol)->section;
                val = sec->bytes[r->offset] | sec->bytes[r->offset] << 8;
                val -= offset;
                sec->bytes[r->offset] = val & 0xff;
                sec->bytes[r->offset] = val & 0xff00;
            }
        }
        input.close();
    }

    for (auto s : sections.sections)
    {
        Section *sec = s.second;
        for (RelocationTable::Record &r : sec->relocationTable.records)
        {
            SymbolTable::Symbol *symbol = symbols.getSymbol(r.symbol);
            if (symbol && symbol->section)
            {
                word number = 0;
                switch (r.type)
                {
                case RelocationTable::R_X86_64_8:
                    number = sec->bytes[r.offset];
                    number += symbol->value * (r.plus ? 1 : -1);
                    sec->bytes[r.offset] = number & 0xff;
                    break;
                case RelocationTable::R_X86_64_16:
                    number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                    number += symbol->value * (r.plus ? 1 : -1);
                    sec->bytes[r.offset] += number & 0xff;
                    sec->bytes[r.offset + 1] += number & 0xff00;
                    break;
                case RelocationTable::R_X86_64_PC16:
                    number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                    number += symbol->value * (r.plus ? 1 : -1);
                    sec->bytes[r.offset] += number & 0xff;
                    sec->bytes[r.offset + 1] += number & 0xff00;
                    break;
                }
            }
            else
                throw SyntaxError("Undefined symbol: " + (symbol ? symbol->name : "?"));
        }
    }
    ofstream output("../test/test.txt");

    for (auto sec : sections.sections)
    {
        Section &s = *sec.second;
        output << s.name << ':' << endl;
        output << "Length = " << s.bytes.size() << endl;
        for (byte b : s.bytes)
        {
            byte tmp = (b >> 4) & 0x0f;
            if (tmp < 10)
                output << (char)('0' + tmp);
            else
                output << (char)('A' - 10 + tmp);
            tmp = b & 0x0f;
            if (tmp < 10)
                output << (char)('0' + tmp) << ' ';
            else
                output << (char)('A' - 10 + tmp) << ' ';
        }
        output << endl
               << endl;
        output << ".rel" + s.name << endl;
        for (RelocationTable::Record &r : s.relocationTable.records)
        {
            output << r.symbol << ' ' << r.offset << ' ' << r.type << ' ' << r.plus << endl;
        }
        output << endl;
    }
    output.close();
}

void Linker::place(const list<pair<unsigned, string>> &places)
{
    Section *prev = nullptr;
    for (auto p : places)
    {
        Section *s = sections.findSection("." + p.second);
        if (!s)
            throw SyntaxError("Section " + p.second + " does not exist");

        if (p.first < next_free_address)
            throw SyntaxError("Section " + p.second + " is overlapping with section " + (prev ? prev->name : "??"));

        if (p.first + s->bytes.size() > Emulator::MEMORY_CAPACITY)
            throw SyntaxError("Section " + p.second + " cannot fit in memory starting from the address " + Linker::unsigned2str(p.first));

        s->start_address = p.first;
        next_free_address += s->start_address + s->bytes.size();
        prev = s;
    }
}

unordered_map<unsigned, vector<byte>*> Linker::getAllSections()
{
    unordered_map<unsigned, vector<byte>*> ret;

    for (auto s : sections.sections)
    {
        Section *sec = s.second;
        for (RelocationTable::Record &r : sec->relocationTable.records)
        {
            SymbolTable::Symbol *symbol = symbols.getSymbol(r.symbol);
            if (symbol && symbol->section)
            {
                word number = 0;
                switch (r.type)
                {
                case RelocationTable::R_X86_64_8:
                    number = sec->bytes[r.offset];
                    number += symbol->section->start_address * (r.plus ? 1 : -1);
                    sec->bytes[r.offset] = number & 0xff;
                    break;
                case RelocationTable::R_X86_64_16:
                    number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                    number += symbol->section->start_address * (r.plus ? 1 : -1);
                    sec->bytes[r.offset] += number & 0xff;
                    sec->bytes[r.offset + 1] += number & 0xff00;
                    break;
                case RelocationTable::R_X86_64_PC16:
                    number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                    number += symbol->section->start_address * (r.plus ? 1 : -1);
                    sec->bytes[r.offset] += number & 0xff;
                    sec->bytes[r.offset + 1] += number & 0xff00;
                    break;
                }
            }
            else
                throw SyntaxError("Undefined symbol: " + (symbol ? symbol->name : "?"));
        }
        ret[sec->start_address] = &sec->bytes;
    }
    return ret;
}

string Linker::unsigned2str(unsigned number)
{
    string ret = "";

    while (number > 0)
    {
        ret += '0' + number % 10;
        number /= 10;
    }

    return string(ret.rbegin(), ret.rend());
}
