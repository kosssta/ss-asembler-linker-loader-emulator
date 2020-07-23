#include "sectionTable.hpp"
#include "section.hpp"

Section* SectionTable::addSection(string name, Section *section) {
    unordered_map<string, Section*>::iterator ret = sections.find(name);
    if(ret != sections.end()) return ret->second;
    sections[name] = section;
    return section;
}

Section* SectionTable::findSection(string name) {
    unordered_map<string, Section*>::iterator ret = sections.find(name);
    if(ret != sections.end()) return ret->second;
    return nullptr;
}
