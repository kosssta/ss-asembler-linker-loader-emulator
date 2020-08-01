#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <string>
#include <list>
#include <unordered_map>
using namespace std;

class Emulator
{
public:
    Emulator(char *input_files[], unsigned cnt);
    void emulate();

    static unsigned parseInt(string number);

    private:
    list<string> input_files;
    unordered_map<string, unsigned> places;
};

#endif
