#include <iostream>
#include <string>
#include "assembler.hpp"
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        cout << "Greska: Potrebno je proslediti tacno 1 ulazni fajl" << endl;
        return 0;
    }

    string output_file = "a.out";
    string input_file;

    for (int i = 1; i < argc; i++) {
        if (argv[i] == "-o") {
            output_file = argv[i++ + 1];
        }
        input_file = argv[i];
    }

    Assembler assembler;
    assembler.assembly(input_file, output_file);

    return 0;
}
