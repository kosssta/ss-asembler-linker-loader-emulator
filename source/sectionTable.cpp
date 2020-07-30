#include "sectionTable.hpp"
#include "section.hpp"
using namespace std;

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

void SectionTable::write(ofstream &output) const
{
    output << "=== Sections ===" << endl;
    if (sections.empty())
    {
        output << "No sections" << endl;
        return;
    }

    for (auto &s : sections)
    {
        Section *curr = s.second;

        output << curr->name << ':' << endl;
        output << "Length = " << curr->bytes.size() << endl;
        for (byte b : curr->bytes)
        {
            byte tmp = (b >> 4) & 0x0f;
            if (tmp < 10)
                output << (char)('0' + tmp);
            else
                output << (char)('A' - 10 + tmp);
            tmp = b & 0x0f;
            if (tmp < 10)
                output << (char)('0' + tmp) << ' ';
            else
                output << (char)('A' - 10 + tmp) << ' ';
        }
        output << endl
               << endl;
    }
}

SectionTable::~SectionTable()
{
    for (auto s : sections)
        delete s.second;
}
