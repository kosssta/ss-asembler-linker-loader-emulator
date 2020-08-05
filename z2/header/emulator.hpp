#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <string>
#include <list>
#include <utility>
using namespace std;

#define PC (*pc)
#define SP (*sp)
#define PSW (registers[PSW_REG])

class Emulator;

typedef int8_t byte;
typedef int16_t word;
typedef uint16_t uword;
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
    static const unsigned NUM_REGISTERS = 16;
    static const unsigned NUM_OPERATIONS = 25;
    static const unsigned SP_REG = 6;
    static const unsigned PC_REG = 7;
    static const unsigned PSW_REG = 15;
    static Operation operations[];

    enum ADDRESSING {IMM, REG_DIR, REG_IND, REG_IND_DISP, MEM_DIR};
    enum FLAGS {PSW_Z, PSW_O = 2, PSW_C = 4, PSW_N = 8, PSW_Tr = 0x2000, PSW_Tl = 0x4000, PSW_I = 0x8000};

    Emulator(char *input_files[], unsigned cnt);
    void emulate();

    static unsigned parseInt(string number);

private:
    list<string> input_files;
    list<pair<unsigned, string>> places;

    byte *memory = nullptr;
    word *registers = nullptr;
    uword *pc = nullptr;
    uword *sp = nullptr;
    bool end = false;
    bool timerInterrupt = false;
    bool inputInterrupt = false;
    bool outputInterrupt = false;

    word src;
    word dst;
    unsigned short dstAddress;
    bool memAdr = false;
    bool reg_h = false;

    void prepareEmulation();
    void cleanEmulation();
    unsigned getOperandsSize();
    void getSrcOperand(unsigned size);
    void getDstOperand(unsigned size, bool jump = false);
    void writeDst(unsigned size);
    void updatePSW(word result, word flags);
    void interrupt(unsigned entry);
    void checkInterrupts();

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
