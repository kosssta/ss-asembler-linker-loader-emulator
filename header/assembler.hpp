#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <string>
#include <vector>
#include "symbolTable.hpp"
#include "sectionTable.hpp"
#include "uncalculatedSymbolsTable.hpp"
#include "relocationTable.hpp"
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
    static vector<string> splitString(string str, string regex);

private:
    SymbolTable symbolTable;
    SectionTable sectionTable;
    UncalculatedSymbolsTable uncalculatedSymbols;
    RelocationTable relocationTable;
    Section *current_section = nullptr;
    bool end = false;

    void processDirective(Instruction *instr);
    void processCommand(Instruction *instr);
    void processLabel(string label);
    void regDir(string operand, byte op_code = 1);
    void literalSimbol(string operand, byte op_code = 0, bool pc_rel = false);
};

#endif
