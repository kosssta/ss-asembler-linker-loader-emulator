#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <string>
#include <list>
#include <utility>
using namespace std;

typedef int8_t byte;
typedef int16_t word;

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
    
    Emulator(char *input_files[], unsigned cnt);
    void emulate();

    static unsigned parseInt(string number);

    private:
    list<string> input_files;
    list<pair<unsigned, string>> places;
    
    byte *memory;
    word *registers;
    word *psw;
};

#endif
