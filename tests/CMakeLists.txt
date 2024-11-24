include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
fetchcontent_declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)
fetchcontent_makeavailable(Catch2)

include(CTest)
include(Catch)

add_compile_options(-g)

add_executable(test_exe
    action_test.cpp
    controller_test.cpp
    file_io_test.cpp
    gapbuffer_test.cpp
    logger_test.cpp
    model_test.cpp
    view_test.cpp
)

if(ENABLE_ASAN)
    message(STATUS "ADDRESSSANITIZER ENABLED")
    target_link_libraries(test_exe PRIVATE -fsanitize=address -static-libsan)
endif()

target_link_libraries(test_exe PRIVATE iris_src rawterm Catch2::Catch2WithMain)
catch_discover_tests(test_exe)
