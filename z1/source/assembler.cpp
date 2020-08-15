#include "../header/assembler.hpp"
#include "../header/instruction.hpp"
#include "../header/lineParser.hpp"
#include "../header/syntaxErrors.hpp"
#include "../header/section.hpp"
#include <iostream>
#include <fstream>
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
            ++num_errors;
        }
    }

    input.close();

    try
    {
        uncalculatedSymbols.calculateAll();
        relocationTable.replace();
        symbolTable.removeAllLocalSymbols();
    }
    catch (SyntaxError err)
    {
        cerr << err.getErrorMessage() << endl;
        ++num_errors;
    }

    if (num_errors > 0)
    {
        cout << "Errors: " << num_errors << endl;
        return;
    }

    ofstream text_output("../test/" + output_file + ".txt");

    symbolTable.write(text_output);
    text_output << endl;
    sectionTable.write(text_output);
    relocationTable.write(text_output, &sectionTable);
    text_output << endl;

    text_output.close();

    ofstream bin_output("../test/" + output_file, ios::binary | ios::out);

    unsigned symbolTableSize = 0;
    unsigned sectionTableSize = 0;

    for (unsigned i = 0; i < 2; ++i)
        bin_output.write((char *)&symbolTableSize, sizeof(symbolTableSize));

    sectionTableSize = sectionTable.writeBinary(bin_output, &relocationTable);
    symbolTableSize = symbolTable.writeBinary(bin_output);

    bin_output.seekp(0);
    bin_output.write((char *)&sectionTableSize, sizeof(sectionTableSize));
    bin_output.write((char *)&symbolTableSize, sizeof(symbolTableSize));

    bin_output.close();
}

void Assembler::processCommand(Instruction *instr)
{
    if (instr->operation == "")
        return;
    if (current_section == nullptr)
        throw SyntaxError("Every instruction needs to be in a section");

    const auto &name = INSTRUCTIONS.find(instr->operation);
    if (name == INSTRUCTIONS.end())
        throw UnrecognizedSymbol(instr->operation);

    const InstructionDetails &instructionDetails = name->second;

    byte op_code = instructionDetails.operation_code << 3;

    if (instructionDetails.num_operands == 0)
    {
        if (instr->op1 != "" || instr->op2 != "")
            throw SyntaxError(instr->operation + " has zero arguments");

        current_section->bytes.push_back(op_code);
        return;
    }

    if (instructionDetails.num_operands == 1)
    {
        if (instr->op1 == "" || instr->op2 != "")
            throw SyntaxError(instr->operation + " has 1 argument");
    }
    else
    {
        if (instr->op1 == "" || instr->op2 == "")
            throw SyntaxError(instr->operation + " has 2 arguments");
    }

    if (instructionDetails.operand_size == 2)
        op_code |= 1 << 2;
    else if (instructionDetails.operand_size == -1)
        op_code |= 1 << 2;

    unsigned op_size = instructionDetails.operand_size == -1 ? 2 : instructionDetails.operand_size;

    current_section->bytes.push_back(op_code);

    bool mem_addr = false;
    string operands[2] = {instr->op1, instr->op2};
    for (unsigned i = 0; i < instructionDetails.num_operands; ++i)
    {
        string operand = operands[i];
        if (operand == "")
            continue;

        smatch sm;
        const string operand_regex = "(?:([^\\(\\)]+)|([^\\(]*)\\(([^\\)]+)\\))";
        if (!regex_match(operand, sm, regex(operand_regex)))
            throw SyntaxError("Bad format for operand(s)");

        if (sm[1] != "")
        {
            string operand = sm[1];
            if (instructionDetails.jump && operand[0] == '*')
            {
                if (operand[1] == '%')
                    processRegister(operand.substr(2));
                else
                {
                    if (mem_addr)
                        throw SyntaxError("Memory addressing allowed for one operand only");
                    processLiteralOrSymbol(operand.substr(1), 2, 4);
                    mem_addr = true;
                }
            }
            else if (!instructionDetails.jump && operand[0] == '$')
            {
                if (instructionDetails.num_operands == 1 && instructionDetails.operation_code != 9 || instructionDetails.num_operands == 2 && (instructionDetails.operation_code == 24 && i == 0 || (instructionDetails.operation_code != 24 && instructionDetails.operation_code != 17) && i == 1))
                    throw SyntaxError("Immediate addressing is not allowed for the destination operand");

                processLiteralOrSymbol(operand.substr(1), 2); // $<literal> ili $<simbol>
            }
            else if (!instructionDetails.jump && operand[0] == '%')
                processRegister(operand.substr(1)); // %r<num>
            else
            {
                if (!instructionDetails.jump && mem_addr)
                    throw SyntaxError("Memory addressing allowed for one operand only");
                processLiteralOrSymbol(operand, instructionDetails.jump ? op_size : 2, instructionDetails.jump ? 0 : 4); // <literal> ili <simbol>
                mem_addr = true;
            }
        }
        else
        {
            string operand = sm.str(2);
            if (instructionDetails.jump)
                if (operand[0] != '*')
                    throw SyntaxError("Invalid syntax: " + operand);
                else
                    operand.erase(0, 1);

            if (operand == "")
            {
                if (sm.str(3)[0] != '%')
                    throw UnrecognizedSymbol(sm.str(3));

                processRegister(sm.str(3).substr(1), 2); // (%r<num>)
            }
            else
            {
                if (sm.str(3)[0] != '%')
                    throw SyntaxError("Invalid syntax: " + sm.str(2) + sm.str(3));

                // <literal>(%r<num>) ili <simbol>(%r<num>) ili <simbol>(%pc/%r7)
                string reg = sm.str(3).substr(1);
                unsigned size = processRegister(reg, 3);

                if (mem_addr)
                    throw SyntaxError("Memory addressing allowed for one operand only");
                processLiteralOrSymbol(operand, size, -1, reg == "pc" || reg == "r7" ? (i == 0 && instructionDetails.num_operands == 2 && !regex_match(operands[i + 1], regex("\\*?%.*")) ? 5 : 2) : -1);
                mem_addr = true;
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
        if (instr->operation == "section" && instr->op1 == "")
            throw SyntaxError();

        string name;
        string rights = "";
        if (instr->operation == "section")
        {
            name = instr->op1;
            rights = instr->op2;
        }
        else
        {
            name = "." + instr->operation;
            rights = instr->op1 + (instr->op2 != "" ? "," + instr->op2 : "");
        }

        if (name[0] != '.')
            throw SyntaxError("Names of sections must start with .");

        if (rights == "")
        {
            if (name == ".text")
                rights = "r,w,x,p";
            else if (name == ".data")
                rights = "r,w,p";
            else if (name == ".bss")
                rights = "r,w";
            else if (name == ".rodata")
                rights = "r,p";
        }

        current_section = new Section(name, rights);
        Section *ret = sectionTable.addSection(name, current_section);
        if (current_section == ret)
        {
            unsigned id = symbolTable.insertSymbol(name, true, 0, current_section);
            current_section->id = id;
        }
        else
            current_section = ret;
    }
    else if (instr->operation == "end")
    {
        if (instr->op1 != "" || instr->op2 != "")
            throw SyntaxError();

        current_section = nullptr;
        end = true;
    }
    else if (instr->operation == "global" || instr->operation == "extern" || instr->operation == "byte" || instr->operation == "word")
    {
        if (instr->op1 == "")
            throw SyntaxError("." + instr->operation + " needs at least 1 operand");

        if (current_section == nullptr && (instr->operation == "byte" || instr->operation == "word"))
            throw SyntaxError("." + instr->operation + " needs to be in a section");

        list<string> elems = Assembler::splitString(instr->op2 == "" ? instr->op1 : instr->op1 + ',' + instr->op2, "(?:,|[^,\\s]+)");
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
                            relocationTable.add(s, current_section, current_section->bytes.size(), instr->operation == "byte" ? RelocationTable::R_X86_64_8 : RelocationTable::R_X86_64_16);
                        current_section->bytes.push_back(number & 0xff);
                        if (instr->operation == "word")
                            current_section->bytes.push_back(number >> 8 & 0xff);
                    }
                }
            }
            ++i;
        }

        if (!(i & 1))
            throw SyntaxError();
    }
    else if (instr->operation == "skip")
    {
        if (instr->op1 == "" && instr->op2 != "")
            throw SyntaxError(".skip needs exactly 1 operand");

        word number = Assembler::parseOperand(instr->op1, current_section, &symbolTable);
        if (number < 0)
            throw SyntaxError("Operand cannot be negative");

        for (unsigned i = 0; i < number; ++i)
        {
            current_section->bytes.push_back(0);
        }
    }
    else if (instr->operation == "equ")
    {
        if (instr->op1 == "" || instr->op2 == "" || regex_match(instr->op1, regex(".*(\\+|-).*")))
            throw SyntaxError();

        if (Assembler::isLiteral(instr->op1))
            throw SyntaxError();

        uncalculatedSymbols.add(instr->op1, instr->op2);
    }
    else
        throw UnrecognizedSymbol(instr->operation);
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
        throw SyntaxError("Not a number: " + arg);

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

