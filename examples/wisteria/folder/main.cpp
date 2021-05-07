#include <iostream>

int main(int argc, char** argv) {
    if (argc < 0) [[unlikely]] {
        std::cout << "less";
    }
    std::cout << "Hello world" << std::endl;
}