include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include/catch2/include) 

add_compile_options(-g)

find_package(Catch2 3 REQUIRED PATHS ${PROJECT_SOURCE_DIR}/include/catch2/lib64/cmake)

add_executable(test_exe
    controller_test.cpp
    model_test.cpp
    text_io_test.cpp
    view_test.cpp
)

if(ENABLE_ASAN)
    message(STATUS "ADDRESSSANITIZER ENABLED")
    target_link_libraries(test_exe PRIVATE -fsanitize=address)
endif()

target_link_libraries(test_exe PUBLIC iris_src rawterm Catch2::Catch2WithMain)
