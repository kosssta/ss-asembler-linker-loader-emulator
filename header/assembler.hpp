#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <string>
#include "symbolTable.hpp"
using namespace std;

struct Instruction;
struct Section;

class Assembler
{
public:
    void assembly(string input_file, string output_file);

private:
    unsigned location_counter = 0;
    SymbolTable symbolTable;
    Section* current_section = nullptr;

    void processDirective(Instruction *instr);
    void processCommand(Instruction *instr);
    void processLabel(string label);
};

#endif
