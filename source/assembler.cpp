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

    while (!end && getline(input, line))
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

    try
    {
        if (!uncalculatedSymbols.calculateAll())
            throw SyntaxError("Uncalculated symbols");
    }
    catch (SyntaxError err)
    {
        cerr << err.getErrorMessage() << endl;
    }

    symbolTable.write();
    cout << endl;
    uncalculatedSymbols.write();
    cout << endl;
    sectionTable.write();
    cout << endl;
    relocationTable.write();
    cout << endl;
}

void Assembler::processCommand(Instruction *instr)
{
    if (instr->operation == "")
        return;
    if (current_section == nullptr)
        throw SyntaxError("Every instruction needs to be in a section");

    const auto &name = INSTRUCTIONS.find(instr->operation);
    if (name == INSTRUCTIONS.end())
        throw SyntaxError("Unrecognized token " + instr->operation);

    InstructionDetails *instructionDetails = name->second;

    byte op_code = instructionDetails->operation_code << 3;

    if (instructionDetails->num_operands == 0)
    {
        if (instr->op1 != "" || instr->op2 != "")
            throw SyntaxError("Error: Instruction should have zero arguments.");

        current_section->bytes.push_back(op_code);
        return;
    }

    if (instructionDetails->num_operands == 1)
    {
        if (instr->op1 == "" || instr->op2 != "")
            throw SyntaxError("Instruction needs 1 argument");
    }
    else
    {
        if (instr->op1 == "" || instr->op2 == "")
            throw SyntaxError("Instruction needs 2 arguments");
    }

    if (instructionDetails->operand_size == 2)
        op_code |= 1 << 2;
    else if (instructionDetails->operand_size == -1)
    {
        op_code |= 1 << 2;
    }

    current_section->bytes.push_back(op_code);

    string operands[2] = {instr->op1, instr->op2};
    for (unsigned i = 0; i < instructionDetails->num_operands; ++i)
    {
        string operand = operands[i];
        if (operand == "")
            continue;

        smatch sm;
        const string operand_regex = "(?:([^\\(\\)]+)|([^\\(]*)\\(([^\\)]+)\\))";
        if (!regex_match(operand, sm, regex(operand_regex)))
            throw SyntaxError("Error - not matched");

        if (sm[1] != "")
        {
            string operand = sm[1];
            if (instructionDetails->jump && operand[0] == '*')
            {
                if (operand[1] == '%')
                    regDir(operand.substr(2));
                else
                    literalSimbol(operand.substr(1), 4);
            }
            else if (!instructionDetails->jump && operand[0] == '$')
                literalSimbol(operand.substr(1)); // $<literal> ili $<simbol>
            else if (!instructionDetails->jump && operand[0] == '%')
                regDir(operand.substr(1)); // %r<num>
            else
                literalSimbol(operand, instructionDetails->jump ? 0 : 4); // <literal> ili <simbol>
        }
        else
        {
            string operand = sm.str(2);
            if (instructionDetails->jump)
                if (operand[0] != '*')
                    throw SyntaxError("Invalid syntax: " + operand);
                else
                    operand.erase(0, 1);

            if (operand == "")
            {
                if (sm.str(3)[0] != '%')
                    throw SyntaxError("Unrecognized symbol: " + sm.str(3));

                regDir(sm.str(3).substr(1), 2); // (%r<num>)
            }
            else
            {
                if (sm.str(3)[0] != '%')
                    throw SyntaxError("Invalid syntax: " + sm.str(2) + sm.str(3));

                // <literal>(%r<num>) ili <simbol>(%r<num>) ili <simbol>(%pc/%r7)
                string reg = sm.str(3).substr(1);
                regDir(reg, 3);
                literalSimbol(operand, -1, reg == "pc" || reg == "r7");
            }
        }
    }
}

