#include "../header/emulator.hpp"
#include "../header/syntaxErrors.hpp"
#include "../header/linker.hpp"
#include "../header/timer.hpp"
#include "../header/terminal.hpp"
#include <regex>
#include <iostream>
using namespace std;

Operation Emulator::operations[] = {&Emulator::halt, &Emulator::iret, &Emulator::ret, &Emulator::Int, &Emulator::call, &Emulator::jmp, &Emulator::jeq, &Emulator::jne, &Emulator::jgt, &Emulator::push, &Emulator::pop, &Emulator::xchg, &Emulator::mov, &Emulator::add, &Emulator::sub, &Emulator::mul, &Emulator::div, &Emulator::cmp, &Emulator::Not, &Emulator::And, &Emulator::Or, &Emulator::Xor, &Emulator::test, &Emulator::shl, &Emulator::shr};

Emulator::Emulator(char *input_files[], unsigned cnt) : terminalOutput(1)
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

Emulator::~Emulator()
{
    cleanEmulation();
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

    cout << "Emulation started\n" << endl;

    while (!end)
    {
        unsigned char op_code = memory[PC] >> 3 & 0x1f;
        if (op_code < NUM_OPERATIONS)
            (this->*operations[op_code])();
        else
        {
            ++PC;
            interrupt(1);
        }
        if (op_code != 1)
            checkInterrupts();
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
    pc = (uword *)&registers[PC_REG];
    sp = (uword *)&registers[SP_REG];

    for (auto s : sections)
    {
        for (unsigned i = 0; i < s.second->size(); ++i)
            memory[s.first + i] = (*s.second)[i];
    }

    SP = Emulator::MEMORY_MAPPED_REGISTERS_START_ADDRESS;
    PC = linker.getMain();
    memory[--SP] = PC >> 8 & 0xff;
    memory[--SP] = PC & 0xff;
    PC = memory[0] & 0xff | memory[1] << 8;

    timer = new Timer((uword *)(memory + 0xff10), &timerInterrupt);
    terminal = new Terminal((uword *)(memory + 0xff02), (uword *)(memory + 0xff00), &terminalInterrupt, &terminalOutput);
}

void Emulator::cleanEmulation()
{
    delete memory;
    memory = nullptr;
    delete registers;
    registers = nullptr;
    pc = sp = nullptr;
    if (timer)
        timer->terminate();
    if (terminal)
        terminal->terminate();
    delete timer;
    delete terminal;
    timer = nullptr;
    terminal = nullptr;
}

unsigned Emulator::getOperandsSize()
{
    return (memory[PC++] >> 2 & 1) + 1;
}

void Emulator::getSrcOperand(unsigned size)
{
    if (size == 0)
        return;

    src = 0;
    byte next = memory[PC++];
    byte addr = next >> 5 & 0x7;

    switch (addr)
    {
    case IMM:
        src |= memory[PC++] & 0xff;
        if (size == 2)
            src |= memory[PC++] << 8;
        break;

    case REG_DIR:
    {
        unsigned reg = next >> 1 & 0xf;
        if (!(reg <= 7 || reg == 0xf))
        {
            interrupt(1);
            return;
        }
        src |= size == 2 ? registers[reg] : registers[reg] >> 8 * (next & 1) & 0xff;
    }
    break;

    case REG_IND:
    case REG_IND_DISP:
    {
        unsigned reg = next >> 1 & 0xf;
        word disp = 0;
        if (!(reg <= 7 || reg == 0xf))
        {
            interrupt(1);
            return;
        }

        if (addr == REG_IND_DISP)
        {
            disp |= memory[PC++] & 0xff;
            disp |= memory[PC++] << 8;
        }

        src |= memory[(uword)registers[reg] + disp] & 0xff;
        if (size == 2)
            src |= memory[(uword)registers[reg] + disp + 1] << 8;
    }
    break;

    case MEM_DIR:
    {
        unsigned short address = 0;
        address |= memory[PC++] & 0xff;
        address |= memory[PC++] << 8;

        src |= memory[address] & 0xff;
        if (size == 2)
            src |= memory[(address + 1) % MEMORY_CAPACITY] << 8;
    }
    break;

    default:
        interrupt(1);
        break;
    }
}

void Emulator::getDstOperand(unsigned size, bool jump)
{
    dst = 0;
    dstAddress = 0;
    byte next = memory[PC++];
    byte addr = next >> 5 & 0x7;

    switch (addr)
    {
    case IMM:
        if (!jump)
        {
            interrupt(1);
            return;
        }
        dst |= memory[PC++] & 0xff;
        if (size == 2)
            dst |= memory[PC++] << 8;
        break;

    case REG_DIR:
    {
        unsigned reg = next >> 1 & 0xf;
        if (!(reg <= 7 || reg == 0xf))
        {
            interrupt(1);
            return;
        }

        reg_h = next & 1;
        dst |= size == 2 ? registers[reg] : registers[reg] >> 8 * reg_h & (reg_h ? 0xff00 : 0x00ff);
        dstAddress = reg;
        memAdr = false;
    }
    break;

    case REG_IND:
    case REG_IND_DISP:
    {
        unsigned reg = next >> 1 & 0xf;
        word disp = 0;
        if (!(reg <= 7 || reg == 0xf))
        {
            interrupt(1);
            return;
        }

        if (addr == REG_IND_DISP)
        {
            disp |= memory[PC++] & 0xff;
            disp |= memory[PC++] << 8;
        }

        dst |= memory[(uword)registers[reg] + disp] & 0xff;
        if (size == 2)
            dst |= ((memory[(uword)registers[reg]] + disp + 1) % MEMORY_CAPACITY) << 8;
        dstAddress = registers[reg] + disp;
        memAdr = true;
        if (jump && reg == PC_REG)
            dst = dstAddress;
    }
    break;

    case MEM_DIR:
    {
        dstAddress |= memory[PC++] & 0xff;
        dstAddress |= memory[PC++] << 8;
        memAdr = true;
        dst &= ~0xff;
        dst |= memory[dstAddress] & 0xff;
        if (size == 2) {
            dst &= 0xff;
            dst |= memory[(dstAddress + 1) % MEMORY_CAPACITY] << 8;
        }
    }
    break;

    default:
        interrupt(1);
        break;
    }
}

void Emulator::writeDst(unsigned size)
{
    if (memAdr)
    {
        if (dstAddress == 0xFF00)
            terminalOutput.acquire();
        memory[dstAddress] = dst & 0xff;
        if (size == 2)
            memory[(dstAddress + 1) % MEMORY_CAPACITY] = dst >> 8 & 0xff;
        if (dstAddress == 0xFF00)
            terminal->waitForChar.release();
    }
    else
    {
        if (size == 2)
            registers[dstAddress] = dst;
        else
        {
            registers[dstAddress] &= reg_h ? 0xff00 : 0x00ff;
            registers[dstAddress] |= dst << 8 * reg_h & (reg_h ? 0xff00 : 0x00ff);
        }
    }
}

void Emulator::updatePSW(word result, word flags)
{
    PSW &= ~flags;

    if (flags & PSW_Z && result == 0)
        PSW |= PSW_Z;
    if (flags & PSW_O && (src > 0 && dst < 0 && result < 0 || src < 0 && dst > 0 && result >= 0))
        PSW |= PSW_O;
    if (flags & PSW_N && result < 0)
        PSW |= PSW_N;
}

void Emulator::interrupt(unsigned entry)
{
    memory[--SP] = PC >> 8 & 0xff;
    memory[--SP] = PC & 0xff;
    memory[--SP] = PSW >> 8 & 0xff;
    memory[--SP] = PSW & 0xff;
    PSW |= PSW_I;
    PC = memory[entry % 8 << 1] & 0xff | memory[(entry % 8 << 1) + 1] << 8;
}

void Emulator::checkInterrupts()
{
    if (PSW & PSW_I)
        return;

    if (!(PSW & PSW_Tr) && timerInterrupt)
    {
        interrupt(2);
        timerInterrupt = false;
    }
    else if (!(PSW & PSW_Tl) && terminalInterrupt)
    {
        interrupt(3);
        terminalInterrupt = false;
    }
}

void Emulator::halt()
{
    terminalOutput.acquire();
    end = true;
}

void Emulator::iret()
{
    PC = 0;
    PSW = 0;
    PSW |= memory[SP++] & 0xff;
    PSW |= memory[SP++] << 8;
    PC |= memory[SP++] & 0xff;
    PC |= (word)memory[SP++] << 8;
}

void Emulator::ret()
{
    PC = memory[SP++] & 0xff | memory[SP++] << 8;
}

void Emulator::Int()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size, true);
    interrupt(dst);
}

