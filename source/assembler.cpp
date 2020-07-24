#include "assembler.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "instruction.hpp"
#include "lineParser.hpp"
#include "syntaxError.hpp"
#include "section.hpp"
#include <regex>
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

    sectionTable.write();
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

    InstructionDetails *instruction = name->second;

    byte op_code = instruction->operation_code << 3;

    if (instruction->num_operands == 0)
    {
        current_section->bytes.add(op_code);
        ++current_section->location_counter;
        return;
    }

    if (instruction->operand_size == 2)
        op_code |= 1 << 2;
    else if (instruction->operand_size == -1)
    {
        op_code |= 1 << 2;
    }

    current_section->bytes.add(op_code);
    ++current_section->location_counter;

    if (instruction->jump)
    {
        smatch sm;
        const string operand_regex = "(?:([^\\(\\)]+)|([^\\(]+)\\(([^\\)]+)\\))";
        if (!regex_match(instr->op1, sm, regex(operand_regex)))
            throw SyntaxError("Error - not matched");

        if (sm[1] != "")
        {
            string operand = sm[1];
            if (operand[0] == '*')
            {
                // *%r<num>
                if (operand[1] == '%')
                {
                    operand.erase(0, 2);
                    unordered_map<string, RegisterDetails *>::const_iterator reg = REGISTERS.find(operand);
                    if (reg == REGISTERS.end())
                        throw SyntaxError("Unrecognized token: " + operand);

                    RegisterDetails *r = reg->second;

                    if (r->size == 1)
                        current_section->bytes.set(current_section->location_counter - 1, current_section->bytes.get(current_section->location_counter - 1) & ~(1 << 2));
                    current_section->bytes.add(1 << 5 | r->code << 1 | (r->size == 1 && r->high ? 1 : 0));
                    ++current_section->location_counter;
                }
                // *<literal> ili *<simbol>
                else
                {
                    operand.erase(0);
                    current_section->bytes.add(4 << 5);
                    ++current_section->location_counter;

                    word number;
                    if (isLiteral(operand))
                        number = parseInt(operand);
                    else
                    {
                        SymbolTable::Symbol *symb = symbolTable.getSymbol(operand);
                        if (symb)
                            number = symb->value;
                        else
                        {
                            number = 0;
                            symbolTable.insertSymbol(operand, false);
                            symb = symbolTable.getSymbol(operand);
                            symb->defined = false;
                            symb->addFLink(current_section ? current_section : sectionTable.global, current_section->location_counter, 2);
                        }
                    }
                    byte array[] = {(byte)(number & 0xff), (byte)((number >> 8) & 0xff)};
                    current_section->bytes.add(array, 2);
                    current_section->location_counter += 2;
                }
            }
            // <literal> ili <simbol>
            else
            {
                current_section->bytes.add(0 << 5);
                ++current_section->location_counter;

                word number;
                if (isLiteral(operand))
                    number = parseInt(operand);
                else
                {
                    SymbolTable::Symbol *symb = symbolTable.getSymbol(operand);
                    if (symb && symb->defined)
                        number = symb->value;
                    else
                    {
                        number = 0;
                        if (!symb)
                        {
                            symbolTable.insertSymbol(operand, false);
                            symb = symbolTable.getSymbol(operand);
                        }
                        symb->addFLink(current_section ? current_section : sectionTable.global, current_section->location_counter, 2);
                    }
                }
                byte array[] = {(byte)(number & 0xff), (byte)((number >> 8) & 0xff)};
                current_section->bytes.add(array, 2);
                current_section->location_counter += 2;
            }
        }
        // ()
        else
        {
            string operand = sm[2];
            if (operand[0] != '*')
                throw SyntaxError("Invalid syntax: " + operand);

            operand.erase(0, 1);

            // *(%r<num>)
            if (operand == "")
            {
                operand = sm[3];
                if (operand[0] != '%')
                {
                    string tmp = sm[2];
                    throw SyntaxError("Invalid syntax: " + tmp + operand);
                }

                operand.erase(0, 1);
                unordered_map<string, RegisterDetails *>::const_iterator reg = REGISTERS.find(operand);
                if (reg == REGISTERS.end())
                    throw SyntaxError("Unrecognized token: " + operand);

                RegisterDetails *r = reg->second;

                if (r->size == 1)
                    current_section->bytes.set(current_section->location_counter - 1, current_section->bytes.get(current_section->location_counter - 1) & ~(1 << 2));
                current_section->bytes.add(2 << 5 | r->code << 1 | (r->size == 1 && r->high ? 1 : 0));
                ++current_section->location_counter;
            }
            // *<literal>(%r<num>) ili *<simbol>(%r<num>) ili *<simbol>(%pc/%r7)
            else
            {
                operand = sm[3];
                if (operand[0] != '%')
                {
                    string tmp = sm[2];
                    throw SyntaxError("Invalid syntax: " + tmp + operand);
                }

                operand.erase(0, 1);
                unordered_map<string, RegisterDetails *>::const_iterator reg = REGISTERS.find(operand);
                if (reg == REGISTERS.end())
                    throw SyntaxError("Unrecognized token: " + operand);

                RegisterDetails *r = reg->second;

                if (r->size == 1)
                    current_section->bytes.set(current_section->location_counter - 1, current_section->bytes.get(current_section->location_counter - 1) & ~(1 << 2));
                current_section->bytes.add(3 << 5 | r->code << 1 | (r->size == 1 && r->high ? 1 : 0));
                ++current_section->location_counter;

                operand = sm[2];
                operand.erase(0, 1);

                word number;
                if (isLiteral(operand))
                    number = parseInt(operand);
                else
                {
                    SymbolTable::Symbol *symb = symbolTable.getSymbol(operand);
                    if (symb)
                        number = symb->value;
                    else
                    {
                        number = 0;
                        symbolTable.insertSymbol(operand, false);
                        symb = symbolTable.getSymbol(operand);
                        symb->defined = false;
                        symb->addFLink(current_section ? current_section : sectionTable.global, current_section->location_counter, 2);
                    }
                }
                byte array[] = {(byte)(number & 0xff), (byte)((number >> 8) & 0xff)};
                current_section->bytes.add(array, 2);
                current_section->location_counter += 2;
            }
        }

        return;
    }
}

