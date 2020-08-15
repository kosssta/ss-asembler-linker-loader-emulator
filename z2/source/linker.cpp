#include "../header/linker.hpp"
#include "../header/syntaxErrors.hpp"
#include "../header/section.hpp"
#include "../header/relocationTable.hpp"
#include "../header/emulator.hpp"
#include <iostream>
#include <fstream>
using namespace std;

const pair<string, unsigned> Linker::interruptSectionNames[] = {{".int.init", 0}, {".int.invadd", 1}, {".int.timer", 2}, {".int.term", 3}};

Linker::Linker()
{
    symbols.addSymbol("data_out", 0xFF00, nullptr, true, true);
    symbols.addSymbol("data_in", 0xFF02, nullptr, true, true);
    symbols.addSymbol("timer_cfg", 0xFF10, nullptr, true, true);
}

void Linker::link(list<string> input_files)
{
    struct BinarySymbol
    {
        unsigned nameLength;
        word value;
        unsigned section;
        bool defined;
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
            sectionId[sb.id] = s->name;
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
            auto secId = sectionId.find(symb.section);
            unsigned id = symbols.addSymbol(name, symb.value + offset, secId == sectionId.end() ? nullptr : sections.findSection(secId->second), symb.defined, symb.global);
            changedIds[symb.id] = id;
            delete name;
        }

        for (auto s : symbols.symbols)
        {
            SymbolTable::Symbol *symb = s.second;
            if (symb->name[0] == '.')
                symb->section->id = symb->id;
        }

        for (RelocationTable::Record *r : relocations)
        {
            r->symbol = changedIds[r->symbol];
            r->offset += offset;
        }
        input.close();
    }

    symbols.checkUndefinedSymbols();

    for (auto s : sections.sections)
    {
        Section *sec = s.second;
        for (RelocationTable::Record &r : sec->relocationTable.records)
        {
            SymbolTable::Symbol *symbol = symbols.getSymbol(r.symbol);

            word number = 0;
            switch (r.type)
            {
            case RelocationTable::R_X86_64_8:
                number = sec->bytes[r.offset];
                number += symbol->value * (r.plus ? 1 : -1);
                sec->bytes[r.offset] = number & 0xff;
                if (!symbols.isSection(symbol->name) && symbol->section)
                    r.symbol = symbol->section->id;
                break;
            case RelocationTable::R_X86_64_16:
                number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                number += symbol->value * (r.plus ? 1 : -1);
                sec->bytes[r.offset] = number & 0xff;
                sec->bytes[r.offset + 1] = number >> 8 & 0xff;
                if (!symbols.isSection(symbol->name) && symbol->section)
                    r.symbol = symbol->section->id;
                break;
            case RelocationTable::R_X86_64_PC16:
                number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                number += symbol->value - r.offset;
                sec->bytes[r.offset] = number & 0xff;
                sec->bytes[r.offset + 1] = number >> 8 & 0xff;
                if (symbol->section->id == sec->id)
                    r.symbol = 0;
                else if (!symbols.isSection(symbol->name) && symbol->section)
                    r.symbol = symbol->section->id;
                break;
            }
        }
        sec->relocationTable.records.remove_if(sec->relocationTable);
    }
}

void Linker::prepareIVT()
{
    for (pair<string, unsigned> p : interruptSectionNames)
    {
        string s = p.first;
        Section *sec = sections.findSection(s);
        if (sec && sections.findSection(".int." + p.second))
            throw SyntaxError("Sections .int." + Linker::unsigned2str(p.second) + " and '" + p.first + "' cannot be both defined in the same program");
        if (!sec)
        {
            sec = sections.addSection(new Section(s));
            unsigned id = symbols.addSymbol(s, 0, sec, true, false);
            sec->id = id;
            SymbolTable::Symbol *symb = symbols.getSymbol(s);
            if (s == ".int.invadd")
                sec->bytes.push_back(0); // halt
            else if (s == ".int.init")
            {
                sec->bytes.push_back(0b01100100); // mov $0, FF10
                sec->bytes.push_back(0);
                sec->bytes.push_back(0);
                sec->bytes.push_back(0);
                sec->bytes.push_back(0x80);
                sec->bytes.push_back(0x10);
                sec->bytes.push_back(0xff);
                sec->bytes.push_back(2 << 3); // ret
            }
            else
                sec->bytes.push_back(1); // iret
        }
    }

    Section *sec = sections.findSection(".iv_table");
    if (!sec)
    {
        sec = sections.addSection(new Section(".iv_table"));
        unsigned id = symbols.addSymbol(".iv_table", 0, sec, true, false);
        sec->id = id;
        SymbolTable::Symbol *symb = symbols.getSymbol(".iv_table");
        for (unsigned i = 0; i < Emulator::IV_TABLE_NUM_ENTRIES * Emulator::IV_TABLE_ENTRY_SIZE; ++i)
            sec->bytes.push_back(0);
    }
}

