include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_compile_options(-g)

add_executable(test_exe
    main.cpp # DO not modify

    controller_test.cpp
    model_test.cpp
    text_io_test.cpp
    view_test.cpp
)

if(ENABLE_ASAN)
    message(STATUS "ADDRESSSANITIZER ENABLED")
    target_link_libraries(test_exe PRIVATE -fsanitize=address)
endif()

target_link_libraries(test_exe PUBLIC rawterm iris_src)
