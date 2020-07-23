#include "array.hpp"

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