void Assembler::processDirective(Instruction *instr)
{
    if (instr->operation == "")
        return;
    if (instr->operation == "section" || instr->operation == "text" || instr->operation == "data" || instr->operation == "bss" || instr->operation == "rodata")
    {
        current_section = new Section();
        current_section = sectionTable.addSection(instr->op1 != "" ? instr->op1 : instr->operation, current_section);
    }
    else if (instr->operation == "end")
        current_section = nullptr;
}

void Assembler::processLabel(string label)
{
    if (label == "")
        return;

    if (!current_section)
        throw SyntaxError("Every symbol needs to be in a section");

    symbolTable.insertSymbol(label, true, current_section->location_counter, current_section);
}

bool Assembler::isLiteral(string arg) const
{
    if (arg == "")
        return false;

    if (arg[0] == '\'' && arg[arg.length() - 1] == '\'' && (arg[1] != '\\' && arg.length() == 3 || arg[1] == '\\' && arg.length() == 4))
        return true;

    if (arg.length() > 2 && arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X'))
    {
        for (unsigned i = 2; i < arg.length(); ++i)
            if (!(arg[i] >= '0' && arg[i] <= '9' || arg[i] >= 'a' && arg[i] <= 'f' || arg[i] >= 'A' && arg[i] <= 'F'))
                return false;

        return true;
    }

    for (unsigned i = 0; i < arg.length(); ++i)
        if (!(arg[i] >= '0' && arg[i] <= '9'))
            return false;

    return true;
}

word Assembler::parseInt(string arg) const
{
    if (arg[0] == '\'' && arg[arg.length() - 1] == '\'')
        if (arg[1] != '\\' && arg.length() == 3)
            return arg[1];
        else if (arg[1] == '\\' && arg.length() == 4)
            return arg[2];

    word number = 0;

    if (arg.length() > 2 && arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X'))
    {
        for (unsigned i = 2; i < arg.length(); ++i)
        {
            number <<= 4;
            if (arg[i] >= '0' && arg[i] <= '9')
                number += arg[i] - '0';
            else if (arg[i] >= 'a' && arg[i] <= 'f')
                number += arg[i] - 'a' + 10;
            else
                number += arg[i] - 'A' + 10;
        }
    }
    else
        for (unsigned i = 0; i < arg.length(); ++i)
        {
            number *= 10;
            number += arg[i] - '0';
        }

    return number;
}