void Emulator::call()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size, true);
    memory[--SP] = PC >> 8 & 0xff;
    memory[--SP] = PC & 0xff;
    PC = dst;
}

void Emulator::jmp()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size, true);
    PC = dst;
}

void Emulator::jeq()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size, true);
    if (PSW & PSW_Z)
        PC = dst;
}

void Emulator::jne()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size, true);
    if (!(PSW & PSW_Z))
        PC = dst;
}

void Emulator::jgt()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size, true);
    if ((PSW & PSW_N) ^ (PSW & PSW_O))
        PC = dst;
}

void Emulator::push()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    memory[--SP] = src >> 8 & 0xff;
    memory[--SP] = src & 0xff;
}

void Emulator::pop()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size);
    dst = 0;
    dst |= memory[SP++] & 0xff;
    dst |= memory[SP++] << 8;
    writeDst(op_size);
}

void Emulator::xchg()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size);
    word tmp_dst = dst;
    unsigned short tmp_dstAddress = dstAddress;
    bool tmp_memAdr = memAdr;
    bool tmp_reg_h = reg_h;
    getDstOperand(op_size);

    word temp = dst;
    dst = tmp_dst;
    writeDst(op_size);

    dst = temp;
    dstAddress = tmp_dstAddress;
    memAdr = tmp_memAdr;
    reg_h = tmp_reg_h;
    writeDst(op_size);
}

