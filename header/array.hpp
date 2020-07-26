#ifndef ARRAY_H_
#define ARRAY_H_

#include <cinttypes>
using namespace std;

#define INITIAL_SIZE 10

typedef int8_t byte;
typedef int16_t word;

class Array { 
    byte *array = new byte[INITIAL_SIZE];
    unsigned size = 0;
    unsigned capacity = INITIAL_SIZE;

    void extend();

    public:
    void add(byte value);
    void add(byte values[], unsigned size);
    void skip(unsigned number);
    unsigned length() const;
    void shrink();
    byte get(unsigned index);
    void set(unsigned index, byte value);
    void set(unsigned index, byte values[], unsigned size);
};

#endif
