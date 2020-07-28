#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>
using namespace std;

typedef int8_t byte;

struct Section {
    vector<byte> bytes;
    char access_rights = 0;
};

#endif
