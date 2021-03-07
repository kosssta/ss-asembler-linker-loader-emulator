#include "../header/assembler.hpp"
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

int main(int argc, char *argv[])
{
    const string error_message = "Error: Usage -> asembler [-o output_file_name] input_file_name";

    if (argc != 2 && argc != 4)
    {
        cerr << error_message << endl;
        return -1;
    }

    string output_file = "a.o";
    string input_file = "";

    if (argc == 2)
    {
        if (strcmp(argv[1], "-o") == 0)
        {
            cerr << error_message << endl;
            return -1;
        }
        else
            input_file = argv[1];
    }
    else
    {
        if (strcmp(argv[1], "-o") == 0 && strcmp(argv[2], "-o") != 0 && strcmp(argv[3], "-o") != 0)
        {
            output_file = argv[2];
            input_file = argv[3];
        }
        else if (strcmp(argv[1], "-o") != 0 && strcmp(argv[2], "-o") == 0 && strcmp(argv[3], "-o") != 0)
        {
            output_file = argv[3];
            input_file = argv[1];
        }
        else
        {
            cerr << error_message << endl;
            return -1;
        }
    }

    Assembler assembler;
    assembler.assemble(input_file, output_file);

    return 0;
}
