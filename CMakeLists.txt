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
set(CODEGEN_DIR ./src/codegen)
set(LIVENESS_ANALYSIS_DIR ./src/liveness_analysis)
set(MEM2REG_DIR ./src/mem2reg)
set(REG_ALLOC_DIR ./src/reg_alloc)

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
file(GLOB_RECURSE CODEGEN_SRCS
    "${CODEGEN_DIR}/*.cpp")
file(GLOB_RECURSE LIVENESS_ANALYSIS_SRCS
    "${LIVENESS_ANALYSIS_DIR}/*.cpp")
file(GLOB_RECURSE MEM2REG_SRCS
    "${MEM2REG_DIR}/*.cpp")
file(GLOB_RECURSE REG_ALLOC_SRCS
    "${REG_ALLOC_DIR}/*.cpp")


aux_source_directory(${COMMON_DIR} COMMON_SRCS)
aux_source_directory(${LEXER_DIR} LEXER_SRCS)

#add_executable(code
#        test/IR_submit.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS})


add_executable(code
        test/codegen_submit.cpp
        src/data_loader/data_loader.cpp
        ${PARSER_SRCS}
        ${COMMON_SRCS}
        ${LEXER_SRCS}
        ${VISITOR_SRCS}
        ${SEMANTIC_SRCS}
        ${IR_SRCS}
        ${IR_VISITOR_SRCS}
        ${CODEGEN_SRCS}
        ${LIVENESS_ANALYSIS_SRCS}
        ${MEM2REG_SRCS}
        ${REG_ALLOC_SRCS})

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
#        ${CODEGEN_SRCS}
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
#        ${CODEGEN_SRCS}
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
#        ${IR_VISITOR_SRCS}
#        ${CODEGEN_SRCS}
#)

#add_executable(IR_single
#        test/IR_single.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS}
#        ${CODEGEN_SRCS}
#        ${LIVENESS_ANALYSIS_SRCS}
#        ${MEM2REG_SRCS}
#        ${REG_ALLOC_SRCS}
#)
#
#add_executable(IR_single_optimized
#        test/IR_single_optimized.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS}
#        ${CODEGEN_SRCS}
#        ${LIVENESS_ANALYSIS_SRCS}
#        ${MEM2REG_SRCS}
#        ${REG_ALLOC_SRCS}
#)
#
#add_executable(codegen_test
#        test/codegen_test.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS}
#        ${CODEGEN_SRCS}
#        ${LIVENESS_ANALYSIS_SRCS}
#        ${MEM2REG_SRCS}
#        ${REG_ALLOC_SRCS}
#)
#
#add_executable(codegen_single
#        test/codegen_single.cpp
#        src/data_loader/data_loader.cpp
#        ${PARSER_SRCS}
#        ${COMMON_SRCS}
#        ${LEXER_SRCS}
#        ${VISITOR_SRCS}
#        ${SEMANTIC_SRCS}
#        ${IR_SRCS}
#        ${IR_VISITOR_SRCS}
#        ${CODEGEN_SRCS}
#        ${LIVENESS_ANALYSIS_SRCS}
#        ${MEM2REG_SRCS}
#        ${REG_ALLOC_SRCS}
#)
#
#add_executable(dominator
#        test/dominator.cpp
#        src/data_loader/data_loader.cpp
#        src/liveness_analysis/dominator_tree.cpp)
#
#add_executable(reg_alloc_test
#        test/reg_alloc_test.cpp
#        src/reg_alloc/interference_graph.cpp)
#
#target_link_libraries(parser_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#target_link_libraries(semantic_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#target_link_libraries(IR_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#target_link_libraries(codegen_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#target_link_libraries(reg_alloc_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#
#add_test(NAME parser_test COMMAND parser_test)
#add_test(NAME semantic_test COMMAND semantic_test)
#add_test(NAME IR_test COMMAND IR_test)
#add_test(NAME codegen_test COMMAND codegen_test)
#add_test(NAME reg_alloc_test COMMAND reg_alloc_test)