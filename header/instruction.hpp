#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <string>
using namespace std;

struct Instruction {
    string label;
    string operation;
    string op1;
    string op2;
    bool isDirective;

    Instruction(string label, string operation, string op1, string op2, bool isDirective = false) {
        this->label = label;
        this->operation = operation;
        this->op1 = op1;
        this->op2 = op2;
        this->isDirective = isDirective;
    }
};

#endif
