cmake_minimum_required(VERSION 3.17)
project(Beegn)

set(CMAKE_CXX_STANDARD 20)

add_executable(Beegn main.cpp Lexer/Lexer.cpp Lexer/Lexer.h Lexer/Token.h Parser/Parser.cpp Parser/Parser.h Context.cpp Context.h Parser/Field/IncludeField.h Parser/Field/DefinesField.h Parser/Field/CommandsField.h Parser/Field/BuildField.h Parser/Field/DefaultField.h)

file(
        COPY ${CMAKE_CURRENT_BASE_DIR}examples
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
)