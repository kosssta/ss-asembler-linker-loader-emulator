#include "lineParser.hpp"
#include "instruction.hpp"
#include <regex>
using namespace std;

Instruction* LineParser::parse(string line) {
    smatch sm;
    if (!regex_match(line, sm, regex("\\s*(?:([A-Za-z][A-Za-z0-9_]*)\\s*:\\s*){0,1}(?:([A-Za-z]+)\\s+([A-Za-z0-9_]+)\\s*(?:,\\s*([A-Za-z0-9_]+)\\s*)*){0,1}"))) {
        // TODO: Error handling
    }
    
    if (sm[1].length() == 0 && sm[2].length() == 0 && sm[3].length() == 0 && sm[4].length() == 0) return nullptr;

    return new Instruction(sm[1], sm[2], sm[3], sm[4]);
}
