#ifndef LINKER_H_
#define LINKER_H_

#include "../header/symbolTable.hpp"
#include "../header/sectionTable.hpp"
#include <list>
#include <string>
using namespace std;

typedef int8_t byte;
typedef int16_t word;

class Linker
{
public:
    void link(list<string> input_files);

    private:
    SymbolTable symbols;
    SectionTable sections;
};

#endif
