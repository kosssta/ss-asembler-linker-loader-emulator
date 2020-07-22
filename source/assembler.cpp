#include "assembler.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "instruction.hpp"
#include "lineParser.hpp"
#include "syntaxError.hpp"
using namespace std;

void Assembler::assembly(string input_file, string output_file)
{
    ifstream input("../test/" + input_file);
    string line = "";
    unsigned line_number = 1;

    while (getline(input, line))
    {
        try
        {
            Instruction *instr = LineParser::parse(line);

            if (instr)
            {
                string res = "";
                if (instr->label != "")
                {
                    res += instr->label + ": ";
                }

                if (instr->operation != "")
                {
                    if (instr->isDirective)
                        res += '.';
                    res += instr->operation + " ";
                }

                if (instr->op1 != "")
                {
                    res += instr->op1;
                }

                if (instr->op2 != "")
                {
                    if (instr->op1 != "")
                        res += ", ";
                    if(instr->isDirective) {
                        
                    }
                    res += instr->op2;
                }

                cout << res << endl;
            }
            ++line_number;
        }
        catch (SyntaxError err)
        {
            err.setLineNumber(line_number++);
            cerr << err.getErrorMessage() << endl;
        }
    }

    input.close();
}
