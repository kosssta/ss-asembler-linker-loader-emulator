#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>
using namespace std;

typedef int8_t byte;

struct Section {
    string name;
    unsigned id;
    vector<byte> bytes;
    char access_rights = 0;

    Section(string name, unsigned id = 0) : name(name), id(id) {}
};

#endif
