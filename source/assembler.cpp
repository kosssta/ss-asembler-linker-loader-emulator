#include "assembler.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "instruction.hpp"
#include "lineParser.hpp"
using namespace std;

void Assembler::assembly(string input_file, string output_file)
{
    ifstream input("../files/" + input_file);
    string line = "";
    unsigned line_number = 1;

    while (getline(input, line))
    {
        Instruction *instr = LineParser::parse(line);

        cout << line_number++ << ':' << endl;
        if (!instr)
            cout << "Prazna naredba ili greska";
        else
        {
            cout << '[' << instr->label << ']' << endl;
            cout << '[' << instr->operation << ']' << endl;
            cout << '[' << instr->op1 << ']' << endl;
            cout << '[' << instr->op2 << ']' << endl;
            cout << '[' << (instr->isDirective ? "true" : "false") << ']' << endl;
        }
    }

    input.close();
}
