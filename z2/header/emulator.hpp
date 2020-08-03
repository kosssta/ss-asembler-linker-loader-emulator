#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <string>
#include <list>
#include <utility>
using namespace std;

class Emulator;

typedef int8_t byte;
typedef int16_t word;
typedef void (Emulator::*Operation)();

class Emulator
{
public:
    static const unsigned MEMORY_CAPACITY = 0x10000;
    static const unsigned MEMORY_MAPPED_REGISTERS_START_ADDRESS = 0xFF00;
    static const unsigned MEMORY_MAPPED_REGISTERS_SIZE = 256;
    static const unsigned IV_TABLE_START_ADDRESS = 0;
    static const unsigned IV_TABLE_NUM_ENTRIES = 8;
    static const unsigned IV_TABLE_ENTRY_SIZE = 2;
    static const unsigned NUM_REGISTERS = 8;
    static const unsigned NUM_OPERATIONS = 24;
    static const unsigned sp = 6;
    static const unsigned pc = 7;
    static Operation operations[];

    enum ADDRESSING {IMM, REG_DIR, REG_IND, REG_IND_DISP, MEM_DIR};

    Emulator(char *input_files[], unsigned cnt);
    void emulate();

    static unsigned parseInt(string number);

private:
    list<string> input_files;
    list<pair<unsigned, string>> places;

    byte *memory = nullptr;
    word *registers = nullptr;
    word *psw = nullptr;
    bool end = false;
    bool memAdr = false;
    unsigned mem_address;
    bool reg_h = false;
    unsigned reg;

    void prepareEmulation();
    void cleanEmulation();
    unsigned getOperandsSize();
    word getOperand(unsigned size);
    void calculateDestAddress();

    void halt();
    void iret();
    void ret();
    void Int();
    void call();
    void jmp();
    void jeq();
    void jne();
    void jgt();
    void push();
    void pop();
    void xchg();
    void mov();
    void add();
    void sub();
    void mul();
    void div();
    void cmp();
    void Not();
    void And();
    void Or();
    void Xor();
    void test();
    void shl();
    void shr();
};

#endif
