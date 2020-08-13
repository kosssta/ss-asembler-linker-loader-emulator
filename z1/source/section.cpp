#include "../header/section.hpp"
#include "../header/assembler.hpp"
#include "../header/syntaxErrors.hpp"

Section::Section(string name, string rights, unsigned id)
{
    this->name = name;
    this->id = id;
    list<string> elems = Assembler::splitString(rights, "(?:,|[^,\\s]+)");
    unsigned i = 0;

    for (string s : elems)
    {
        if (i & 1)
        {
            if (s[0] != ',')
                throw SyntaxError();
        }
        else
        {
            if (s[0] == ',')
                throw SyntaxError();
            else if (s == "r")
                access_rights |= R;
            else if (s == "w")
                access_rights |= W;
            else if (s == "x")
                access_rights |= X;
            else if (s == "p")
                access_rights |= P;
        }
        ++i;
    }

    if (i != 0 && !(i & 1))
        throw SyntaxError();
}

string Section::getAccessRights() const
{
    string ret = "";

    if (access_rights & R)
        ret += "R";
    if (access_rights & W)
        ret += "W";
    if (access_rights & X)
        ret += "X";
    if (access_rights & P)
        ret += "P";

    return ret;
}
