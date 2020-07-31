#ifndef SECTION_TABLE_H_
#define SECTION_TABLE_H_

#include <unordered_map>
#include <forward_list>
#include <string>
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
    forward_list<string> getAllNames() const;
    void write(ofstream& output) const;
    unsigned writeBinary(ofstream& output) const;
    friend bool operator<(const Section &s1, const Section &s2);
};

#endif
