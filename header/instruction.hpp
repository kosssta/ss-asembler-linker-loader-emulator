#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <string>
#include <unordered_map>
using namespace std;

struct InstructionDetails
{
    unsigned operation_code;
    char num_operands;
    char operand_size;

    InstructionDetails(unsigned oc, char op = 0, char sz = -1) : operation_code(oc), num_operands(op), operand_size(sz) {}
};

const unordered_map<string, InstructionDetails *> INSTRUCTIONS = {
    {"halt", new InstructionDetails(0)},
    {"iret", new InstructionDetails(1)},
    {"ret", new InstructionDetails(2)},
    {"int", new InstructionDetails(3, 1, 2)},
    {"call", new InstructionDetails(4, 1, 2)},
    {"jmp", new InstructionDetails(5, 1, 2)},
    {"jeq", new InstructionDetails(6, 1, 2)},
    {"jne", new InstructionDetails(7, 1, 2)},
    {"jgt", new InstructionDetails(8, 1, 2)},
    {"push", new InstructionDetails(9, 1, 2)},
    {"pop", new InstructionDetails(10, 1, 2)},
    {"xchg", new InstructionDetails(11, 2)},
    {"xchgw", new InstructionDetails(11, 2, 2)},
    {"xchgb", new InstructionDetails(11, 2, 1)},
    {"mov", new InstructionDetails(12, 2)},
    {"movw", new InstructionDetails(12, 2, 2)},
    {"movb", new InstructionDetails{12, 2, 1}},
    {"add", new InstructionDetails(13, 2)},
    {"addw", new InstructionDetails(13, 2, 2)},
    {"addb", new InstructionDetails(13, 2, 1)},
    {"sub", new InstructionDetails(14, 2)},
    {"subw", new InstructionDetails(14, 2, 2)},
    {"subb", new InstructionDetails(14, 2, 1)},
    {"mul", new InstructionDetails(15, 2)},
    {"mulw", new InstructionDetails(15, 2, 2)},
    {"mulb", new InstructionDetails(15, 2, 1)},
    {"div", new InstructionDetails(16, 2)},
    {"divw", new InstructionDetails(16, 2, 2)},
    {"divb", new InstructionDetails(16, 2, 1)},
    {"cmp", new InstructionDetails(17, 2)},
    {"cmpw", new InstructionDetails(17, 2, 2)},
    {"cmpb", new InstructionDetails(17, 2, 1)},
    {"not", new InstructionDetails(18, 2)},
    {"notw", new InstructionDetails(18, 2, 2)},
    {"notb", new InstructionDetails(18, 2, 1)},
    {"and", new InstructionDetails(19, 2)},
    {"andw", new InstructionDetails(19, 2, 2)},
    {"andb", new InstructionDetails(19, 2, 1)},
    {"or", new InstructionDetails(20, 2)},
    {"orw", new InstructionDetails(20, 2, 2)},
    {"orb", new InstructionDetails(20, 2, 1)},
    {"xor", new InstructionDetails(21, 2)},
    {"xorw", new InstructionDetails(21, 2, 2)},
    {"xorb", new InstructionDetails(21, 2, 1)},
    {"test", new InstructionDetails(22, 2)},
    {"testw", new InstructionDetails(22, 2, 2)},
    {"testb", new InstructionDetails(22, 2, 1)},
    {"shl", new InstructionDetails(23, 2)},
    {"shlw", new InstructionDetails(23, 2, 2)},
    {"shlb", new InstructionDetails(23, 2, 1)},
    {"shr", new InstructionDetails(24, 2)},
    {"shrw", new InstructionDetails(24, 2, 2)},
    {"shrb", new InstructionDetails(24, 2, 1)}
};

struct Instruction
{
    string label;
    string operation;
    string op1;
    string op2;
    bool isDirective;

    Instruction(string label, string operation, string op1, string op2, bool isDirective = false)
    {
        this->label = label;
        this->operation = operation;
        this->op1 = op1;
        this->op2 = op2;
        this->isDirective = isDirective;
    }
};

#endif
