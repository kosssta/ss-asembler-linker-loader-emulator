#ifndef RELOCATION_TABLE_H_
#define RELOCATION_TABLE_H_

#include <forward_list>
#include <unordered_map>
using namespace std;

class RelocationTable
{
public:
    enum RelocationType
    {
        R_X86_64_PC16,
        R_X86_64_16,
        R_X86_64_8
    };
    struct Record
    {
        unsigned symbol;
        unsigned offset;
        RelocationType type;
        bool plus;

        Record(unsigned symbol, unsigned offset, RelocationType type, bool plus) : symbol(symbol), offset(offset), type(type), plus(plus) {}
    };

    Record* add(unsigned symbol, unsigned offset, RelocationType type, bool plus = true);
    bool operator()(const Record &r);

forward_list<RelocationTable::Record> records;
private:
    
};

#endif
