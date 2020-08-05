#include "../header/terminal.hpp"
#include <iostream>
using namespace std;

Terminal::Terminal(uword *input, uword *output, bool *interrupt_input, bool *interrupt_output)
{
    this->input = input;
    this->output = output;
    this->interrupt_input = interrupt_input;
    this->interrupt_output = interrupt_output;
    terminal_input = new thread(&Terminal::input_func, this);
    terminal_output = new thread(&Terminal::output_func, this);
}

Terminal::~Terminal()
{
    if (!end)
        terminate();
}

void Terminal::input_func()
{
    while (true) {
        *input = getchar();
        *interrupt_input = true;
    }
}

void Terminal::output_func()
{
}

void Terminal::terminate()
{
    end = true;
    //  terminal_input->join();
    terminal_output->join();
    delete terminal_input;
    delete terminal_output;
    terminal_input = nullptr;
    terminal_output = nullptr;
}
