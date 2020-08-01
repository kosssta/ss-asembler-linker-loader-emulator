#include "linker.hpp"
#include "syntaxErrors.hpp"
#include "../header/section.hpp"
#include "../header/relocationTable.hpp"
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
            unsigned id = symbols.addSymbol(name, symb.value, secId == sectionId.end() ? nullptr : sections.findSection(secId->second), symb.global);
            changedIds[symb.id] = id;
            delete name;
        }

        for (RelocationTable::Record *r : relocations)
            r->symbol = changedIds[r->symbol];

        input.close();
    }
}