unsigned Assembler::processRegister(string operand, byte op_code)
{
    auto reg = REGISTERS.find(operand);
    if (reg == REGISTERS.end())
        throw UnrecognizedSymbol(operand);

    const RegisterDetails &r = reg->second;

    if (r.size == 1)
        current_section->bytes[current_section->bytes.size() - 1] &= ~0x4;
    current_section->bytes.push_back(op_code << 5 | r.code << 1 | (r.size == 1 && r.high ? 1 : 0));
    return r.size;
}

void Assembler::processLiteralOrSymbol(string operand, unsigned size, byte op_code, byte pc_rel_off)
{
    bool pc_rel = pc_rel_off != -1;

    if (op_code != -1)
        current_section->bytes.push_back(op_code << 5);

    if (size == 1)
        current_section->bytes[current_section->bytes.size() - 2] &= ~0x4;

    word number = Assembler::parseOperand(operand, current_section, &symbolTable);
    if (!Assembler::isLiteral(operand))
        relocationTable.add(operand, current_section, current_section->bytes.size(), pc_rel ? RelocationTable::R_X86_64_PC16 : (size == 2 ? RelocationTable::R_X86_64_16 : RelocationTable::R_X86_64_8));
    else if (pc_rel)
        throw SyntaxError("Literals not allowed with pc relative addressing");

    SymbolTable::Symbol *symb = symbolTable.getSymbol(operand);

    if (symb && symb->global && symb->section)
        number = 0;

    if (pc_rel)
        number -= pc_rel_off;

    current_section->bytes.push_back(number & 0xff);
    if (size == 2)
        current_section->bytes.push_back(number >> 8 & 0xff);
}

list<string> Assembler::splitString(string str, string regex)
{
    list<string> elems;
    smatch sm;

    while (regex_search(str, sm, std::regex(regex)))
    {
        elems.push_back(sm.str());
        str = sm.suffix().str();
    }

    return elems;
}
