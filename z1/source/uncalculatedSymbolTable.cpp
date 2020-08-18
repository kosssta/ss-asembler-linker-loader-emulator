#include "../header/uncalculatedSymbolsTable.hpp"
#include "../header/assembler.hpp"
#include "../header/section.hpp"
#include "../header/syntaxErrors.hpp"
#include <set>
using namespace std;

UncalculatedSymbolsTable::Symbol::Symbol(string name, string expression, SymbolTable *symbTable) : name(name)
{
    list<string> elems = Assembler::splitString(expression, "(?:\\+|-|[^-\\+\\s]+)");
    bool minusSign = false;
    unsigned i = 0;
    SymbolTable::Symbol *symbol = symbTable->getSymbol(name);

    for (string s : elems)
    {
        if (s == "+" || s == "-")
        {
            if ((i & 1) || i++ == 0)
                minusSign = s == "-";
            else
                throw SyntaxError(".equ: Invalid expression for symbol " + name);
        }
        else
        {
            if (i & 1)
                throw SyntaxError(".equ: Invalid expression for symbol " + name);
            else if (Assembler::isLiteral(s))
                symbol->value += Assembler::parseInt(s);
            else
                symbols.push_back(make_pair(!minusSign, s));
        }
        ++i;
    }

    if (!(i & 1))
        throw SyntaxError(".equ: Invalid expression for symbol " + name);
    if (symbols.size() == 0)
        symbol->defined = true;
}

UncalculatedSymbolsTable::Symbol *UncalculatedSymbolsTable::get(string name)
{
    auto ret = symbols.find(name);
    return ret == symbols.end() ? nullptr : ret->second;
}

void UncalculatedSymbolsTable::Symbol::calculateValue(UncalculatedSymbolsTable *uTable, SymbolTable *symbTable, RelocationTable *relTable)
{
    if (symbols.size() == 0)
        return;

    set<string> used_equ_symbols;
    bool extern_found = false;
    list<pair<bool, string>> to_remove;
    used_equ_symbols.insert(name);

    for (auto s : symbols)
    {
        bool defined = parseOperand(s.second, symbTable, !s.first);
        if (!defined)
        {
            if (symbTable->getExternSymbol(s.second)) {
                index.add(nullptr, true);
                extern_found = true;
            }
            else
            {
                Symbol *symbol = uTable->get(s.second);
                if (symbol)
                {
                    if (used_equ_symbols.find(s.second) != used_equ_symbols.end())
                        throw SyntaxError(".equ: Loop detected in the definition of symbol " + name);

                    used_equ_symbols.insert(s.second);
                    SymbolTable::Symbol *symb = symbTable->getSymbol(name);
                    SymbolTable::Symbol *symb2 = symbTable->getSymbol(name);
                    symb->value += symb2->value;
                    for (auto newSymbol : symbol->symbols)
                        symbols.push_back(make_pair(s.first ? newSymbol.first : !newSymbol.first, newSymbol.second));
                    to_remove.push_back(s);
                }
                else
                    throw SyntaxError("Undefined symbol: " + s.second);
            }
        }
    }

    for (auto t : to_remove)
        symbols.remove(t);

    checkIndex(symbTable);
    SymbolTable::Symbol *symbol = symbTable->getSymbol(name);
    symbol->defined = !extern_found;
}

bool UncalculatedSymbolsTable::Symbol::parseOperand(string operand, SymbolTable *symbTable, bool minusSign)
{
    word number;
    bool defined = true;

    if (Assembler::isLiteral(operand))
        number = Assembler::parseInt(operand);
    else
    {
        SymbolTable::Symbol *symb = symbTable->getSymbol(operand);
        if (symb && symb->defined)
        {
            number = symb->value;
            if (symb->section)
                index.add(symb->section, !minusSign);
        }
        else
        {
            number = 0;
            if (!symb)
                symbTable->insertSymbol(operand, false);
            defined = false;
        }
    }

    if (number != 0)
    {
        SymbolTable::Symbol *symbol = symbTable->getSymbol(name);
        if (symbol)
            symbol->value += number * (minusSign ? -1 : 1);
    }

    return defined;
}

void UncalculatedSymbolsTable::add(string name, string expression)
{
    if (get(name))
        throw SymbolRedefinitionError(name);

    symbTable->insertSymbol(name, false);
    Symbol *symbol = new Symbol(name, expression, symbTable);
    symbols[name] = symbol;
}

void UncalculatedSymbolsTable::calculateAll()
{
    for (auto s : symbols)
        s.second->calculateValue(this, symbTable, relTable);
    for (auto s : symbols) {
        SymbolTable:: Symbol *symb = symbTable->getSymbol(s.second->name);
        symb->defined = true;
    }
    
    relTable->add(symbols);
}

UncalculatedSymbolsTable::~UncalculatedSymbolsTable()
{
    for (auto s : symbols)
    {
        cout << "Symbol " + s.first << endl;
        for (auto ind : s.second->index.index)
        {
            cout << (ind.first ? ind.first->name : "UND") << ' ' << ind.second << endl;
        }
        cout << endl;
        delete s.second;
    }
}

void UncalculatedSymbolsTable::RelocationIndex::add(Section *section, bool plus)
{
    auto ind = index.find(section);
    if (ind == index.end())
        index[section] = 0;
    index[section] += 1 * (plus ? 1 : -1);
}

void UncalculatedSymbolsTable::Symbol::checkIndex(SymbolTable *symbTable)
{
    Section *sec = nullptr;
    for (auto ind : index.index)
    {
        if (ind.first && ind.second != 0)
            if (ind.second == 1 && !sec)
                sec = ind.first;
            else
                throw SyntaxError("Incorrect expression for symbol " + name);
    }

    if (sec)
    {
        SymbolTable::Symbol *symbol = symbTable->getSymbol(name);
        if (symbol)
            symbol->section = sec;
    }
}
