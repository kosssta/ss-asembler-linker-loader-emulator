#include "sectionTable.hpp"
#include "section.hpp"
#include <iostream>
using namespace std;

SectionTable::SectionTable()
{
    global = new Section();
}

Section *SectionTable::addSection(string name, Section *section)
{
    unordered_map<string, Section *>::iterator ret = sections.find(name);
    if (ret != sections.end())
    {
        delete section;
        return ret->second;
    }
    sections[name] = section;
    return section;
}

Section *SectionTable::findSection(string name) const
{
    unordered_map<string, Section *>::const_iterator ret = sections.find(name);
    if (ret != sections.end())
        return ret->second;
    return nullptr;
}

void SectionTable::write() const
{
    for (auto &s : sections)
    {
        Section *curr = s.second;

        cout << '.' << s.first << ':' << endl; 
        cout << "Length = " << curr->bytes.length() << endl;
        for (unsigned i = 0; i < curr->bytes.length(); ++i)
        {
            byte tmp = (curr->bytes.get(i) >> 4) & 0x0f;
            if (tmp < 10)
                cout << (char)('0' + tmp);
            else
                cout << (char)('A' - 10 + tmp);
            tmp = curr->bytes.get(i) & 0x0f;
            if (tmp < 10)
                cout << (char)('0' + tmp) << ' ';
            else
                cout << (char)('A' - 10 + tmp) << ' ';
        }
        cout << endl;
    }
}
