cmake_minimum_required(VERSION 3.28)

project(tinyb)

set(CMAKE_CXX_STANDARD 20)

add_executable(tinyb src/main.cpp
                     src/lexer.cpp
                     src/parser.cpp)