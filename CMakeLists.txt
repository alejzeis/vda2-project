cmake_minimum_required(VERSION 3.24)

project(PA3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(PA3
    src/suraj_parser.cpp
    src/main.cpp
    src/matrix.cpp
    src/placer.cpp
)
target_include_directories(PA3 PRIVATE include)

