#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <string>
#include "symbolTable.hpp"
#include "sectionTable.hpp"
using namespace std;

typedef int16_t word;

struct Instruction;
struct Section;

class Assembler
{
public:
    void assembly(string input_file, string output_file);

private:
    SymbolTable symbolTable;
    SectionTable sectionTable;
    Section* current_section = nullptr;

    void processDirective(Instruction *instr);
    void processCommand(Instruction *instr);
    void processLabel(string label);
    bool isLiteral(string arg) const;
    word parseInt(string arg) const;
};

#endif
