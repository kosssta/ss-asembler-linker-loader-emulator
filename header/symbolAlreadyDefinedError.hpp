#ifndef SYMBOLALREADYDEFINEDERROR_H_
#define SYMBOLALREADYDEFINEDERROR_H_

#include "syntaxError.hpp"

class SymbolAlreadyDefinedError : public SyntaxError {
    public:
    SymbolAlreadyDefinedError(string symbol_name) : SyntaxError("Symbol " + symbol_name + " already defined") {}
};

#endif
