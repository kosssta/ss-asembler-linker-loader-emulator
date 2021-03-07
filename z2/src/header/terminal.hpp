#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "../header/semaphore.hpp"
#include <thread>
using namespace std;

class Emulator;
typedef uint16_t uword;

class Terminal
{
public:
    Terminal(uword *input, uword *output, bool *interrupt, Semaphore *outputDone);
    ~Terminal();

    static char getch();
    void input_func();
    void output_func();
    void terminate();

    friend class Emulator;
private:
    thread *terminal_input = nullptr;
    thread *terminal_output = nullptr;
    uword *input = nullptr;
    uword *output = nullptr;
    bool *interrupt = nullptr;
    bool end = false;
    Semaphore waitForChar;
    Semaphore *outputDone = nullptr;
};

#endif
