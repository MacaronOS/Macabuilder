cmake_minimum_required(VERSION 3.17)
project(Macabuilder)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-O3 -lpthread")

add_executable(Macabuilder Sources/main.cpp Sources/Parser/Lexer/Lexer.cpp Sources/Parser/Lexer/Lexer.h Sources/Parser/Lexer/Token.h Sources/Parser/Parser.cpp Sources/Parser/Parser.h Sources/Context.cpp Sources/Context.h Sources/Parser/Field/IncludeField.h Sources/Parser/Field/DefinesField.h Sources/Parser/Field/CommandsField.h Sources/Parser/Field/BuildField.h Sources/Parser/Field/DefaultField.h Sources/Finder/Finder.h Sources/Executor/Executor.cpp Sources/Executor/Executor.h Sources/Executor/Command.cpp Sources/Executor/Command.h Sources/Utils/Logger.h Sources/Utils/Utils.h Sources/Utils/Utils.cpp Sources/Utils/Utils.h Sources/Executor/ExecutableUnit.h Sources/Utils/ThreadQueue.h Sources/Utils/Lock.h Examples/wisteria/wisterialib/library.cpp Sources/Config.cpp Sources/Config.h Sources/Translator/Translator.cpp Sources/Translator/Translator.h Sources/Finder/Glob.h)

file(
        COPY ${CMAKE_CURRENT_BASE_DIR}Examples/wisteria/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
)