void Assembler::processDirective(Instruction *instr)
{
    if (instr->operation == "")
        return;

    if (instr->operation == "section" || instr->operation == "text" || instr->operation == "data" || instr->operation == "bss" || instr->operation == "rodata")
    {
        if (instr->op2 != "" || instr->operation != "section" && instr->op1 != "")
            throw SyntaxError("Syntax error");

        current_section = new Section();
        string name = instr->op1 != "" ? instr->op1.substr(1) : instr->operation;
        Section *ret = sectionTable.addSection(name, current_section);
        if (current_section == ret)
            symbolTable.insertSymbol('.' + name, true, 0, current_section);
        else
            current_section = ret;
    }
    else if (instr->operation == "end")
    {
        if (instr->op1 != "" || instr->op2 != "")
            throw SyntaxError("Syntax error");

        current_section = nullptr;
        end = true;
    }
    else if (instr->operation == "global" || instr->operation == "extern" || instr->operation == "byte" || instr->operation == "word")
    {
        if (instr->op1 == "")
            throw SyntaxError("." + instr->operation + " needs at least 1 operand");

        if (current_section == nullptr && (instr->operation == "byte" || instr->operation == "word"))
            throw SyntaxError("." + instr->operation + " needs to be in a section");

        vector<string> elems = Assembler::splitString(instr->op1 + ',' + instr->op2, "(?:,|[^,\\s]+)");
        unsigned i = 0;

        for (string s : elems)
        {
            if (i & 1)
            {
                if (s[0] != ',')
                    throw SyntaxError();
            }
            else
            {
                if (s[0] == ',')
                    throw SyntaxError();
                else
                {
                    if (instr->operation == "global")
                        symbolTable.setSymbolGlobal(s);
                    else if (instr->operation == "extern")
                        symbolTable.insertExternSymbol(s);
                    else
                    {
                        word number = Assembler::parseOperand(s, current_section, &symbolTable, instr->operation == "byte");
                        if (!isLiteral(s))
                            relocationTable.add(current_section, current_section->bytes.size(), instr->operation == "byte" ? RelocationTable::R_X86_64_8 : RelocationTable::R_X86_64_16);
                        current_section->bytes.push_back(number & 0xff);
                        if (instr->operation == "word")
                            current_section->bytes.push_back(number >> 8 & 0xff);
                    }
                }
            }
            ++i;
        }
    }
    else if (instr->operation == "skip")
    {
        if (instr->op1 == "" && instr->op2 != "")
            throw SyntaxError(".skip needs exactly 1 argument");

        word number = Assembler::parseOperand(instr->op1, current_section, &symbolTable);
        if (number < 0)
            throw SyntaxError("Operand cannot be less than zero.");

        byte *arr = new byte[number];
        for (unsigned i = 0; i < number; ++i)
        {
            current_section->bytes.push_back(arr[i]);
        }
        delete arr;
    }
    else if (instr->operation == "equ")
    {
        if (instr->op1 == "" || instr->op2 == "" || regex_match(instr->op1, regex(".*(\\+|-).*")))
            throw SyntaxError();

        if (Assembler::isLiteral(instr->op1))
            throw SyntaxError();

        uncalculatedSymbols.add(instr->op1, instr->op2, current_section);
    }
    else
        throw SyntaxError("Unrecognized directive: ." + instr->operation);
}

void Assembler::processLabel(string label)
{
    if (label == "")
        return;

    if (!current_section)
        throw SyntaxError("Every symbol needs to be in a section");

    symbolTable.insertSymbol(label, true, current_section->bytes.size(), current_section);
}

bool Assembler::isLiteral(string arg)
{
    return arg != "" && (regex_match(arg, regex("'\\\\?.'")) || regex_match(arg, regex("(?:\\+|-)?[0-9]+")) || regex_match(arg, regex("(?:\\+|-)?(?:0x|0X)[0-9a-fA-F]+")));
}

word Assembler::parseInt(string arg)
{
    if (arg[0] == '\'' && arg[arg.length() - 1] == '\'')
        if (arg[1] != '\\' && arg.length() == 3)
            return arg[1];
        else if (arg[1] == '\\' && arg.length() == 4)
            return arg[2];

    word number = 0;
    bool minus = false;
    if (arg[0] == '-' || arg[0] == '+')
    {
        minus = arg[0] == '-';
        arg.erase(0, 1);
    }

    if (regex_match(arg, regex("(?:0x|0X)[0-9a-fA-F]+")))
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
    else if (regex_match(arg, regex("[0-9]+")))
        for (unsigned i = 0; i < arg.length(); ++i)
        {
            number *= 10;
            number += arg[i] - '0';
        }
    else
        throw SyntaxError(arg + "is not a number");

    return number * (minus ? -1 : 1);
}

word Assembler::parseOperand(string operand, Section *section, SymbolTable *symbolTable, bool lowerByteOnly)
{
    word number;
    if (Assembler::isLiteral(operand))
        number = Assembler::parseInt(operand);
    else
    {
        SymbolTable::Symbol *symb = symbolTable->getSymbol(operand);
        if (symb && symb->defined)
            number = symb->value;
        else
        {
            number = 0;
            if (!symb)
            {
                symbolTable->insertSymbol(operand, false);
                symb = symbolTable->getSymbol(operand);
            }
            symb->addFLink(section, section->bytes.size(), lowerByteOnly ? 1 : 2);
        }
    }

    return number;
}

void Assembler::regDir(string operand, byte op_code)
{
    unordered_map<string, RegisterDetails *>::const_iterator reg = REGISTERS.find(operand);
    if (reg == REGISTERS.end())
        throw SyntaxError("Unrecognized token: " + operand);

    RegisterDetails *r = reg->second;

    if (r->size == 1)
        current_section->bytes[current_section->bytes.size() - 1] &= ~(1 << 2);
    current_section->bytes.push_back(op_code << 5 | r->code << 1 | (r->size == 1 && r->high ? 1 : 0));
}

void Assembler::literalSimbol(string operand, byte op_code, bool pc_rel)
{
    if (op_code != -1)
        current_section->bytes.push_back(op_code << 5);

    word number = Assembler::parseOperand(operand, current_section, &symbolTable);
    if (!Assembler::isLiteral(operand))
        relocationTable.add(current_section, current_section->bytes.size(), pc_rel ? RelocationTable::R_X86_64_PC16 : RelocationTable::R_X86_64_16);
    current_section->bytes.push_back(number & 0xff);
    current_section->bytes.push_back(number >> 8 & 0xff);
}

vector<string> Assembler::splitString(string str, string regex)
{
    vector<string> elems;
    smatch sm;

    while (regex_search(str, sm, std::regex(regex)))
    {
        elems.push_back(sm.str());
        str = sm.suffix().str();
    }

    return elems;
}
