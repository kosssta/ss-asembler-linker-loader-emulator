#ifndef TIMER_H_
#define TIMER_H_

#include <thread>
using namespace std;
typedef uint16_t uword;

class Timer
{
public:
    const static unsigned INTERVALS[];

    Timer(uword *interval, bool *interrupt);
    ~Timer();

    void timer_func();
    void terminate();

private:
    thread *timer = nullptr;
    uword *interval = nullptr;
    bool *interrupt = nullptr;
    bool end = false;
};

#endif
