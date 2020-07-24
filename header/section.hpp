#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include "array.hpp"
using namespace std;

typedef int8_t byte;

struct Section {
    Array bytes;
    unsigned location_counter = 0;
    char access_rights = 0;
};

#endif
