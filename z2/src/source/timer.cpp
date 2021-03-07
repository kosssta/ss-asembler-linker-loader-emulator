#include "../header/timer.hpp"
#include <chrono>
using namespace std;

const unsigned Timer::INTERVALS[] = {1, 2, 3, 4, 10, 20, 60, 120};

Timer::Timer(uword *interval, bool *interrupt)
{
    this->interval = interval;
    *this->interval = 0;
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
        for (unsigned i = 0; i < INTERVALS[*interval]; ++i)
        {
            this_thread::sleep_for(chrono::milliseconds(500));
            if (end)
                return;
        }
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
