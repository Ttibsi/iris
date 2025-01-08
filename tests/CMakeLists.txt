include_directories(${PROJECT_SOURCE_DIR}/include)

add_compile_options(-g)

add_executable(test_exe
    main.cpp # DO not modify
)

if(ENABLE_ASAN)
    message(STATUS "ADDRESSSANITIZER ENABLED")
    target_link_libraries(test_exe PRIVATE -fsanitize=address)
endif()

target_link_libraries(test_exe PUBLIC rawterm iris_src)
