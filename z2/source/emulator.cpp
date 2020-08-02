#include "../header/emulator.hpp"
#include "../header/syntaxErrors.hpp"
#include "../header/linker.hpp"
#include <regex>
#include <iostream>
using namespace std;

Emulator::Emulator(char *input_files[], unsigned cnt)
{
    for (unsigned i = 0; i < cnt; ++i)
    {
        string file = input_files[i];
        if (file.length() > 7 && file.substr(0, 7) == "-place=")
        {
            unsigned pos = file.find('@');
            if (pos == string::npos)
                throw SyntaxError("Error: Usage -> emulator [-place=<section_name>@<location>] input_files");
            places.push_front(make_pair<unsigned, string>(Emulator::parseInt(file.substr(pos + 1)), file.substr(7, pos - 7)));
        }
        else
            this->input_files.push_back(file);
    }
    places.sort();
}

unsigned Emulator::parseInt(string arg)
{
    unsigned number = 0;
    bool minus = false;
    if (arg[0] == '-' || arg[0] == '+')
    {
        minus = arg[0] == '-';
        arg.erase(0, 1);
    }

    if (regex_match(arg, regex("(?:0x|0X)[0-9a-fA-F]+")))
    {
        for (unsigned i = 2; i < arg.length(); ++i)
        {
            number <<= 4;
            if (arg[i] >= '0' && arg[i] <= '9')
                number += arg[i] - '0';
            else if (arg[i] >= 'a' && arg[i] <= 'f')
                number += arg[i] - 'a' + 10;
            else
                number += arg[i] - 'A' + 10;
        }
    }
    else if (regex_match(arg, regex("[0-9]+")))
        for (unsigned i = 0; i < arg.length(); ++i)
        {
            number *= 10;
            number += arg[i] - '0';
        }
    else
        throw SyntaxError("Not a number: " + arg);

    return number * (minus ? -1 : 1);
}

void Emulator::emulate()
{
    Linker linker;
    linker.link(input_files);
    linker.place(places);
    
}
