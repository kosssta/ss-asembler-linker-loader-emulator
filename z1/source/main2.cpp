#include <iostream>
#include <fstream>
#include "../header/sectionTable.hpp"
#include "../header/section.hpp"
#include "../header/symbolTable.hpp"
#include "../header/relocationTable.hpp"
using namespace std;
/*
int main() {
    ofstream bin_output("../test/out1.o", ios::binary | ios::out);

    unsigned symbolTableSize = 0;
    unsigned sectionTableSize = 0;

    for (unsigned i = 0; i < 2; ++i)
        bin_output.write((char *)&symbolTableSize, sizeof(symbolTableSize));

    SectionTable sections;
    SymbolTable symbols;

    Section *text = sections.addSection(".text", new Section(".text"));
    Section *data = sections.addSection(".data", new Section(".data"));
    Section *bss = sections.addSection(".bss", new Section(".bss"));

    text->id = 1;
    data->id = 2;
    bss->id = 3;

    text->bytes.push_back(0x87);
    text->bytes.push_back(0xFC);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0x32);
    text->bytes.push_back(0xFC);
    text->bytes.push_back(0xFC);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);

    data->bytes.push_back(0x02);
    data->bytes.push_back(0x03);
    data->bytes.push_back(0x04);
    data->bytes.push_back(0x05);
    data->bytes.push_back(0x06);
    data->bytes.push_back(0x07);
    data->bytes.push_back(0x08);
    data->bytes.push_back(0x09);
    data->bytes.push_back(0x0A);
    data->bytes.push_back(0x0B);

    for(unsigned i = 0; i < 18; ++i)
        bss->bytes.push_back(0);
    
    symbols.insertSymbol(".text", true, 0, text);
    symbols.insertSymbol(".data", true, 0, data);
    symbols.insertSymbol(".bss", true, 0, bss);
    
    symbols.insertSymbol("c", true, 2, data);
    symbols.insertSymbol("a", false, 0xF, text);
    //symbols.insertSymbol("b", true, 0xB, data);
    symbols.insertSymbol("d", false, 0x18, bss);
    symbols.insertExternSymbol("a");
    symbols.insertExternSymbol("d");
    RelocationTable relocations(&symbols);

    relocations.add("c", text, 1, RelocationTable::R_X86_64_PC16);
    relocations.add("d", data, 2, RelocationTable::R_X86_64_16);
    
    sectionTableSize = sections.writeBinary(bin_output, &relocations);
    symbolTableSize = symbols.writeBinary(bin_output);

    bin_output.seekp(0);
    bin_output.write((char *)&sectionTableSize, sizeof(sectionTableSize));
    bin_output.write((char *)&symbolTableSize, sizeof(symbolTableSize));

    bin_output.close();
    return 0;
}
*/
/*
int main() {
    ofstream bin_output("../test/out2.o", ios::binary | ios::out);

    unsigned symbolTableSize = 0;
    unsigned sectionTableSize = 0;

    for (unsigned i = 0; i < 2; ++i)
        bin_output.write((char *)&symbolTableSize, sizeof(symbolTableSize));

    SectionTable sections;
    SymbolTable symbols;

    Section *text = sections.addSection(".text", new Section(".text"));
    Section *data = sections.addSection(".data", new Section(".data"));
    Section *bss = sections.addSection(".bss", new Section(".bss"));

    text->id = 1;
    data->id = 2;
    bss->id = 3;

    text->bytes.push_back(0x5A);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFE);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);
    text->bytes.push_back(0xFF);

    data->bytes.push_back(0x11);
    data->bytes.push_back(0x12);
    data->bytes.push_back(0x13);
    data->bytes.push_back(0x14);
    data->bytes.push_back(0x15);
    data->bytes.push_back(0x16);
    data->bytes.push_back(0x17);

    for(unsigned i = 0; i < 0x16; ++i)
        bss->bytes.push_back(0);
    
    symbols.insertSymbol(".text", true, 0, text);
    symbols.insertSymbol(".data", true, 0, data);
    symbols.insertSymbol(".bss", true, 0, bss);
    
    symbols.insertSymbol("a", true, 4, text);
    symbols.insertSymbol("b", true, 1, data);
    symbols.insertSymbol("d", false, 6, bss);
    symbols.insertSymbol("c", false, 2, data);
    symbols.insertExternSymbol("c");
    RelocationTable relocations(&symbols);

    relocations.add(".data", text, 4, RelocationTable::R_X86_64_PC16);
    relocations.add("c", data, 3, RelocationTable::R_X86_64_PC16);
    
    sectionTableSize = sections.writeBinary(bin_output, &relocations);
    symbolTableSize = symbols.writeBinary(bin_output);

    bin_output.seekp(0);
    bin_output.write((char *)&sectionTableSize, sizeof(sectionTableSize));
    bin_output.write((char *)&symbolTableSize, sizeof(symbolTableSize));

    bin_output.close();
    return 0;
}
*/