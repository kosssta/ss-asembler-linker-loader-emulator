#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include "../header/symbolTable.hpp"
#include "../header/sectionTable.hpp"
#include "../header/uncalculatedSymbolsTable.hpp"
#include "../header/relocationTable.hpp"
#include <string>
#include <list>
using namespace std;

typedef int16_t word;
typedef int8_t byte;

struct Instruction;
struct Section;

class Assembler
{
public:
    Assembler() : uncalculatedSymbols(&symbolTable, &relocationTable), relocationTable(&symbolTable) {}
    void assemble(string input_file, string output_file);
    
    static bool isLiteral(string arg);
    static word parseInt(string arg);
    static word parseOperand(string operand, Section *section, SymbolTable *SymbolTable, bool lowerByteOnly = false);
    static list<string> splitString(string str, string regex);

private:
    SymbolTable symbolTable;
    SectionTable sectionTable;
    UncalculatedSymbolsTable uncalculatedSymbols;
    RelocationTable relocationTable;
    Section *current_section = nullptr;
    bool end = false;
    unsigned num_errors = 0;

    void processDirective(Instruction *instr);
    void processCommand(Instruction *instr);
    void processLabel(string label);
    unsigned processRegister(string operand, byte op_code = 1);
    void processLiteralOrSymbol(string operand, unsigned size, byte op_code = 0, byte pc_rel_off = -1);
};

#endif
