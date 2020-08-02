#include "../header/lineParser.hpp"
#include "../header/instruction.hpp"
#include "../header/syntaxErrors.hpp"
#include <regex>
using namespace std;

Instruction* LineParser::parse(string line) {
    smatch sm;
    bool isDirective = false;

    // strings issue 
    if (!regex_match(line, sm, regex("\\s*(?:([A-Za-z0-9_\\.][A-Za-z0-9_]*)\\s*:\\s*){0,1}(?:([A-Za-z0-9_]+)(?:\\s+([^,\\s]+)\\s*(?:,\\s*([^,\\s]+)\\s*)*){0,1}){0,1}"))) {
        if(!regex_match(line, sm, regex("\\s*(?:([A-Za-z0-9_\\.][A-Za-z0-9_]*)\\s*:\\s*){0,1}(?:\\.([A-Za-z0-9_\\.]+)\\s*(?:(?:\\s+([^,]+)\\s*,{0,1})(?:\\s*(.+)\\s*)*)*){0,1}"))) {
            throw SyntaxError();
        }

        if (sm[1].length() != 0 || sm[2].length() != 0 || sm[3].length() != 0 || sm[4].length() != 0) isDirective = true;
    }
    
    if (sm[1].length() == 0 && sm[2].length() == 0 && sm[3].length() == 0 && sm[4].length() == 0) return nullptr;

    return new Instruction(sm[1], sm[2], sm[3], sm[4], isDirective);
}
