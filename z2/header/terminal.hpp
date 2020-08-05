#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <thread>
using namespace std;

typedef uint16_t uword;

class Terminal
{
public:
    Terminal(uword *input, uword *output, bool *interrupt_input, bool *interrupt_output);
    ~Terminal();

    void input_func();
    void output_func();
    void terminate();

private:
    thread *terminal_input = nullptr;
    thread *terminal_output = nullptr;
    uword *input = nullptr;
    uword *output = nullptr;
    bool *interrupt_input = nullptr;
    bool *interrupt_output = nullptr;
    bool end = false;
};

#endif
