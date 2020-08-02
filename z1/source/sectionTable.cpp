#include "../header/sectionTable.hpp"
#include "../header/section.hpp"
#include "../header/relocationTable.hpp"
#include <list>
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
    auto ret = sections.find(name.substr(1));
    return ret == sections.end() ? nullptr : ret->second;
}

void SectionTable::write(ofstream &output) const
{
    output << "=== Sections ===" << endl;
    if (sections.empty())
    {
        output << "No sections" << endl;
        return;
    }

    list<Section> secs;

    for (auto s : sections)
    {
        secs.push_back(*s.second);
    }

    secs.sort();
    for (Section &s : secs)
    {
        output << s.name << ':' << endl;
        output << "Length = " << s.bytes.size() << endl;
        for (byte b : s.bytes)
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

forward_list<string> SectionTable::getAllNames() const
{
    forward_list<string> ret;
    for (auto s : sections)
        ret.push_front(s.second->name);
    return ret;
}

unsigned SectionTable::writeBinary(ofstream &output, RelocationTable *relTable) const
{
    struct SectionBinary
    {
        unsigned nameLength;
        unsigned id;
        unsigned length;
        char access_rights;
        unsigned num_relocations;
    };

    list<Section> secs;

    for (auto s : sections)
    {
        secs.push_back(*s.second);
    }

    secs.sort();

    SectionBinary sb;
    for (auto sec : secs)
    {
        sb.nameLength = sec.name.size();
        sb.id = sec.id;
        sb.access_rights = sec.access_rights;
        sb.length = sec.bytes.size();

        unsigned tmp = output.tellp();
        output.write((char *)&sb, sizeof(sb));
        output << sec.name;
        output.write((char *)&sec.bytes[0], sec.bytes.size() * sizeof(sec.bytes[0]));
        sb.num_relocations = relTable->writeBinary(output, &sec);
        unsigned tmp2 = output.tellp();
        output.seekp(tmp);
        output.write((char *)&sb, sizeof(sb));
        output.seekp(tmp2);
    }
    return sections.size();
}

bool operator<(const Section &s1, const Section &s2)
{
    return s1.id < s2.id;
}
