#ifndef LINKER_H_
#define LINKER_H_

#include "../header/symbolTable.hpp"
#include "../header/sectionTable.hpp"
#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
using namespace std;

typedef int8_t byte;
typedef int16_t word;

class Linker
{
public:
    static const pair<string, unsigned> interruptSectionNames[];

    Linker();
    void link(list<string> input_files);
    void place(const list<pair<unsigned, string>> &places);
    unordered_map<unsigned, vector<byte>*> getAllSections();
    void prepareIVT();
    unsigned getMain() const;

    static string unsigned2str(unsigned number);

private:
    SymbolTable symbols;
    SectionTable sections;
    unsigned next_free_address = 0;
};

#endif
