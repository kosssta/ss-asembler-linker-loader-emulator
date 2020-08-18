#include "../header/terminal.hpp"
#include <iostream>
#include <unistd.h>
#include <termios.h>
using namespace std;

Terminal::Terminal(uword *input, uword *output, bool *interrupt, Semaphore *outputDone)
{
    this->input = input;
    this->output = output;
    this->interrupt = interrupt;
    this->outputDone = outputDone;
    terminal_input = new thread(&Terminal::input_func, this);
    terminal_output = new thread(&Terminal::output_func, this);
}

Terminal::~Terminal()
{
    if (!end)
        terminate();
}

char Terminal::getch()
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

void Terminal::input_func()
{
    while (true)
    {
        *input = getch();
        if (end)
            break;
        *interrupt = true;
    }
}

void Terminal::output_func()
{
    while (true)
    {
        waitForChar.acquire();
        if (end)
            break;
        cout << (unsigned char)*output;
        cout.flush();
        outputDone->release();
    }
}

void Terminal::terminate()
{
    end = true;
    waitForChar.release();
    cout << "\nPress any key to continue..." << endl;
    terminal_input->join();
    terminal_output->join();
    delete terminal_input;
    delete terminal_output;
    terminal_input = nullptr;
    terminal_output = nullptr;
    outputDone = nullptr;
}
