#include "../header/relocationTable.hpp"

RelocationTable::Record* RelocationTable::add(unsigned symbol, unsigned offset, RelocationTable::RelocationType type, bool plus) {
    records.push_front(Record(symbol, offset, type, plus));
    return &records.front();
}

bool RelocationTable::operator()(const RelocationTable::Record &r) {
    return r.symbol == 0;
}