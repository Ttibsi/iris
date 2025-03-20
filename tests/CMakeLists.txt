include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_compile_options(-g)

include(FetchContent)
fetchcontent_declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.0
)
fetchcontent_makeavailable(Catch2)

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

target_link_libraries(test_exe PUBLIC rawterm iris_src)
