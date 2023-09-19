add_executable(test_exe
    # test files here
    main.cpp
)

target_link_libraries(test_exe PRIVATE lib)
add_test(Tests test_exe)
