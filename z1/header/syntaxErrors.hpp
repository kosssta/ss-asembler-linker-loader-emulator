#ifndef SYNTAX_ERROR_H_
#define SYNTAX_ERROR_H_

#include <exception>
#include <string>
using namespace std;

class SyntaxError : public exception
{
protected:
    unsigned line_number = 0;
    string error_message = "";

public:
    SyntaxError(string error_message = "Syntax error", unsigned line_number = 0)
    {
        this->error_message = error_message;
        this->line_number = line_number;
    }

    string getErrorMessage()
    {
        string message = "";
        if (line_number != 0)
        {
            message += "Line ";

            // converting line_number to ASCII
            string number = "";
            do
            {
                number += line_number % 10 + '0';
                line_number /= 10;
            } while (line_number > 0);

            for (int i = number.length() - 1; i >= 0; --i)
            {
                message += number[i];
            }
            message += ": ";
        }
        message += error_message;
        return message;
    }

    void setLineNumber(unsigned line_number)
    {
        this->line_number = line_number;
    }

    void setErrorMessage(string error_messsage)
    {
        this->error_message = error_message;
    }
};

class SymbolRedefinitionError : public SyntaxError
{
public:
    SymbolRedefinitionError(string symbol_name) : SyntaxError("Symbol " + symbol_name + " is already defined") {}
};

class UnrecognizedSymbol : public SyntaxError
{
public:
    UnrecognizedSymbol(string symbol_name) : SyntaxError("Unrecognized symbol " + symbol_name) {}
};

#endif
