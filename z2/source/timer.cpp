#include "../header/timer.hpp"
#include <chrono>
using namespace std;

const unsigned Timer::INTERVALS[] = {500, 1000, 1500, 2000, 5000, 10000, 30000, 60000};

Timer::Timer(uword *interval, bool *interrupt)
{
    this->interval = interval;
    this->interrupt = interrupt;
    timer = new thread(&Timer::timer_func, this);
}

Timer::~Timer()
{
    if (!end)
        terminate();
}

void Timer::timer_func()
{
    while (true)
    {
        this_thread::sleep_for(chrono::milliseconds(INTERVALS[*interval]));
        if (end)
            break;
        *interrupt = true;
    }
}

void Timer::terminate()
{
    end = true;
    timer->join();
    delete timer;
    timer = nullptr;
}
