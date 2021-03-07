#include "../header/semaphore.hpp"

void Semaphore::acquire()
{
    unique_lock<mutex> lock(mut);
    while (count <= 0)
        condition.wait(lock);
    --count;
}

void Semaphore::release()
{
    lock_guard<mutex> lock(mut);
    ++count;
    condition.notify_one();
}

