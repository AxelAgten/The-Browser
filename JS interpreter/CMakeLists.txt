cmake_minimum_required(VERSION 4.1)
project(JS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

configure_file(
        ${CMAKE_SOURCE_DIR}/test/test.js
        ${CMAKE_BINARY_DIR}/test.js
        COPYONLY
)

add_executable(JS
        lexer.cpp
        lexer.h
        main.cpp
        parser.cpp
        parser.h
        interpreter.cpp
        Interpreter.h)
