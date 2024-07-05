cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_DIRECTORY_LABELS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(iris
  LANGUAGES
    C CXX
  VERSION
    0.0.1
)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/rawterm
)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/text_transform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view.cpp
)

target_compile_features(${PROJECT_NAME}
  PRIVATE cxx_std_20
)

target_compile_options(${PROJECT_NAME} PUBLIC
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_link_options(${PROJECT_NAME} PRIVATE
    -static-libgcc
    -static-libstdc++
    -Wl,-R,'$$ORIGIN'
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE rawterm
)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
)
