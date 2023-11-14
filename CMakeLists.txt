cmake_minimum_required(VERSION 3.22.0)
if (${APPLE})
    set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
endif()

project(iris LANGUAGES CXX VERSION 0.1.0)
option(RUN_TESTS "Run unit tests" ON)
option(DBG "Build with debug symbols" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(DBG)
    message(STATUS "DEBUG SYMBOLS ENABLED")
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall)
add_compile_options(-Wextra)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_subdirectory(src)

if(RUN_TESTS)
    enable_testing()
    message(STATUS "UNIT TESTS ENABLED")
    add_subdirectory(tests)
endif()
