include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(lib${PROJECT_NAME} SHARED
    buffer.cpp
    keypress.cpp
    rawterm_wrapper.cpp
)

add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(lib${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} PUBLIC lib${PROJECT_NAME})
