#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <string>
using namespace std;

struct Instruction {
    string label;
    string operation;
    string op1;
    string op2;

    Instruction(string label, string operation, string op1, string op2) {
        this->label = label;
        this->operation = operation;
        this->op1 = op1;
        this->op2 = op2;
    }
};

#endif
