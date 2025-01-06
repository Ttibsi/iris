cmake_minimum_required(VERSION 3.22.0)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_COMPILER "/usr/bin/clang")

project(iris LANGUAGES CXX)
option(RUN_TESTS "Run unit tests" OFF)
option(ENABLE_ASAN "Compile with AddressSanitizer" OFF)
option(RELEASE "Release build" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(RELEASE)
    set(CMAKE_BUILD_TYPE Release)
else()
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-g)

# TODO: When I'm close to release-1, enable static linkage
if(ENABLE_ASAN)
    add_compile_options(-g)
    add_compile_options(-fsanitize=address)
endif()

add_subdirectory(src)

if(RUN_TESTS)
    message(STATUS "UNIT TESTS ENABLED")
    add_subdirectory(tests)
endif()
