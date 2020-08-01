#include "emulator.hpp"
#include "syntaxErrors.hpp"
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Error: Usage -> emulator [-place=<section_name>@<location>] input_files" << endl;
        return -1;
    }

    try
    {
        Emulator(argv + 1, argc - 1).emulate();
    }
    catch (SyntaxError err)
    {
        cerr << err.getErrorMessage() << endl;
    }
    return 0;
}
