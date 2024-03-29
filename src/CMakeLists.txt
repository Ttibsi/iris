include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

# This is all for building the `--version` flag at compile time
execute_process(
    COMMAND git tag
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
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

add_subdirectory(highlighting)

add_library(lib SHARED
    buffer.cpp
    editor.cpp
    file_manip.cpp
    handle_keypress.cpp
    text_manip.cpp
    viewport.cpp
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(lib PRIVATE highlighter)
target_link_libraries(${PROJECT_NAME} PUBLIC lib highlighter)
