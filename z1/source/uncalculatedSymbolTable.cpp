#include "../header/uncalculatedSymbolsTable.hpp"
#include "../header/assembler.hpp"
#include "../header/section.hpp"
#include "../header/syntaxErrors.hpp"
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
            symbols.push_front(make_pair(!minusSign, e));
    }
}

UncalculatedSymbolsTable::Symbol *UncalculatedSymbolsTable::get(string name)
{
    auto ret = symbols.find(name);
    return ret == symbols.end() ? nullptr : ret->second;
}

bool UncalculatedSymbolsTable::Symbol::calculateValue(UncalculatedSymbolsTable *uTable, SymbolTable *symbTable, RelocationTable *relTable)
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
                if (!status)
                {
                    if (symbTable->getExternSymbol(s) || uTable->get(s) && uTable->get(s)->extern_)
                    {
                        extern_ = true;
                        index.add(nullptr, true);
                    }
                    else
                    {
                        ret = false;
                        updated_expresion.append((minusSign ? "-" : "+") + s);
                    }
                }
                else
                {
                    if (uTable->get(s))
                    {
                    /*    if (uTable->get(s)->extern_)
                        {
                            SymbolTable::Symbol *symb = symbTable->getSymbol(s);
                            index.add(nullptr, true);
                            if (symb->section)
                                index.add(symb->section, !minusSign);
                        }
                    */}
                    else if (symbTable->getSymbol(s) && symbTable->getSymbol(s)->section)
                    {
                        index.add(symbTable->getSymbol(s)->section, !minusSign);
                        symbols.remove(make_pair(!minusSign, s));
                    }
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

void UncalculatedSymbolsTable::add(string name, string expression)
{
    if (get(name))
        throw SymbolRedefinitionError(name);

    Symbol *symbol = new Symbol(name, expression);
    bool calculated = symbol->calculateValue(this, symbTable, relTable);

    symbTable->insertSymbol(name, calculated, symbol->value);
    auto symb = symbTable->getSymbol(name);
    if (calculated)
        symbol->checkIndex(symbTable);
    else
        symb->section = nullptr;

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
            bool calculated = s->calculateValue(this, symbTable, relTable);
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

    list<pair<bool, string>> to_remove;

    do
    {
        change = false;
        for (auto symb : symbols)
        {
            Symbol *s = symb.second;
            for (auto symb : s->symbols)
            {
                Symbol *symbol = get(symb.second);
                if (symbol)
                {
                    change = true;
                    to_remove.push_back(symb);
                    s->value += symbol->value;

                    SymbolTable::Symbol *sym = symbTable->getSymbol(symbol->name);
                    if (sym->section)
                        s->index.add(sym->section, symb.first);
                    s->index.add(sym->section, symb.first);
                    for (auto sy : symbol->symbols)
                    {
                        s->symbols.push_back(sy);
                    }
                }
            }
            for (auto r : to_remove)
                s->symbols.remove(r);
            to_remove.clear();
        }

    } while (change);

    bool ret = true;
    for (auto symb : symbols)
    {
        Symbol *s = symb.second;
        if (s->expression != "")
            ret = false;
        else
            s->checkIndex(symbTable);

        //     delete s;
    }

    relTable->add(symbols);

    // symbols.clear();

    return ret;
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
