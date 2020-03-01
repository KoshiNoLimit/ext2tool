cmake_minimum_required(VERSION 3.15)
project(untitled1)

set(CMAKE_CXX_STANDARD 17)

add_executable(untitled1 main.cpp config.h structures.h CLI/CLI.hpp CLI11-master/include/CLI/CLI.hpp)