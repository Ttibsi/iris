cmake_minimum_required(VERSION 3.22.0)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-18")

project(iris LANGUAGES CXX)
option(RUN_TESTS "Run unit tests" OFF)
option(ENABLE_COVERAGE "Generate coverage report" OFF)
option(ENABLE_ASAN "Compile with AddressSanitizer" OFF)
option(RELEASE "Release build" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(RELEASE)
    set(CMAKE_BUILD_TYPE Release)
else()
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall)
add_compile_options(-Wextra)

if(ENABLE_ASAN)
    add_compile_options(-g)
    add_compile_options(-fsanitize=address)
endif()

add_subdirectory(src)

if(RUN_TESTS)
    enable_testing()
    message(STATUS "UNIT TESTS ENABLED")
    add_subdirectory(tests)
endif()
