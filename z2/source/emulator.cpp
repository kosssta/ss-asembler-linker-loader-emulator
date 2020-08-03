#include "../header/emulator.hpp"
#include "../header/syntaxErrors.hpp"
#include "../header/linker.hpp"
#include <regex>
#include <iostream>
using namespace std;

Operation Emulator::operations[] = {&Emulator::halt, &Emulator::iret, &Emulator::ret, &Emulator::Int, &Emulator::call, &Emulator::jmp, &Emulator::jeq, &Emulator::jne, &Emulator::jgt, &Emulator::push, &Emulator::pop, &Emulator::xchg, &Emulator::mov, &Emulator::add, &Emulator::sub, &Emulator::mul, &Emulator::div, &Emulator::cmp, &Emulator::Not, &Emulator::And, &Emulator::Or, &Emulator::Xor, &Emulator::test, &Emulator::shl, &Emulator::shr};

Emulator::Emulator(char *input_files[], unsigned cnt)
{
    for (unsigned i = 0; i < cnt; ++i)
    {
        string file = input_files[i];
        if (file.length() > 7 && file.substr(0, 7) == "-place=")
        {
            unsigned pos = file.find('@');
            if (pos == string::npos)
                throw SyntaxError("Error: Usage -> emulator [-place=<section_name>@<location>] input_files");
            places.push_front(make_pair<unsigned, string>(Emulator::parseInt(file.substr(pos + 1)), file.substr(7, pos - 7)));
        }
        else
            this->input_files.push_back(file);
    }
    places.sort();
}

unsigned Emulator::parseInt(string arg)
{
    unsigned number = 0;
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

void Emulator::emulate()
{
    prepareEmulation();

    while (!end)
    {
        unsigned char op_code = memory[registers[pc]] >> 3 & 0x1f;
        if (op_code >= NUM_OPERATIONS)
            break;
        (this->*operations[op_code])();
    }

    cleanEmulation();
}

void Emulator::prepareEmulation()
{
    Linker linker;
    linker.link(input_files);
    linker.place(places);
    unordered_map<unsigned, vector<byte> *> sections = linker.getAllSections();

    memory = new byte[MEMORY_CAPACITY];
    registers = new word[NUM_REGISTERS]();
    psw = new word();

    for (auto s : sections)
    {
        for (unsigned i = 0; i < s.second->size(); ++i)
            memory[s.first + i] = (*s.second)[i];
    }

    registers[sp] = Emulator::MEMORY_MAPPED_REGISTERS_START_ADDRESS;
    registers[pc] = linker.getMain();
}

void Emulator::cleanEmulation()
{
    delete memory;
    memory = nullptr;
    delete registers;
    registers = nullptr;
    delete psw;
    psw = nullptr;
}

unsigned Emulator::getOperandsSize()
{
    return (memory[registers[pc]++] >> 2 & 1) + 1;
}

word Emulator::getOperand(unsigned size)
{
    if (size == 0)
        return 0;

    word ret = 0;
    byte next = memory[registers[pc]++];
    byte addr = next >> 5 & 0x7;

    switch (addr)
    {
    case IMM:
        ret |= memory[registers[pc]++];
        if (size == 2)
            ret |= memory[registers[pc]++] << 8;
        break;

    case REG_DIR:
    {
        unsigned reg = next >> 1 & 0xf;
        if (!(reg <= 7 || reg == 0xf))
        {
            //error
            return 0;
        }
        if (size == 2)
            ret = registers[reg];
        else
            ret |= registers[reg] >> 8 * (next & 1) & 0xff;
    }
    break;

    case REG_IND:
    case REG_IND_DISP:
    {
        unsigned reg = next >> 1 & 0xf;
        word disp = 0;
        if (!(reg <= 7 || reg == 0xf))
        {
            //error
            return 0;
        }

        if (addr == REG_IND_DISP)
        {
            disp |= memory[registers[pc]++];
            disp |= memory[registers[pc]++] << 8;
        }

        ret |= memory[registers[reg] + disp];
        if (size == 2)
            ret |= ((memory[registers[reg]] + disp + 1) % MEMORY_CAPACITY) << 8;
    }
    break;

    case MEM_DIR:
    {
        unsigned address = 0;
        address |= memory[registers[pc]++];
        address |= memory[registers[pc]++] << 8;

        ret |= memory[address];
        if (size == 2)
            ret |= memory[(address + 1) % MEMORY_CAPACITY] << 8;
    }
    break;

    default:
        // error
        break;
    }

    return ret;
}

void Emulator::calculateDestAddress()
{
    byte next = memory[registers[pc]++];
    byte addr = next >> 5 & 0x7;

    switch (addr)
    {
    case IMM:
        //error
        break;

    case REG_DIR:
    {
        unsigned reg = next >> 1 & 0xf;
        if (!(reg <= 7 || reg == 0xf))
        {
            //error
        }
        this->reg = reg;
        this->reg_h = next & 1;
        this->memAdr = false;
    }
    break;

    case REG_IND:
    case REG_IND_DISP:
    {
        unsigned reg = next >> 1 & 0xf;
        word disp = 0;
        if (!(reg <= 7 || reg == 0xf))
        {
            //error
        }

        if (addr == REG_IND_DISP)
        {
            disp |= memory[registers[pc]++];
            disp |= memory[registers[pc]++] << 8;
        }

        mem_address = registers[reg] + disp;
        memAdr = true;
    }
    break;

    case MEM_DIR:
    {
        mem_address |= memory[registers[pc]++];
        mem_address |= memory[registers[pc]++] << 8;
        memAdr = false;
    }
    break;

    default:
        // error
        break;
    }
}

void Emulator::halt()
{
    end = true;
}

void Emulator::iret()
{
}

void Emulator::ret()
{
}

void Emulator::Int()
{
}

void Emulator::call()
{
}

void Emulator::jmp()
{
    cout << "jmp" << endl;
    unsigned op_size = getOperandsSize();

    end = true;
}

void Emulator::jeq()
{
}

void Emulator::jne()
{
}

void Emulator::jgt()
{
}

void Emulator::push()
{
}

void Emulator::pop()
{
}

void Emulator::xchg()
{
}

void Emulator::mov()
{
    cout << "mov" << endl;
    unsigned op_size = getOperandsSize();
    word src = getOperand(op_size);
    calculateDestAddress();
    if (memAdr)
    {
        memory[mem_address] = src & 0xff;
        if (op_size == 2)
            memory[(mem_address + 1) % MEMORY_CAPACITY] = src >> 8 & 0xff;
    }
    else
    {
        if (op_size == 2)
            registers[reg] = src;
        else
        {
            registers[reg] &= reg_h ? 0x00ff : 0xff00;
            registers[reg] |= reg_h ? reg << 8 & 0xff : reg & 0xff;
        }
    }
}

void Emulator::add()
{
}

void Emulator::sub()
{
}

void Emulator::mul()
{
}

void Emulator::div()
{
}

void Emulator::cmp()
{
}

void Emulator::Not()
{
}

void Emulator::And()
{
}

void Emulator::Or()
{
}

void Emulator::Xor()
{
}

void Emulator::test()
{
}

void Emulator::shl()
{
}

void Emulator::shr()
{
}