void Emulator::mov()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    dst = src;
    writeDst(op_size);
    updatePSW(dst, PSW_Z | PSW_N);
}

void Emulator::add()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst + src, PSW_Z | PSW_N | PSW_O);
    if ((int)src + (int)dst != (int)(src + dst))
        PSW |= PSW_C;
    else
        PSW &= ~PSW_C;
    dst += src;
    writeDst(op_size);
}

void Emulator::sub()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst - src, PSW_Z | PSW_N | PSW_O);
    if ((int)dst - (int)src != (int)(dst - src))
        PSW |= PSW_C;
    else
        PSW &= ~PSW_C;
    dst -= src;
    writeDst(op_size);
}

void Emulator::mul()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst * src, PSW_Z | PSW_N);
    dst *= src;
    writeDst(op_size);
}

void Emulator::div()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    if (src == 0)
    {
        interrupt(1);
        return;
    }
    updatePSW(dst / src, PSW_Z | PSW_N);
    dst /= src;
    writeDst(op_size);
}

void Emulator::cmp()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    word s = src;
    getSrcOperand(op_size);
    updatePSW(src - s, PSW_Z | PSW_N | PSW_O);
    if ((int)src - (int)s != (int)(src - s))
        PSW |= PSW_C;
    else
        PSW &= ~PSW_C;
}

void Emulator::Not()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(~src, PSW_Z | PSW_N);
    dst = ~src;
    writeDst(op_size);
}

void Emulator::And()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst & src, PSW_Z | PSW_N);
    dst &= src;
    writeDst(op_size);
}

void Emulator::Or()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst | src, PSW_Z | PSW_N);
    dst |= src;
    writeDst(op_size);
}

void Emulator::Xor()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst ^ src, PSW_Z | PSW_N);
    dst ^= src;
    writeDst(op_size);
}

void Emulator::test()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst & src, PSW_Z | PSW_N);
}

void Emulator::shl()
{
    unsigned op_size = getOperandsSize();
    getSrcOperand(op_size);
    getDstOperand(op_size);
    updatePSW(dst << src, PSW_Z | PSW_N);
    if (dst != 0 && src >= 8 || (int)dst << src != (int)(dst << src))
        PSW |= PSW_C;
    else
        PSW &= ~PSW_C;
    dst <<= src;
    writeDst(op_size);
}

void Emulator::shr()
{
    unsigned op_size = getOperandsSize();
    getDstOperand(op_size);
    getSrcOperand(op_size);
    updatePSW(dst >> src, PSW_Z | PSW_N);
    if (dst != 0 && src >= 8 || ((int)dst << 8) >> src & 0xff)
        PSW |= PSW_C;
    else
        PSW &= ~PSW_C;
    dst >>= src;
    writeDst(op_size);
}
