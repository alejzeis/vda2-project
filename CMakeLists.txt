cmake_minimum_required(VERSION 3.24)

project(PA3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(FASTPLACE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/fastplace)
set(SFQPLACE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/sfqplace)

add_library(fastplace
    ${FASTPLACE_ROOT}/src/suraj_parser.cpp
    ${FASTPLACE_ROOT}/src/matrix.cpp
    ${FASTPLACE_ROOT}/src/placer.cpp
)
target_include_directories(fastplace PRIVATE ${FASTPLACE_ROOT}/include)

#add_executable(sfqplace
#    ${SFQPLACE_ROOT}/main.cpp
#    # TODO: more files
#)
#target_include_directories(sfqplace PRIVATE
#    ${SFQPLACE_ROOT}/include
#    ${FASTPLACE_ROOT}/include
#)
#target_link_libraries(sfqplace fastplace)

