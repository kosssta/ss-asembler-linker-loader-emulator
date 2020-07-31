#include "uncalculatedSymbolsTable.hpp"
#include "assembler.hpp"
#include "section.hpp"
#include "syntaxErrors.hpp"
#include <iomanip>
using namespace std;

UncalculatedSymbolsTable::Symbol::Symbol(string name, string expression) : name(name), expression(expression), value(0)
{
    list<string> elems = Assembler::splitString(expression, "(?:\\+|-|[^-\\+\\s]+)");
    bool minusSign = false;
    for (string e : elems)
    {
        if (e == "+")
            minusSign = false;
        else if (e == "-")
            minusSign = true;
        else if (!Assembler::isLiteral(e))
            symbols.push_front((minusSign ? "-" : "+") + e);
    }
}

UncalculatedSymbolsTable::Symbol *UncalculatedSymbolsTable::get(string name)
{
    auto ret = symbols.find(name);
    return ret == symbols.end() ? nullptr : ret->second;
}

bool UncalculatedSymbolsTable::Symbol::calculateValue(SymbolTable *symbTable, RelocationTable *relTable)
{
    if (expression == "")
        return false;

    bool ret = true;

    string updated_expresion = "";
    bool minusSign = false;
    unsigned i = 0;

    list<string> elems = Assembler::splitString(expression, "(?:\\+|-|[^-\\+\\s]+)");
    for (string s : elems)
    {
        if (s == "+" || s == "-")
        {
            if ((i & 1) || i++ == 0)
                minusSign = s == "-";
            else
                throw SyntaxError();
        }
        else
        {
            if (i & 1)
                throw SyntaxError();
            else
            {
                bool status;
                value += parseOperand(s, symbTable, relTable, minusSign, &status) * (minusSign ? -1 : 1);
                if (!status && !symbTable->getExternSymbol(s))
                {
                    ret = false;
                    updated_expresion.append((minusSign ? "-" : "+") + s);
                }
            }
        }

        ++i;
    }

    if (!(i & 1))
        throw SyntaxError();

    expression = updated_expresion;
    return ret;
}

word UncalculatedSymbolsTable::Symbol::parseOperand(string operand, SymbolTable *symbTable, RelocationTable *relTable, bool minusSign, bool *status)
{
    word number;
    if (Assembler::isLiteral(operand))
    {
        number = Assembler::parseInt(operand);
        if (status)
            *status = true;
    }
    else
    {
        SymbolTable::Symbol *symb = symbTable->getSymbol(operand);
        if (symb && symb->defined)
        {
            number = symb->value;
            if (status)
                *status = true;
        }
        else
        {
            number = 0;
            if (!symb)
                symbTable->insertSymbol(operand, false);
            if (status)
                *status = false;
        }
    }

    return number;
}

void UncalculatedSymbolsTable::add(string name, string expression, Section *section)
{
    if (get(name))
        throw SymbolRedefinitionError(name);

    Symbol *symbol = new Symbol(name, expression);
    bool calculated = symbol->calculateValue(symbTable, relTable);

    symbTable->insertSymbol(name, calculated, symbol->value, section);
    auto symb = symbTable->getSymbol(name);
    symb->section = section;
    
    symbols[name] = symbol;
}

void UncalculatedSymbolsTable::write(ofstream &output) const
{
    output << "=== Uncalculated symbols ===" << endl;
    if (symbols.empty())
    {
        output << "No symbols" << endl;
        return;
    }

    output << left << setw(10) << setfill(' ') << "Name";
    output << left << setw(12) << setfill(' ') << "Expression";
    output << left << setw(8) << setfill(' ') << "Value";
    output << endl;

    for (auto symb : symbols)
    {
        Symbol *s = symb.second;
        if (s->expression != "")
        {
            output << left << setw(10) << setfill(' ') << s->name;
            output << left << setw(12) << setfill(' ') << s->expression;
            output << left << setw(8) << setfill(' ') << s->value;
            output << endl;
        }
    }
}

bool UncalculatedSymbolsTable::calculateAll()
{
    bool change;
    do
    {
        change = false;
        for (auto sym : symbols)
        {
            Symbol *s = sym.second;
            bool calculated = s->calculateValue(symbTable, relTable);
            SymbolTable::Symbol *symb = symbTable->getSymbol(s->name);
            if (symb)
            {
                symb->defined |= calculated;
                symb->value = s->value;
                if (calculated)
                    symb->clearFLink();
            }
            change |= calculated;
        }
    } while (change);

    relTable->add(symbols);

    bool ret = true;
    for (auto symb : symbols)
    {
        Symbol *s = symb.second;
        if (s->expression != "")
            ret = false;
        delete s;
    }
    symbols.clear();

    return ret;
}

UncalculatedSymbolsTable::~UncalculatedSymbolsTable()
{
    for (auto s : symbols)
        delete s.second;
}
