#ifndef SECTION_TABLE_H_
#define SECTION_TABLE_H_

#include <unordered_map>
#include <string>
using namespace std;

struct Section;

class SectionTable {
    public:
    Section* addSection(Section *section);
    Section* findSection(string name) const;
    
    private:
    unordered_map<string, Section *> sections;
};

#endif
