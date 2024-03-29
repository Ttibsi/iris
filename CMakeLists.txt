cmake_minimum_required(VERSION 3.22.0)
if(APPLE)
    set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_COMPILER "/usr/bin/g++-13")
endif()

project(iris LANGUAGES CXX VERSION 0.1.0)
option(RUN_TESTS "Run unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall)
add_compile_options(-Wextra)

add_subdirectory(src)

if(RUN_TESTS)
    enable_testing()
    message(STATUS "UNIT TESTS ENABLED")
    add_subdirectory(tests)
endif()
