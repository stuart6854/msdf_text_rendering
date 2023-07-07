#include "font.hpp"

#include <format>
#include <iostream>

template<typename ... Args>
void println(std::string_view fmt, Args&&... args)
{
    std::cout << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << "MSDF Text Rendering\n";

    Font font("fonts/OpenSans-Regular.ttf");

    return 0;
}