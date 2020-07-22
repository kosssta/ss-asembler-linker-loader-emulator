#include <exception>
#include <string>
using namespace std;

class SyntaxError : public exception
{
protected:
    unsigned line_number = 0;
    string error_message = "";

public:
    SyntaxError(string error_message, unsigned line_number = 0)
    {
        this->error_message = error_message;
        this->line_number = line_number;
    }

    string getErrorMessage()
    {
        string message = "";
        if (line_number != 0)
        {
            message += "Line " + line_number;
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
