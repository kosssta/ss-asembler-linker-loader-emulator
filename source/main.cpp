#include <iostream>
#include <string>
#include <string.h>
#include "assembler.hpp"
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        cout << "Greska: Potrebno je proslediti tacno 1 ulazni fajl" << endl;
        return 0;
    }

    string output_file = "a.out";
    string input_file = "";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc)
                output_file = argv[++i];
            else {
                cout << "Greska u sintaksi" << endl;
                return 0;
            }
        } else {
            input_file = argv[i];
        }
    }

    Assembler assembler;
    assembler.assembly(input_file, output_file);

    return 0;
}
