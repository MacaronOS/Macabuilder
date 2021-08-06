cmake_minimum_required(VERSION 3.17)
project(Beelder)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-O3")

add_executable(Beelder main.cpp Lexer/Lexer.cpp Lexer/Lexer.h Lexer/Token.h Parser/Parser.cpp Parser/Parser.h Context.cpp Context.h Parser/Field/IncludeField.h Parser/Field/DefinesField.h Parser/Field/CommandsField.h Parser/Field/BuildField.h Parser/Field/DefaultField.h Finder/Finder.h Executor/Executor.cpp Executor/Executor.h Executor/Command.cpp Executor/Command.h utils/Logger.h utils/Utils.h utils/Utils.cpp utils/Utils.h Executor/ExecutableUnit.h Finder/Finder.cpp utils/ThreadQueue.h utils/Lock.h examples/wisteria/wisterialib/library.cpp Config.cpp Config.h Translator/Translator.cpp Translator/Translator.h)

file(
        COPY ${CMAKE_CURRENT_BASE_DIR}examples/wisteria/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
)