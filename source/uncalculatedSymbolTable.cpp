#include "uncalculatedSymbolsTable.hpp"
#include "assembler.hpp"
#include "syntaxError.hpp"
#include "section.hpp"
#include "symbolAlreadyDefinedError.hpp"
#include <regex>
using namespace std;

bool UncalculatedSymbolsTable::Symbol::calculateValue(SymbolTable *symbTable)
{
    if(expression == "") return false;
    
    bool ret = true;

    string updated_expresion = "";
    bool minusSign = false;
    unsigned i = 0;

    vector<string> elems = Assembler::splitString(expression, "(?:\\+|-|[^-\\+\\s]+)");
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
                value += parseOperand(s, symbTable, &status) * (minusSign ? -1 : 1);
                if (!status)
                {
                    ret = false;
                    updated_expresion.append((minusSign ? "-" : "+") + s);
                }
            }
        }

        ++i;
    }

    expression = updated_expresion;
    return ret;
}

word UncalculatedSymbolsTable::Symbol::parseOperand(string operand, SymbolTable *symbTable, bool *status)
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
    Symbol *symbol = new Symbol(name, expression, section);
    bool calculated = symbol->calculateValue(symbTable);

    symbTable->insertSymbol(name, calculated, symbol->value, section);

    if (calculated)
        delete symbol;
    else
        symbols.push_back(symbol);
}

void UncalculatedSymbolsTable::write() const
{
    cout << "Uncalculated symbols:" << endl;
    cout << "Name\tExpression\tValue\tSection\n";

    for (Symbol *s : symbols)
    {
        cout << s->name << '\t' << s->expression << "\t\t" << s->value << '\t' << ((long)s->section & 0xffff) << endl;
    }
}

bool UncalculatedSymbolsTable::calculateAll()
{
    bool change;
    do
    {
        change = false;
        for (Symbol *s : symbols)
        {
            bool calculated = s->calculateValue(symbTable);
            SymbolTable::Symbol *symb = symbTable->getSymbol(s->name);
            if (symb)
            {
                symb->section = s->section;
                symb->defined = calculated;
                symb->value = s->value;
                symb->clearFLink();
            }
            change |= calculated;
        }
    } while (change);

    for (Symbol *s : symbols)
    {
        if (s->expression != "")
            return false;
    }

    return true;
}
