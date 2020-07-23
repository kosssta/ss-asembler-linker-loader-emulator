#include "assembler.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "instruction.hpp"
#include "lineParser.hpp"
#include "syntaxError.hpp"
#include "section.hpp"
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
                processLabel(instr->label);
                if (instr->isDirective)
                    processDirective(instr);
                else
                    processCommand(instr);
            }
            delete instr;
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

void Assembler::processCommand(Instruction *instr)
{
    if (instr->operation == "")
        return;
    if (current_section == nullptr)
        throw SyntaxError("Every command needs to be in a section");

    unordered_map<string, InstructionDetails *>::const_iterator name = INSTRUCTIONS.find(instr->operation);
    if (name == INSTRUCTIONS.end())
        throw SyntaxError("Unrecognized token " + instr->operation);
}

void Assembler::processDirective(Instruction *instr)
{
    if (instr->operation == "")
        return;
    if (instr->operation == "section" || instr->operation == "text" || instr->operation == "data" || instr->operation == "bss" || instr->operation == "rodata")
    {
        current_section = new Section();
        cout << "Current section: " << (instr->op1 != "" ? instr->op1 : instr->operation) << endl;
    }
    else if (instr->operation == "end")
        current_section = nullptr;
}

void Assembler::processLabel(string label)
{
    if (label == "")
        return;

    symbolTable.insertSymbol(label, location_counter, current_section);
}
