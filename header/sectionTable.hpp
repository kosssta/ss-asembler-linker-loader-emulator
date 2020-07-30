#ifndef SECTION_TABLE_H_
#define SECTION_TABLE_H_

#include <unordered_map>
#include <fstream>
#include <iostream>
using namespace std;

struct Section;

class SectionTable {
    unordered_map<string, Section*> sections;
    
    public:
    ~SectionTable();
    Section* addSection(string name, Section *section);
    Section* findSection(string name) const;
    void write(ofstream& output) const;
};

#endif
