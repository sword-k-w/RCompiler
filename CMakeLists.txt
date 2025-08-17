cmake_minimum_required(VERSION 3.22)

project(RCompiler CXX)

find_package(GTest REQUIRED)

set(CMAKE_CXX_STANDARD 20)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(src/include)

set(LEXER_DIR ./src/lexer)
set(PARSER_DIR ./src/parser)
set(COMMON_DIR ./src/common)

file(GLOB_RECURSE PARSER_SRCS
    "${PARSER_DIR}/*.cpp")
aux_source_directory(${COMMON_DIR} COMMON_SRCS)
aux_source_directory(${LEXER_DIR} LEXER_SRCS)

add_executable(parser_test
        test/parser_test.cpp
        src/data_loader/data_loader.cpp
        ${PARSER_SRCS}
        ${COMMON_SRCS}
        ${LEXER_SRCS}
)

target_link_libraries(parser_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

add_test(NAME parser_test COMMAND parser_test)