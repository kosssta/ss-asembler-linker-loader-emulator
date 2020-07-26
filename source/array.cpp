#include "array.hpp"
#include <stdexcept>
using namespace std;

void Array::add(byte value)
{
    byte arr[] = {value};
    add(arr, 1);
}

void Array::add(byte values[], unsigned size)
{
    while (length() + size > capacity)
        extend();

    for (unsigned i = 0; i < size; ++i)
    {
        array[this->size++] = values[i];
    }
}

void Array::extend()
{
    byte *new_array = new byte[capacity *= 2];
    for (unsigned i = 0; i < size; ++i)
    {
        new_array[i] = array[i];
    }
    delete array;
    array = new_array;
}

unsigned Array::length() const
{
    return size;
}

void Array::shrink()
{
    if (size < capacity)
    {
        byte *new_array = new byte[size];
        for (unsigned i = 0; i < size; ++i)
        {
            new_array[i] = array[i];
        }
        delete array;
        array = new_array;
    }
}

byte Array::get(unsigned index)
{
    if (index >= size)
        throw out_of_range("Index out of range");
    return array[index];
}

void Array::set(unsigned index, byte value)
{
    byte tmp[] = {value};
    set(index, tmp, 1);
}

void Array::set(unsigned index, byte values[], unsigned size)
{
    if (index + size > length())
        throw out_of_range("Index out of range");

    for (unsigned i = 0; size > 0; --size, ++i)
    {
        array[i + index] = values[i];
    }
}

void Array::skip(unsigned size)
{
    while (length() + size > capacity)
        extend();
    this->size += size;
}
