#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include "array.hpp"
using namespace std;

typedef char byte;

struct Section {
    Array bytes;
    char access_rights = 0;
};

#endif
