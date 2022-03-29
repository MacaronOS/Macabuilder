#include <iostream>
#include <string>
#include "file.hpp"

extern void print(const std::string& m);

int main(int argc, char** argv) {
    if (argc < 0) [[unlikely]] {
        std::cout << "less";
    }

    std::cout << "Hello world" << std::endl;
    print("library reference");

    std::string a;
    std::cin >> a;
    std::cout << a << std::endl;
}