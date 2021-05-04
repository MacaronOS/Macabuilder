#pragma once

#include <iostream>

enum class Color {
    Red = 31,
    Green = 32,
    Blue = 34,
};

template <class... Types>
void Log(Color color, Types... args)
{
    std::cout << "\033[1;" << static_cast<uint32_t>(color) << "m";
    auto print_args = { (std::cout << args << " ", 0)... };
    std::cout << "\033[0m\n";
}
