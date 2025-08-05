cmake_minimum_required(VERSION 3.22)
find_package(GTest REQUIRED)

project(RCompiler)

set(CMAKE_CXX_STANDARD 14)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(src/include)
