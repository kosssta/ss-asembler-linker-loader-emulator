#include "../header/sectionTable.hpp"
#include "../header/section.hpp"

Section *SectionTable::addSection(Section *section)
{
    if (!section)
        return nullptr;

    Section *s = findSection(section->name);
    if (s) {
        delete section;
        return s;
    }

    s = new Section(section->name, section->id, section->access_rights);
    s->bytes = section->bytes;
    sections[section->name] = s;
    return s;
}

Section *SectionTable::findSection(string name) const
{
    auto s = sections.find(name);
    return s == sections.end() ? nullptr : s->second;
}
