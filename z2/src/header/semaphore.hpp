#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <mutex>
#include <condition_variable>
using namespace std;

class Semaphore
{
public:
    Semaphore(int count = 0) : count(count) {}
    void acquire();
    void release();

private:
    mutex mut;
    condition_variable condition;
    int count;
};

#endif
