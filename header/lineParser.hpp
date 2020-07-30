#ifndef LINE_PARSER_H_
#define LINE_PARSER_H_

#include <string>
using namespace std;

struct Instruction;

class LineParser {
    public:
    static Instruction* parse(string line);
};

#endif
