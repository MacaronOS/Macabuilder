cmake_minimum_required(VERSION 3.17)
project(Beegn)

set(CMAKE_CXX_STANDARD 20)

add_executable(Beegn main.cpp Lexer/Lexer.cpp Lexer/Lexer.h Lexer/Token.cpp Lexer/Token.h)