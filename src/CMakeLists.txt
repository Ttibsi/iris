include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(lib SHARED
    buffer.cpp
    file_manip.cpp
    handle_keypress.cpp
    text_manip.cpp
)

add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} PUBLIC lib)
