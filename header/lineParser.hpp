#ifndef LINEPARSER_H_
#define LINEPARSER_H_

#include <string>
using namespace std;

struct Instruction;

class LineParser {
    public:
    static Instruction* parse(string line);
};

#endif
