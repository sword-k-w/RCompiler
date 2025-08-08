cmake_minimum_required(VERSION 3.22)
#find_package(GTest REQUIRED)

project(RCompiler)

set(CMAKE_CXX_STANDARD 20)

#include_directories(${GTEST_INCLUDE_DIRS})
include_directories(src/include)
add_executable(lexer_test
        src/data_loader/data_loader.cpp
        src/lexer/lexer.cpp
        src/lexer/single.cpp
        src/lexer/intermediate.cpp
        src/lexer/meaningless.cpp
        src/lexer/token.cpp
        src/lexer/lexer_test.cpp
)

add_executable(parser_test
        src/parser/parser_test.cpp)