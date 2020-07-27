#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <string>
#include "symbolTable.hpp"
#include "sectionTable.hpp"
#include "uncalculatedSymbolsTable.hpp"
using namespace std;

typedef int16_t word;
typedef int8_t byte;

struct Instruction;
struct Section;

class Assembler
{
public:
    Assembler() : uncalculatedSymbols(&symbolTable) {}

    void assembly(string input_file, string output_file);
    static bool isLiteral(string arg);
    static word parseInt(string arg);
    static word parseOperand(string operand, Section *section, SymbolTable *SymbolTable, bool lowerByteOnly = false);

private:
    SymbolTable symbolTable;
    SectionTable sectionTable;
    UncalculatedSymbolsTable uncalculatedSymbols;
    Section *current_section = nullptr;
    bool end = false;

    void processDirective(Instruction *instr);
    void processCommand(Instruction *instr);
    void processLabel(string label);
    unsigned parseArgs(string args, word *values);
    void regDir(string operand, byte op_code = 1);
    void literalSimbol(string operand);
};

#endif
