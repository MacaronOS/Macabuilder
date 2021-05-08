cmake_minimum_required(VERSION 3.17)
project(Beegn)

set(CMAKE_CXX_STANDARD 20)

add_executable(Beegn main.cpp Lexer/Lexer.cpp Lexer/Lexer.h Lexer/Token.h Parser/Parser.cpp Parser/Parser.h Context.cpp Context.h Parser/Field/IncludeField.h Parser/Field/DefinesField.h Parser/Field/CommandsField.h Parser/Field/BuildField.h Parser/Field/DefaultField.h Finder/Finder.h Executor/Executor.cpp Executor/Executor.h Executor/Command.cpp Executor/Command.h Logger.h utils/Utils.h utils/Utils.cpp utils/Utils.h Executor/ExecutableUnit.h Finder/Finder.cpp utils/ThreadQueue.h utils/Lock.h examples/wisteria/wisterialib/library.cpp)

file(
        COPY ${CMAKE_CURRENT_BASE_DIR}examples
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
)