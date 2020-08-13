#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>
using namespace std;

typedef int8_t byte;

struct Section {
    enum ACCESS_RIGHTS { P = 1, R = 2, W = 4, X = 8 };

    string name;
    unsigned id;
    vector<byte> bytes;
    char access_rights = 0;

    Section(string name, string rights, unsigned id = 0);
    string getAccessRights() const;
};

#endif
