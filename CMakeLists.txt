cmake_minimum_required(VERSION 3.22)

project(RCompiler CXX)

enable_testing()

find_package(GTest REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(src/include)

set(LEXER_DIR ./src/lexer)
set(PARSER_DIR ./src/parser)
set(COMMON_DIR ./src/common)
set(VISITOR_DIR ./src/visitor)
set(SEMANTIC_DIR ./src/semantic)
set(IR_DIR ./src/IR)
set(IR_VISITOR_DIR ./src/IR_visitor)

file(GLOB_RECURSE VISITOR_SRCS
    "${VISITOR_DIR}/*.cpp")
file(GLOB_RECURSE PARSER_SRCS
    "${PARSER_DIR}/*.cpp")
file(GLOB_RECURSE SEMANTIC_SRCS
    "${SEMANTIC_DIR}/*.cpp")
file(GLOB_RECURSE IR_SRCS
    "${IR_DIR}/*.cpp")
file(GLOB_RECURSE IR_VISITOR_SRCS
        "${IR_VISITOR_DIR}/*.cpp")

aux_source_directory(${COMMON_DIR} COMMON_SRCS)
aux_source_directory(${LEXER_DIR} LEXER_SRCS)

add_executable(code
        test/IR_submit.cpp
        src/data_loader/data_loader.cpp
        ${PARSER_SRCS}
        ${COMMON_SRCS}
        ${LEXER_SRCS}
        ${VISITOR_SRCS}
        ${SEMANTIC_SRCS}
        ${IR_SRCS}
        ${IR_VISITOR_SRCS})

add_custom_target(build
        DEPENDS code
        COMMENT "Compiling project..."
)

add_custom_target(run
        DEPENDS build
        COMMAND ./code
        COMMENT "Running myapp..."
)
#add_executable(parser_test
#        test/parser_test.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS}
#)
#
#add_executable(semantic_test
#        test/semantic_test.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS}
#)
#
#add_executable(IR_test
#        test/IR_test.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS})
#
#target_link_libraries(parser_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#target_link_libraries(semantic_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#target_link_libraries(IR_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

add_test(NAME parser_test COMMAND parser_test)
add_test(NAME semantic_test COMMAND semantic_test)
add_test(NAME IR_test COMMAND IR_test)