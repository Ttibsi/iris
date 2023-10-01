include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
fetchcontent_declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
fetchcontent_makeavailable(googletest)
include(GoogleTest)

add_library(GTest INTERFACE IMPORTED)
target_link_libraries(GTest INTERFACE gtest_main)

add_executable(test_exe
    # test files here
    main.cpp
)

target_link_libraries(test_exe PRIVATE lib GTest)
gtest_discover_tests(test_exe)