void Linker::place(const list<pair<unsigned, string>> &places)
{
    prepareIVT();
    Section *iv_table = sections.findSection(".iv_table");
    iv_table->start_address = 0;
    next_free_address = Emulator::IV_TABLE_NUM_ENTRIES * Emulator::IV_TABLE_ENTRY_SIZE;

    Section *prev = iv_table;
    for (auto p : places)
    {
        Section *s = sections.findSection("." + p.second);
        if (!s)
            throw SyntaxError("Section " + p.second + " does not exist");

        if (s->start_address != -1)
        {
            if (s->start_address != p.first)
                throw SyntaxError("Error: Found more -place attributes for section " + s->name);
            else
                continue;
        }

        if (p.first < next_free_address)
            throw SyntaxError("Section ." + p.second + " is overlapping with section " + (prev ? prev->name : "??"));

        if (p.first >= Emulator::MEMORY_MAPPED_REGISTERS_START_ADDRESS && p.first < Emulator::MEMORY_MAPPED_REGISTERS_START_ADDRESS + Emulator::MEMORY_MAPPED_REGISTERS_SIZE || p.first + s->bytes.size() >= Emulator::MEMORY_MAPPED_REGISTERS_START_ADDRESS && p.first + s->bytes.size() < Emulator::MEMORY_MAPPED_REGISTERS_START_ADDRESS + Emulator::MEMORY_MAPPED_REGISTERS_SIZE)
            throw SyntaxError("Section ." + p.second + " is overlapping with memory mapped registers");

        if (p.first + s->bytes.size() > Emulator::MEMORY_CAPACITY)
            throw SyntaxError("Section ." + p.second + " cannot fit in memory starting from the address " + Linker::unsigned2str(p.first));

        s->start_address = p.first;
        next_free_address = s->start_address + s->bytes.size();
        prev = s;
    }

    for (auto s : sections.sections)
    {
        Section *sec = s.second;
        if (sec->start_address == -1)
        {
            sec->start_address = next_free_address;
            next_free_address += sec->bytes.size();
            if (next_free_address > Emulator::MEMORY_CAPACITY)
                throw SyntaxError("Section " + sec->name + " cannot fit in memory starting from address " + Linker::unsigned2str(sec->start_address));
        }

        bool found = false;
        for (pair<string, unsigned> p : interruptSectionNames)
            if (sec->name == p.first)
            {
                for (unsigned i = 0; i < Emulator::IV_TABLE_ENTRY_SIZE; ++i)
                    iv_table->bytes[p.second * Emulator::IV_TABLE_ENTRY_SIZE + i] = sec->start_address >> 8 * i & 0xff;
                found = true;
                break;
            }

        if (!found && sec->name.length() > 5 && sec->name.substr(0, 5) == ".int.")
        {
            string entry = sec->name.substr(5);
            if (entry > unsigned2str(8))
                throw SyntaxError("IVT entry " + entry + " does not exist");
            unsigned numb = 0;
            for (unsigned i = 0; i < entry.length(); ++i)
                if (entry[i] < 0 || entry[i] > 9)
                    throw SyntaxError("IVT entry '" + entry + "' does not exist");
                else
                {
                    numb *= 10;
                    numb += entry[i] - '0';
                }
            for (unsigned i = 0; i < Emulator::IV_TABLE_ENTRY_SIZE; ++i)
                iv_table->bytes[numb * Emulator::IV_TABLE_ENTRY_SIZE + i] = sec->start_address >> 8 * i & 0xff;
        }
    }
}

unordered_map<unsigned, vector<byte> *> Linker::getAllSections()
{
    unordered_map<unsigned, vector<byte> *> ret;

    for (auto s : sections.sections)
    {
        Section *sec = s.second;
        for (RelocationTable::Record &r : sec->relocationTable.records)
        {
            SymbolTable::Symbol *symbol = symbols.getSymbol(r.symbol);
            if (!symbol->section)
                continue;
            word number = 0;
            switch (r.type)
            {
            case RelocationTable::R_X86_64_8:
                number = sec->bytes[r.offset];
                number += symbol->section->start_address;
                sec->bytes[r.offset] = number & 0xff;
                break;
            case RelocationTable::R_X86_64_16:
                number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                number += symbol->section->start_address;
                sec->bytes[r.offset] = number & 0xff;
                sec->bytes[r.offset + 1] = number >> 8 & 0xff;
                break;
            case RelocationTable::R_X86_64_PC16:
                number = sec->bytes[r.offset] & 0xff | sec->bytes[r.offset + 1] << 8;
                number += symbol->section->start_address - sec->start_address;
                sec->bytes[r.offset] = number & 0xff;
                sec->bytes[r.offset + 1] = number >> 8 & 0xff;
                break;
            }
        }
        ret[sec->start_address] = &sec->bytes;
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
            output << r.offset << ' ' << (r.plus ? '+' : '-') << ' ' << r.type << ' ' << r.symbol << endl;
        }
        output << endl;
    }
    output.close();
    return ret;
}

unsigned Linker::getMain() const
{
    SymbolTable::Symbol *main = symbols.getSymbol("main");
    if (!main)
        throw SyntaxError("Symbol main not found");
    return main->value + main->section->start_address;
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
