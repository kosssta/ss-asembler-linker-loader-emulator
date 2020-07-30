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
    bool jump;

    InstructionDetails(unsigned oc, char op = 0, char sz = -1, bool jmp = false) : operation_code(oc), num_operands(op), operand_size(sz), jump(jmp) {}
};

const unordered_map<string, InstructionDetails> INSTRUCTIONS = {
    {"halt", InstructionDetails(0)},
    {"iret", InstructionDetails(1)},
    {"ret", InstructionDetails(2)},
    {"int", InstructionDetails(3, 1, 2, true)},
    {"call", InstructionDetails(4, 1, 2, true)},
    {"jmp", InstructionDetails(5, 1, 2, true)},
    {"jeq", InstructionDetails(6, 1, 2, true)},
    {"jne", InstructionDetails(7, 1, 2, true)},
    {"jgt", InstructionDetails(8, 1, 2, true)},
    {"push", InstructionDetails(9, 1, 2)},
    {"pop", InstructionDetails(10, 1, 2)},
    {"xchg", InstructionDetails(11, 2)},
    {"xchgw", InstructionDetails(11, 2, 2)},
    {"xchgb", InstructionDetails(11, 2, 1)},
    {"mov", InstructionDetails(12, 2)},
    {"movw", InstructionDetails(12, 2, 2)},
    {"movb", InstructionDetails{12, 2, 1}},
    {"add", InstructionDetails(13, 2)},
    {"addw", InstructionDetails(13, 2, 2)},
    {"addb", InstructionDetails(13, 2, 1)},
    {"sub", InstructionDetails(14, 2)},
    {"subw", InstructionDetails(14, 2, 2)},
    {"subb", InstructionDetails(14, 2, 1)},
    {"mul", InstructionDetails(15, 2)},
    {"mulw", InstructionDetails(15, 2, 2)},
    {"mulb", InstructionDetails(15, 2, 1)},
    {"div", InstructionDetails(16, 2)},
    {"divw", InstructionDetails(16, 2, 2)},
    {"divb", InstructionDetails(16, 2, 1)},
    {"cmp", InstructionDetails(17, 2)},
    {"cmpw", InstructionDetails(17, 2, 2)},
    {"cmpb", InstructionDetails(17, 2, 1)},
    {"not", InstructionDetails(18, 2)},
    {"notw", InstructionDetails(18, 2, 2)},
    {"notb", InstructionDetails(18, 2, 1)},
    {"and", InstructionDetails(19, 2)},
    {"andw", InstructionDetails(19, 2, 2)},
    {"andb", InstructionDetails(19, 2, 1)},
    {"or", InstructionDetails(20, 2)},
    {"orw", InstructionDetails(20, 2, 2)},
    {"orb", InstructionDetails(20, 2, 1)},
    {"xor", InstructionDetails(21, 2)},
    {"xorw", InstructionDetails(21, 2, 2)},
    {"xorb", InstructionDetails(21, 2, 1)},
    {"test", InstructionDetails(22, 2)},
    {"testw", InstructionDetails(22, 2, 2)},
    {"testb", InstructionDetails(22, 2, 1)},
    {"shl", InstructionDetails(23, 2)},
    {"shlw", InstructionDetails(23, 2, 2)},
    {"shlb", InstructionDetails(23, 2, 1)},
    {"shr", InstructionDetails(24, 2)},
    {"shrw", InstructionDetails(24, 2, 2)},
    {"shrb", InstructionDetails(24, 2, 1)}};

struct RegisterDetails
{
    char code;
    char size;
    bool high;

    RegisterDetails(char cd, char sz = 2, bool hgh = false) : code(cd), size(sz), high(hgh) {}
};

const unordered_map<string, RegisterDetails> REGISTERS = {
    {"r0", RegisterDetails(0)},
    {"r1", RegisterDetails(1)},
    {"r2", RegisterDetails(2)},
    {"r3", RegisterDetails(3)},
    {"r4", RegisterDetails(4)},
    {"r5", RegisterDetails(5)},
    {"r6", RegisterDetails(6)},
    {"r7", RegisterDetails(7)},
    {"r0h", RegisterDetails(0, 1, true)},
    {"r1h", RegisterDetails(1, 1, true)},
    {"r2h", RegisterDetails(2, 1, true)},
    {"r3h", RegisterDetails(3, 1, true)},
    {"r4h", RegisterDetails(4, 1, true)},
    {"r5h", RegisterDetails(5, 1, true)},
    {"r6h", RegisterDetails(6, 1, true)},
    {"r7h", RegisterDetails(7, 1, true)},
    {"r0l", RegisterDetails(0, 1)},
    {"r1l", RegisterDetails(1, 1)},
    {"r2l", RegisterDetails(2, 1)},
    {"r3l", RegisterDetails(3, 1)},
    {"r4l", RegisterDetails(4, 1)},
    {"r5l", RegisterDetails(5, 1)},
    {"r6l", RegisterDetails(6, 1)},
    {"r7l", RegisterDetails(7, 1)},
    {"pc", RegisterDetails(7)},
    {"sp", RegisterDetails(6)},
    {"psw", RegisterDetails(15)}
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
