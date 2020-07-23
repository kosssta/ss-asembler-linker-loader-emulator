#ifndef ARRAY_H_
#define ARRAY_H_

#define INITIAL_SIZE 10
typedef char byte;

class Array { 
    byte *array = new byte[INITIAL_SIZE];
    unsigned size = 0;
    unsigned capacity = INITIAL_SIZE;

    void extend();

    public:
    void add(byte value);
    void add(byte values[], unsigned size);
    unsigned length() const;
    void shrink();
};

#endif
