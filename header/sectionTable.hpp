#ifndef SECTIONTABLE_H_
#define SECTIONTABLE_H_

#include <unordered_map>
using namespace std;

struct Section;

class SectionTable {
    unordered_map<string, Section*> sections;
    
    public:
    Section *global = nullptr;
    SectionTable();
    Section* addSection(string name, Section *section);
    Section* findSection(string name) const;
    void write() const;
};

#endif
