#include "Lexer/Lexer.h"

#include <fstream>
#include <iostream>

int main()
{
    std::ifstream myfile("example.bgn");
    if (myfile.is_open()) {
        auto l = Lexer(myfile);
        l.run();
    } else {
        std::cout << "Unable to open file";
    }
    return 0;
}
