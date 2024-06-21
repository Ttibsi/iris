include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
fetchcontent_declare(
  rawterm
  GIT_REPOSITORY https://github.com/ttibsi/rawterm
  GIT_TAG        origin/main
)
fetchcontent_makeavailable(rawterm)

# For configuring version.h
# TODO: Investigate putting this in it's own cmake file
execute_process(
    COMMAND git describe
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_VARIABLE GIT_DESC_ERROR
    RESULT_VARIABLE RET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT (RET EQUAL 0))
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(GIT_VERSION "v0.0.0-${GIT_HASH}")
endif()

string(TIMESTAMP COMPILE_DATE "%Y/%m/%d")
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(BUILD_MODE "Default")
else()
    set(BUILD_MODE ${CMAKE_BUILD_TYPE})
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.h.in"
    "${PROJECT_SOURCE_DIR}/src/version.h"
    @ONLY
)

add_library(iris_src STATIC
    controller.cpp
    editor.cpp
    filesystem.cpp
    model.cpp
    text_transform.cpp
    view.cpp
)
target_link_libraries(iris_src PUBLIC rawterm)

if(ENABLE_ASAN)
    target_link_libraries(iris_src PUBLIC -fsanitize=address)
endif()

if(ENABLE_COVERAGE)
    message(STATUS "COVERAGE ENABLED")
    add_compile_options(--coverage -g -O0)
    add_link_options(--coverage)
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC rawterm iris_src)
